#include "isotp_tester.hpp"

#include "obd_codec.hpp"

#include <cstring>

void IsotpTester::reset()
{
    state_ = State::Idle;
    pending_fc_ = false;
    expected_len_ = 0;
    got_ = 0;
    next_sn_ = 1;
    std::memset(buf_, 0, sizeof(buf_));
}

void IsotpTester::start_vin_request()
{
    reset();
    state_ = State::WaitFfOrSf;
}

bool IsotpTester::wants_tx(CanFrame &out)
{
    if (pending_fc_) {
        pending_fc_ = false;
        out = {};
        out.id = kObdIdEcmReq;
        out.data[0] = 0x30;
        out.data[1] = 0x00;
        out.data[2] = 0x00;
        out.dlc = 3;
        for (int i = 3; i < 8; ++i) {
            out.data[i] = 0x00;
        }
        out.dlc = 8;
        return true;
    }
    return false;
}

IsotpEvent IsotpTester::on_rx(const CanFrame &frame)
{
    if (state_ == State::Idle || frame.dlc < 1) {
        return IsotpEvent::None;
    }
    uint8_t pci = frame.data[0] >> 4;
    if (pci == 0x0 && state_ == State::WaitFfOrSf) {
        uint8_t len = frame.data[0] & 0x0F;
        if (len + 1u > frame.dlc) {
            state_ = State::Idle;
            return IsotpEvent::Error;
        }
        expected_len_ = len;
        got_ = 0;
        for (uint8_t i = 0; i < len && got_ < sizeof(buf_); ++i) {
            buf_[got_++] = frame.data[1 + i];
        }
        state_ = State::Idle;
        return IsotpEvent::Complete;
    }
    if (pci == 0x1 && state_ == State::WaitFfOrSf && frame.dlc >= 2) {
        expected_len_ = ((frame.data[0] & 0x0F) << 8) | frame.data[1];
        got_ = 0;
        for (uint8_t i = 2; i < frame.dlc && got_ < expected_len_ && got_ < sizeof(buf_); ++i) {
            buf_[got_++] = frame.data[i];
        }
        next_sn_ = 1;
        pending_fc_ = true;
        state_ = State::WaitCf;
        peer_id_ = frame.id;
        return IsotpEvent::NeedFlowControl;
    }
    if (pci == 0x2 && state_ == State::WaitCf) {
        uint8_t sn = frame.data[0] & 0x0F;
        if (sn != (next_sn_ & 0x0F)) {
            state_ = State::Idle;
            return IsotpEvent::Error;
        }
        next_sn_ = static_cast<uint8_t>((next_sn_ + 1) & 0x0F);
        for (uint8_t i = 1; i < frame.dlc && got_ < expected_len_ && got_ < sizeof(buf_); ++i) {
            buf_[got_++] = frame.data[i];
        }
        if (got_ >= expected_len_) {
            state_ = State::Idle;
            return IsotpEvent::Complete;
        }
        return IsotpEvent::None;
    }
    return IsotpEvent::None;
}

bool IsotpTester::vin(char out[18]) const
{
    if (!out) {
        return false;
    }
    std::memset(out, 0, 18);
    // 49 02 01 + 17 ASCII
    if (got_ < 20 || buf_[0] != 0x49 || buf_[1] != 0x02) {
        return false;
    }
    std::size_t start = (buf_[2] == 0x01) ? 3 : 2;
    if (got_ < start + 17) {
        return false;
    }
    std::memcpy(out, buf_ + start, 17);
    out[17] = '\0';
    return true;
}
