#include "reader_ctx.hpp"
#include "tasks/tasks.hpp"

#include "obd_codec.hpp"
#include "esp_task_wdt.h"
#include "stats.hpp"

static const uint8_t kDiscoverPids[] = {0x00, 0x20, 0x40, 0x60};
static const uint8_t kSlowPids[] = {0x05, 0x0D, 0x0F, 0x11, 0x2F, 0x51, 0x01, 0x21};

static void send_pid(uint8_t mode, uint8_t pid)
{
    CanFrame req = {};
    obd_make_mode_request(req, kObdIdFunctional, mode, pid);
    xQueueSend(reader_ipc().q_tx, &req, 0);
}

void task_obd_client(void *)
{
    esp_task_wdt_add(nullptr);
    auto &ipc = reader_ipc();
    bool discovered = false;
    size_t slow_idx = 0;
    TickType_t last_0c = 0;
    TickType_t last_slow = 0;

    for (;;) {
        esp_task_wdt_reset();
        EventBits_t bits = xEventGroupWaitBits(ipc.events, kEvtTxArmed, pdFALSE, pdTRUE, pdMS_TO_TICKS(200));
        if ((bits & kEvtTxArmed) == 0) {
            discovered = false;
            continue;
        }

        if (bits & kEvtDiscover) {
            xEventGroupClearBits(ipc.events, kEvtDiscover);
            for (uint8_t pid : kDiscoverPids) {
                send_pid(0x01, pid);
                vTaskDelay(pdMS_TO_TICKS(30));
            }
            discovered = true;
        }

        if (bits & kEvtOneshot) {
            xEventGroupClearBits(ipc.events, kEvtOneshot);
            send_pid(ipc.oneshot_mode, ipc.oneshot_pid);
        }

        TickType_t now = xTaskGetTickCount();
        if (!discovered) {
            xEventGroupSetBits(ipc.events, kEvtDiscover);
            continue;
        }

        if ((now - last_0c) >= pdMS_TO_TICKS(100)) {
            last_0c = now;
            send_pid(0x01, 0x0C);
        }
        if ((now - last_slow) >= pdMS_TO_TICKS(500)) {
            last_slow = now;
            send_pid(0x01, kSlowPids[slow_idx]);
            slow_idx = (slow_idx + 1) % (sizeof(kSlowPids) / sizeof(kSlowPids[0]));
        }

        (void)stats_obd_timeout;
    }
}
