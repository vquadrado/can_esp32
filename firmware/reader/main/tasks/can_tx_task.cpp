#include "reader_ctx.hpp"
#include "tasks/tasks.hpp"

#include "can_bus.hpp"

void task_can_tx(void *)
{
    auto &ipc = reader_ipc();
    CanFrame frame = {};
    for (;;) {
        if (xQueueReceive(ipc.q_tx, &frame, pdMS_TO_TICKS(200)) != pdTRUE) {
            continue;
        }
        if (!reader_tx_armed()) {
            continue;
        }
        can_bus_transmit(frame, 50);
    }
}
