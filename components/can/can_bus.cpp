#include "can_bus.hpp"

#include <atomic>
#include <cstring>

#include "driver/gpio.h"
#include "esp_log.h"
#include "esp_timer.h"
#include "esp_twai.h"
#include "esp_twai_onchip.h"
#include "freertos/semphr.h"
#include "hal/twai_types.h"

static const char *TAG = "can_bus";

static SemaphoreHandle_t s_mutex;
static twai_node_handle_t s_node;
static QueueHandle_t s_rx_q;
static CanBusConfig s_cfg;
static bool s_started;
static std::atomic<uint32_t> s_isr_drops{0};
static std::atomic<uint32_t> s_isr_err{0};
static std::atomic<uint32_t> s_bus_off{0};

static bool IRAM_ATTR on_rx_done(twai_node_handle_t handle, const twai_rx_done_event_data_t *, void *)
{
    uint8_t buf[8] = {};
    twai_frame_t rx = {};
    rx.buffer = buf;
    rx.buffer_len = sizeof(buf);
    if (twai_node_receive_from_isr(handle, &rx) != ESP_OK) {
        return false;
    }

    CanFrame cf = {};
    cf.timestamp_us = static_cast<uint32_t>(esp_timer_get_time());
    cf.id = rx.header.id;
    cf.dlc = static_cast<uint8_t>(rx.header.dlc > 8 ? 8 : rx.header.dlc);
    cf.is_extended = rx.header.ide;
    cf.is_rtr = rx.header.rtr;
    std::memcpy(cf.data, buf, cf.dlc);

    BaseType_t woken = pdFALSE;
    if (s_rx_q && xQueueSendFromISR(s_rx_q, &cf, &woken) != pdTRUE) {
        s_isr_drops.fetch_add(1, std::memory_order_relaxed);
    }
    return woken == pdTRUE;
}

static bool IRAM_ATTR on_error(twai_node_handle_t, const twai_error_event_data_t *, void *)
{
    s_isr_err.fetch_add(1, std::memory_order_relaxed);
    return false;
}

static bool IRAM_ATTR on_state_change(twai_node_handle_t, const twai_state_change_event_data_t *edata, void *)
{
    if (edata->new_sta == TWAI_ERROR_BUS_OFF) {
        s_bus_off.fetch_add(1, std::memory_order_relaxed);
    }
    return false;
}

static esp_err_t create_node(uint32_t bitrate, bool listen_only)
{
    twai_onchip_node_config_t node_config = {};
    node_config.io_cfg.tx = static_cast<gpio_num_t>(s_cfg.tx_gpio);
    node_config.io_cfg.rx = static_cast<gpio_num_t>(s_cfg.rx_gpio);
    node_config.io_cfg.quanta_clk_out = GPIO_NUM_NC;
    node_config.io_cfg.bus_off_indicator = GPIO_NUM_NC;
    node_config.bit_timing.bitrate = bitrate;
    node_config.tx_queue_depth = 8;
    node_config.fail_retry_cnt = listen_only ? 0 : 3;
    node_config.flags.enable_listen_only = listen_only ? 1 : 0;
    node_config.flags.no_receive_rtr = 1;

    esp_err_t err = twai_new_node_onchip(&node_config, &s_node);
    if (err != ESP_OK) {
        return err;
    }

    twai_event_callbacks_t cbs = {};
    cbs.on_rx_done = on_rx_done;
    cbs.on_error = on_error;
    cbs.on_state_change = on_state_change;
    err = twai_node_register_event_callbacks(s_node, &cbs, nullptr);
    if (err != ESP_OK) {
        twai_node_delete(s_node);
        s_node = nullptr;
        return err;
    }
    return twai_node_enable(s_node);
}

static void destroy_node()
{
    if (!s_node) {
        return;
    }
    twai_node_disable(s_node);
    twai_node_delete(s_node);
    s_node = nullptr;
}

