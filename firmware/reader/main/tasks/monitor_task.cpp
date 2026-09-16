#include "reader_ctx.hpp"
#include "tasks/tasks.hpp"

#include "can_bus.hpp"
#include "esp_heap_caps.h"
#include "esp_task_wdt.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "sdkconfig.h"
#include "stats.hpp"

#include <cstdio>

void task_monitor(void *)
{
    esp_task_wdt_add(nullptr);
    auto &ipc = reader_ipc();
    const int period_s = CONFIG_READER_STATS_PERIOD_S;
    for (;;) {
        esp_task_wdt_reset();
        vTaskDelay(pdMS_TO_TICKS(1000));
        if (can_bus_is_bus_off()) {
            xEventGroupSetBits(ipc.events, kEvtBusOff);
            reader_set_tx_armed(false);
            can_bus_recover_if_bus_off();
            can_bus_restart(500000, true);
            reader_log_text("recovered listen-only");
        }
        if (period_s <= 0) {
            continue;
        }
        TaskHandle_t rx_handle = xTaskGetHandle("can_rx");
        UBaseType_t stack_rx = rx_handle ? uxTaskGetStackHighWaterMark(rx_handle) : 0;
        printf("uptime_s %lu rx %lu err %lu drops_rx %lu drops_log %lu q_rx_hwm %lu heap_free %u stack_can_rx %u bus_off %lu tx_armed %d\n",
               (unsigned long)(xTaskGetTickCount() / configTICK_RATE_HZ),
               (unsigned long)stats_rx(),
               (unsigned long)stats_err(),
               (unsigned long)stats_drops_rx(),
               (unsigned long)stats_drops_log(),
               (unsigned long)stats_q_rx_hwm(),
               (unsigned)heap_caps_get_free_size(MALLOC_CAP_DEFAULT),
               (unsigned)stack_rx,
               (unsigned long)stats_bus_off(),
               reader_tx_armed() ? 1 : 0);
        fflush(stdout);
    }
}
