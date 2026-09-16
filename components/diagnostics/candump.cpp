#include "candump.hpp"

#include <cstdio>

size_t candump_format(char *out, size_t out_len, const CanFrame &frame)
{
    if (!out || out_len < 8) {
        return 0;
    }
    int n = std::snprintf(out, out_len, "(%u) %03X [%u]",
                          static_cast<unsigned>(frame.timestamp_us),
                          static_cast<unsigned>(frame.id),
                          static_cast<unsigned>(frame.dlc));
    if (n < 0) {
        return 0;
    }
    size_t used = static_cast<size_t>(n);
    for (uint8_t i = 0; i < frame.dlc && used + 4 < out_len; ++i) {
        int m = std::snprintf(out + used, out_len - used, " %02X", frame.data[i]);
        if (m < 0) {
            break;
        }
        used += static_cast<size_t>(m);
    }
    return used;
}

const char *signal_type_name(SignalType type)
{
    switch (type) {
    case SignalType::RPM:
        return "RPM";
    case SignalType::SPEED:
        return "SPEED";
    case SignalType::COOLANT_TEMP:
        return "COOLANT";
    case SignalType::INTAKE_TEMP:
        return "IAT";
    case SignalType::THROTTLE:
        return "TPS";
    case SignalType::FUEL_LEVEL:
        return "FUEL";
    case SignalType::BATTERY_VOLTAGE:
        return "VBAT";
    default:
        return "UNK";
    }
}
