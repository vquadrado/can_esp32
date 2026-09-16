#include "reader_ctx.hpp"
#include "tasks/tasks.hpp"

#include "can_bus.hpp"
#include "stats.hpp"

void task_bitrate_probe(void *)
{
    vTaskDelay(pdMS_TO_TICKS(3000));
    uint32_t rx = stats_rx();
    uint32_t err = stats_err() + can_bus_error_count();
    if (rx == 0 && err > 0 && !reader_tx_armed()) {
        reader_log_text("no frames, trying 250000");
        if (can_bus_restart(250000, true) == ESP_OK) {
            vTaskDelay(pdMS_TO_TICKS(3000));
            if (stats_rx() == rx) {
                reader_log_text("revert 500000");
                can_bus_restart(500000, true);
            }
        }
    }
    xEventGroupSetBits(reader_ipc().events, kEvtBitrateLocked);
    vTaskDelete(nullptr);
}
