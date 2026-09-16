#pragma once

#include "can_types.hpp"

#include <cstddef>
#include <cstdint>

enum class IsotpEvent {
    None,
    NeedFlowControl,
    Complete,
    Timeout,
    Error
};

class IsotpTester {
public:
    void start_vin_request();
    bool wants_tx(CanFrame &out);
    IsotpEvent on_rx(const CanFrame &frame);
    bool vin(char out[18]) const;
    void reset();

private:
    enum class State { Idle, WaitFfOrSf, WaitCf };
    State state_ = State::Idle;
    bool pending_fc_ = false;
    uint16_t expected_len_ = 0;
    uint16_t got_ = 0;
    uint8_t next_sn_ = 1;
    uint8_t buf_[64] = {};
    uint32_t peer_id_ = 0x7E8;
};
