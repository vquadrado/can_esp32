#include "obd_codec.hpp"

#include <cstring>

static void pad8(CanFrame &out)
{
    for (int i = out.dlc; i < 8; ++i) {
        out.data[i] = 0x00;
    }
    out.dlc = 8;
    out.is_extended = false;
    out.is_rtr = false;
}

void obd_make_mode01_request(CanFrame &out, uint32_t can_id, uint8_t pid)
{
    obd_make_mode_request(out, can_id, 0x01, pid);
}

void obd_make_mode_request(CanFrame &out, uint32_t can_id, uint8_t mode, uint8_t pid)
{
    out = {};
    out.id = can_id;
    if (mode == 0x03 || mode == 0x07) {
        out.data[0] = 0x01;
        out.data[1] = mode;
        out.dlc = 2;
    } else {
        out.data[0] = 0x02;
        out.data[1] = mode;
        out.data[2] = pid;
        out.dlc = 3;
    }
    pad8(out);
}

bool obd_parse_nrc(const CanFrame &frame, uint8_t *sid, uint8_t *code)
{
    if (frame.dlc < 4) {
        return false;
    }
    uint8_t pci = frame.data[0] & 0xF0;
    if (pci != 0x00) {
        return false;
    }
    if (frame.data[1] != 0x7F) {
        return false;
    }
    if (sid) {
        *sid = frame.data[2];
    }
    if (code) {
        *code = frame.data[3];
    }
    return true;
}

bool obd_parse_mode01(const CanFrame &frame, uint8_t *pid, uint8_t *a, uint8_t *b, uint8_t *payload_len)
{
    if (frame.dlc < 3) {
        return false;
    }
    uint8_t pci_type = frame.data[0] >> 4;
    if (pci_type != 0) {
        return false;
    }
    uint8_t len = frame.data[0] & 0x0F;
    if (len < 2 || frame.data[1] != 0x41) {
        return false;
    }
    if (pid) {
        *pid = frame.data[2];
    }
    if (a) {
        *a = (frame.dlc > 3) ? frame.data[3] : 0;
    }
    if (b) {
        *b = (frame.dlc > 4) ? frame.data[4] : 0;
    }
    if (payload_len) {
        *payload_len = (len > 2) ? static_cast<uint8_t>(len - 2) : 0;
    }
    return true;
}
