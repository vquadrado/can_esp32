#include "reader_ctx.hpp"
#include "tasks/tasks.hpp"

#include "isotp_tester.hpp"
#include "obd_codec.hpp"

void task_isotp(void *)
{
    auto &ipc = reader_ipc();
    IsotpTester session;
    CanFrame frame = {};
    bool active = false;
    TickType_t activity = 0;

    for (;;) {
        if (xEventGroupGetBits(ipc.events) & kEvtVin) {
            xEventGroupClearBits(ipc.events, kEvtVin);
            session.start_vin_request();
            active = true;
            activity = xTaskGetTickCount();
            CanFrame req = {};
            obd_make_mode_request(req, kObdIdFunctional, 0x09, 0x02);
            if (reader_tx_armed()) {
                xQueueSend(ipc.q_tx, &req, 0);
            }
        }

        if (xQueueReceive(ipc.q_isotp, &frame, pdMS_TO_TICKS(50)) == pdTRUE) {
            IsotpEvent ev = session.on_rx(frame);
            activity = xTaskGetTickCount();
            if (ev == IsotpEvent::NeedFlowControl) {
                CanFrame fc = {};
                if (session.wants_tx(fc) && reader_tx_armed()) {
                    xQueueSend(ipc.q_tx, &fc, 0);
                }
                active = true;
            } else if (ev == IsotpEvent::Complete) {
                char vin[18] = {};
                if (session.vin(vin)) {
                    reader_log_text("VIN %s", vin);
                } else {
                    reader_log_text("isotp complete");
                }
                session.reset();
                active = false;
            } else if (ev == IsotpEvent::Error) {
                reader_log_text("isotp error");
                session.reset();
                active = false;
            }
        } else if (active && (xTaskGetTickCount() - activity) > pdMS_TO_TICKS(1000)) {
            reader_log_text("isotp timeout");
            session.reset();
            active = false;
        }
    }
}
