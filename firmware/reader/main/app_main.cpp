#include "reader_ctx.hpp"
#include "tasks/tasks.hpp"

#include "can_bus.hpp"
#include "esp_log.h"
#include "nvs_flash.h"
#include "sdkconfig.h"

static const char *TAG = "reader";

extern "C" void app_main(void)
{
    ESP_LOGI(TAG, "reader boot");
    esp_err_t nvs = nvs_flash_init();
    if (nvs == ESP_ERR_NVS_NO_FREE_PAGES || nvs == ESP_ERR_NVS_NEW_VERSION_FOUND) {
        nvs_flash_erase();
        nvs_flash_init();
    }

    auto &ipc = reader_ipc();
    ipc.q_isr = xQueueCreate(64, sizeof(CanFrame));
    ipc.q_rx = xQueueCreate(64, sizeof(CanFrame));
    ipc.q_tx = xQueueCreate(8, sizeof(CanFrame));
    ipc.q_log = xQueueCreate(32, sizeof(LogMsg));
    ipc.q_signal = xQueueCreate(16, sizeof(DecodedSignal));
    ipc.q_isotp = xQueueCreate(8, sizeof(CanFrame));
    ipc.events = xEventGroupCreate();
    ipc.oneshot_mode = 0x01;
    ipc.oneshot_pid = 0x0C;

    CanBusConfig cfg = {};
    cfg.tx_gpio = CONFIG_READER_TWAI_TX_GPIO;
    cfg.rx_gpio = CONFIG_READER_TWAI_RX_GPIO;
    cfg.bitrate = 500000;
    cfg.listen_only = true;
    cfg.rx_isr_queue = ipc.q_isr;
    ESP_ERROR_CHECK(can_bus_start(cfg));

    reader_start_tasks();
#if CONFIG_READER_ALLOW_TX
    const int allow_tx = 1;
#else
    const int allow_tx = 0;
#endif
    ESP_LOGI(TAG, "listen-only 500k tx=%d rx=%d allow_tx=%d",
             CONFIG_READER_TWAI_TX_GPIO, CONFIG_READER_TWAI_RX_GPIO, allow_tx);
    vTaskDelay(portMAX_DELAY);
}
