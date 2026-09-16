#include "obd_decode.hpp"

#include "obd_codec.hpp"

bool obd_decode_signal(const CanFrame &frame, DecodedSignal &out)
{
    uint8_t pid = 0, a = 0, b = 0, plen = 0;
    if (!obd_parse_mode01(frame, &pid, &a, &b, &plen)) {
        return false;
    }
    out.timestamp_us = frame.timestamp_us;
    switch (pid) {
    case 0x0C:
        out.type = SignalType::RPM;
        out.value = (256.0f * a + b) / 4.0f;
        return true;
    case 0x0D:
        out.type = SignalType::SPEED;
        out.value = static_cast<float>(a);
        return true;
    case 0x05:
        out.type = SignalType::COOLANT_TEMP;
        out.value = static_cast<float>(a) - 40.0f;
        return true;
    case 0x0F:
        out.type = SignalType::INTAKE_TEMP;
        out.value = static_cast<float>(a) - 40.0f;
        return true;
    case 0x11:
        out.type = SignalType::THROTTLE;
        out.value = a * 100.0f / 255.0f;
        return true;
    case 0x2F:
        out.type = SignalType::FUEL_LEVEL;
        out.value = a * 100.0f / 255.0f;
        return true;
    default:
        return false;
    }
}
