#include "reader_ctx.hpp"

#include "tasks/tasks.hpp"

#include <cstdarg>
#include <cstdio>
#include <cstring>

#include "stats.hpp"

static ReaderIpc s_ipc;

ReaderIpc &reader_ipc()
{
    return s_ipc;
}

void reader_log_text(const char *fmt, ...)
{
    LogMsg msg = {};
    msg.kind = LogKind::Text;
    va_list ap;
    va_start(ap, fmt);
    vsnprintf(msg.text, sizeof(msg.text), fmt, ap);
    va_end(ap);
    if (s_ipc.q_log && xQueueSend(s_ipc.q_log, &msg, 0) != pdTRUE) {
        stats_inc_drops_log();
    }
}

void reader_enqueue_log_frame(const CanFrame &frame)
{
    LogMsg msg = {};
    msg.kind = LogKind::Frame;
    msg.frame = frame;
    if (s_ipc.q_log && xQueueSend(s_ipc.q_log, &msg, 0) != pdTRUE) {
        stats_inc_drops_log();
    }
}

void reader_set_tx_armed(bool armed)
{
    if (!s_ipc.events) {
        return;
    }
    if (armed) {
        xEventGroupSetBits(s_ipc.events, kEvtTxArmed);
    } else {
        xEventGroupClearBits(s_ipc.events, kEvtTxArmed);
    }
}

bool reader_tx_armed()
{
    if (!s_ipc.events) {
        return false;
    }
    return (xEventGroupGetBits(s_ipc.events) & kEvtTxArmed) != 0;
}

void reader_start_tasks()
{
    xTaskCreatePinnedToCore(task_can_rx, "can_rx", 4096, nullptr, kPrioCanRx, nullptr, 0);
    xTaskCreatePinnedToCore(task_can_tx, "can_tx", 4096, nullptr, kPrioCanTx, nullptr, 0);
    xTaskCreatePinnedToCore(task_parser, "parser", 4096, nullptr, kPrioParser, nullptr, 0);
    xTaskCreatePinnedToCore(task_isotp, "isotp", 6144, nullptr, kPrioIsotp, nullptr, 0);
    xTaskCreatePinnedToCore(task_obd_client, "obd", 4096, nullptr, kPrioObd, nullptr, 0);
    xTaskCreatePinnedToCore(task_monitor, "monitor", 4096, nullptr, kPrioMonitor, nullptr, 1);
    xTaskCreatePinnedToCore(task_logger, "logger", 6144, nullptr, kPrioLogger, nullptr, 1);
    xTaskCreatePinnedToCore(task_cli, "cli", 4096, nullptr, kPrioCli, nullptr, 1);
    xTaskCreatePinnedToCore(task_bitrate_probe, "br_probe", 3072, nullptr, 3, nullptr, 1);
}
