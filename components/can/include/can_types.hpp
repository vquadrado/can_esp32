#pragma once

#include <cstdint>

struct CanFrame {
    uint32_t timestamp_us;
    uint32_t id;
    uint8_t dlc;
    uint8_t data[8];
    bool is_extended;
    bool is_rtr;
};

enum class SignalType : uint8_t {
    RPM,
    SPEED,
    COOLANT_TEMP,
    INTAKE_TEMP,
    THROTTLE,
    FUEL_LEVEL,
    BATTERY_VOLTAGE,
    UNKNOWN
};

struct DecodedSignal {
    SignalType type;
    uint32_t timestamp_us;
    float value;
};

inline bool can_id_is_obd_response(uint32_t id)
{
    return id >= 0x7E8 && id <= 0x7EF;
}
