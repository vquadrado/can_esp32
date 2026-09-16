#include "reader_ctx.hpp"
#include "tasks/tasks.hpp"

#include "candump.hpp"
#include "esp_task_wdt.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

#include <cstdio>

void task_logger(void *)
{
    esp_task_wdt_add(nullptr);
    auto &ipc = reader_ipc();
    LogMsg msg = {};
    char line[128];
    for (;;) {
        esp_task_wdt_reset();
        if (xQueueReceive(ipc.q_log, &msg, pdMS_TO_TICKS(500)) != pdTRUE) {
            continue;
        }
        switch (msg.kind) {
        case LogKind::Frame:
            candump_format(line, sizeof(line), msg.frame);
            printf("%s\n", line);
            break;
        case LogKind::Signal:
            printf("sig %s=%.2f\n", signal_type_name(msg.signal.type), msg.signal.value);
            break;
        case LogKind::Text:
            printf("%s\n", msg.text);
            break;
        }
        fflush(stdout);
    }
}