esp_err_t can_bus_start(const CanBusConfig &cfg)
{
    if (!s_mutex) {
        s_mutex = xSemaphoreCreateMutex();
    }
    xSemaphoreTake(s_mutex, portMAX_DELAY);
    s_cfg = cfg;
    s_rx_q = cfg.rx_isr_queue;

    gpio_config_t io = {};
    io.pin_bit_mask = 1ULL << cfg.tx_gpio;
    io.mode = GPIO_MODE_OUTPUT;
    io.pull_up_en = GPIO_PULLUP_ENABLE;
    gpio_config(&io);
    gpio_set_level(static_cast<gpio_num_t>(cfg.tx_gpio), 1);

    esp_err_t err = create_node(cfg.bitrate, cfg.listen_only);
    s_started = err == ESP_OK;
    xSemaphoreGive(s_mutex);
    if (err == ESP_OK) {
        ESP_LOGI(TAG, "TWAI start bitrate=%lu listen_only=%d tx=%d rx=%d",
                 (unsigned long)cfg.bitrate, (int)cfg.listen_only, cfg.tx_gpio, cfg.rx_gpio);
    } else {
        ESP_LOGE(TAG, "TWAI start failed: %s", esp_err_to_name(err));
    }
    return err;
}

esp_err_t can_bus_restart(uint32_t bitrate, bool listen_only)
{
    xSemaphoreTake(s_mutex, portMAX_DELAY);
    destroy_node();
    s_cfg.bitrate = bitrate;
    s_cfg.listen_only = listen_only;
    esp_err_t err = create_node(bitrate, listen_only);
    s_started = err == ESP_OK;
    xSemaphoreGive(s_mutex);
    ESP_LOGI(TAG, "TWAI restart bitrate=%lu listen_only=%d err=%s",
             (unsigned long)bitrate, (int)listen_only, esp_err_to_name(err));
    return err;
}

esp_err_t can_bus_transmit(const CanFrame &frame, int timeout_ms)
{
    if (!s_node || s_cfg.listen_only) {
        return ESP_ERR_INVALID_STATE;
    }
    uint8_t buf[8];
    std::memcpy(buf, frame.data, 8);
    twai_frame_t tx = {};
    tx.header.id = frame.id;
    tx.header.dlc = frame.dlc;
    tx.buffer = buf;
    tx.buffer_len = frame.dlc;
    xSemaphoreTake(s_mutex, portMAX_DELAY);
    esp_err_t err = twai_node_transmit(s_node, &tx, timeout_ms);
    xSemaphoreGive(s_mutex);
    return err;
}

void can_bus_recover_if_bus_off()
{
    if (!s_node) {
        return;
    }
    twai_node_status_t st = {};
    if (twai_node_get_info(s_node, &st, nullptr) != ESP_OK) {
        return;
    }
    if (st.state == TWAI_ERROR_BUS_OFF) {
        twai_node_recover(s_node);
    }
}

uint32_t can_bus_error_count()
{
    if (!s_node) {
        return 0;
    }
    twai_node_record_t rec = {};
    twai_node_get_info(s_node, nullptr, &rec);
    return rec.bus_err_num;
}

bool can_bus_is_bus_off()
{
    if (!s_node) {
        return false;
    }
    twai_node_status_t st = {};
    if (twai_node_get_info(s_node, &st, nullptr) != ESP_OK) {
        return false;
    }
    return st.state == TWAI_ERROR_BUS_OFF;
}

uint32_t can_bus_take_isr_drops()
{
    return s_isr_drops.exchange(0, std::memory_order_relaxed);
}

uint32_t can_bus_take_isr_err()
{
    return s_isr_err.exchange(0, std::memory_order_relaxed);
}

uint32_t can_bus_take_bus_off()
{
    return s_bus_off.exchange(0, std::memory_order_relaxed);
}

void can_bus_stop()
{
    xSemaphoreTake(s_mutex, portMAX_DELAY);
    destroy_node();
    s_started = false;
    xSemaphoreGive(s_mutex);
}
