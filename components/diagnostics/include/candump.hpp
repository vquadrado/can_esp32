#pragma once

#include "can_types.hpp"

#include <cstddef>

size_t candump_format(char *out, size_t out_len, const CanFrame &frame);
const char *signal_type_name(SignalType type);
