#include "reader_ctx.hpp"
#include "tasks/tasks.hpp"

#include "obd_codec.hpp"
#include "obd_decode.hpp"
#include "stats.hpp"

void task_parser(void *)
{
    auto &ipc = reader_ipc();
    CanFrame frame = {};
    for (;;) {
        if (xQueueReceive(ipc.q_rx, &frame, portMAX_DELAY) != pdTRUE) {
            continue;
        }
        if (obd_parse_nrc(frame, nullptr, nullptr)) {
            stats_inc_obd_nrc();
            continue;
        }
        DecodedSignal sig = {};
        if (obd_decode_signal(frame, sig)) {
            LogMsg msg = {};
            msg.kind = LogKind::Signal;
            msg.signal = sig;
            if (xQueueSend(ipc.q_log, &msg, 0) != pdTRUE) {
                stats_inc_drops_log();
            }
        }
    }
}
