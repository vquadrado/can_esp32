#pragma once

#include "can_types.hpp"

#include <cstddef>

struct DbcSignal {
    uint32_t id;
    uint8_t start_bit;
    uint8_t len;
    float scale;
    float offset;
    SignalType type;
};

extern const DbcSignal kDbcYaris[];
extern const std::size_t kDbcYarisCount;
