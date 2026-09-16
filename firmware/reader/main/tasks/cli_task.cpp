#include "reader_ctx.hpp"
#include "tasks/tasks.hpp"

#include "can_bus.hpp"
#include "sdkconfig.h"

#include <cstdio>
#include <cstdlib>
#include <cstring>

static void help()
{
    printf("commands: help stats tx on|off obd pid <hex> obd discover obd vin mode listen\n");
}

void task_cli(void *)
{
    auto &ipc = reader_ipc();
    char line[64];
    help();
    for (;;) {
        if (!fgets(line, sizeof(line), stdin)) {
            vTaskDelay(pdMS_TO_TICKS(200));
            continue;
        }
        line[strcspn(line, "\r\n")] = 0;
        if (line[0] == 0) {
            continue;
        }
        if (strcmp(line, "help") == 0) {
            help();
        } else if (strcmp(line, "stats") == 0) {
            xEventGroupSetBits(ipc.events, 0); // monitor prints periodically
            printf("tx_armed=%d\n", reader_tx_armed() ? 1 : 0);
        } else if (strcmp(line, "tx on") == 0) {
#if CONFIG_READER_ALLOW_TX
            if (can_bus_restart(500000, false) == ESP_OK) {
                reader_set_tx_armed(true);
                xEventGroupSetBits(ipc.events, kEvtDiscover);
                reader_log_text("tx armed");
            }
#else
            printf("tx denied (CONFIG_READER_ALLOW_TX=n)\n");
#endif
        } else if (strcmp(line, "tx off") == 0 || strcmp(line, "mode listen") == 0) {
            reader_set_tx_armed(false);
            can_bus_restart(500000, true);
            reader_log_text("listen-only");
        } else if (strcmp(line, "obd discover") == 0) {
            if (!reader_tx_armed()) {
                printf("tx not armed\n");
            } else {
                xEventGroupSetBits(ipc.events, kEvtDiscover);
            }
        } else if (strcmp(line, "obd vin") == 0) {
            if (!reader_tx_armed()) {
                printf("tx not armed\n");
            } else {
                xEventGroupSetBits(ipc.events, kEvtVin);
            }
        } else if (strncmp(line, "obd pid ", 8) == 0) {
            if (!reader_tx_armed()) {
                printf("tx not armed\n");
                continue;
            }
            unsigned pid = 0;
            if (sscanf(line + 8, "%x", &pid) == 1) {
                ipc.oneshot_mode = 0x01;
                ipc.oneshot_pid = static_cast<uint8_t>(pid);
                xEventGroupSetBits(ipc.events, kEvtOneshot);
            }
        } else {
            printf("unknown command\n");
            help();
        }
    }
}
