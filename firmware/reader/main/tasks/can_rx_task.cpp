#include "reader_ctx.hpp"
#include "tasks/tasks.hpp"

#include "can_bus.hpp"
#include "can_types.hpp"
#include "esp_task_wdt.h"
#include "esp_timer.h"
#include "sdkconfig.h"
#include "stats.hpp"

void task_can_rx(void *)
{
    esp_task_wdt_add(nullptr);
    auto &ipc = reader_ipc();
    CanFrame frame = {};
    for (;;) {
        esp_task_wdt_reset();
        uint32_t drops = can_bus_take_isr_drops();
        while (drops--) {
            stats_inc_drops_rx();
        }
        uint32_t errs = can_bus_take_isr_err();
        while (errs--) {
            stats_inc_err();
        }
        uint32_t bo = can_bus_take_bus_off();
        if (bo) {
            while (bo--) {
                stats_inc_bus_off();
            }
            xEventGroupSetBits(ipc.events, kEvtBusOff);
            reader_set_tx_armed(false);
            reader_log_text("bus-off");
        }

        if (xQueueReceive(ipc.q_isr, &frame, pdMS_TO_TICKS(200)) != pdTRUE) {
            continue;
        }
        if (frame.timestamp_us == 0) {
            frame.timestamp_us = static_cast<uint32_t>(esp_timer_get_time());
        }
        if (frame.is_extended || frame.is_rtr) {
            stats_inc_err();
            continue;
        }
        stats_inc_rx();
        stats_note_q_rx_depth(uxQueueMessagesWaiting(ipc.q_rx) + 1);
        if (xQueueSend(ipc.q_rx, &frame, 0) != pdTRUE) {
            stats_inc_drops_rx();
        }
#if CONFIG_READER_LOG_ALL
        reader_enqueue_log_frame(frame);
#else
        if (can_id_is_obd_response(frame.id)) {
            reader_enqueue_log_frame(frame);
        }
#endif
        if (can_id_is_obd_response(frame.id)) {
            xQueueSend(ipc.q_isotp, &frame, 0);
        }
    }
}
