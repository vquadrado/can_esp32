#pragma once

#include "can_types.hpp"

bool obd_decode_signal(const CanFrame &frame, DecodedSignal &out);
