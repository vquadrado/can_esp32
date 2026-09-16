#include "candump.hpp"
#include "isotp_tester.hpp"
#include "obd_codec.hpp"
#include "obd_decode.hpp"

#include <cassert>
#include <cstdio>
#include <cstring>

static void test_candump()
{
    CanFrame f = {};
    f.timestamp_us = 1023400;
    f.id = 0x7E8;
    f.dlc = 8;
    f.data[0] = 0x04;
    f.data[1] = 0x41;
    f.data[2] = 0x0C;
    f.data[3] = 0x1A;
    f.data[4] = 0x2B;
    char line[80];
    candump_format(line, sizeof(line), f);
    assert(std::strstr(line, "(1023400) 7E8 [8]") != nullptr);
}

static void test_pid_0c()
{
    CanFrame f = {};
    f.dlc = 8;
    f.data[0] = 0x04;
    f.data[1] = 0x41;
    f.data[2] = 0x0C;
    uint16_t raw = 800 * 4;
    f.data[3] = static_cast<uint8_t>(raw >> 8);
    f.data[4] = static_cast<uint8_t>(raw);
    DecodedSignal s = {};
    assert(obd_decode_signal(f, s));
    assert(s.type == SignalType::RPM);
    assert(s.value > 799.0f && s.value < 801.0f);
}

static void test_dlc0()
{
    CanFrame f = {};
    f.dlc = 0;
    DecodedSignal s = {};
    assert(!obd_decode_signal(f, s));
}

static void test_vin_multiframe()
{
    IsotpTester t;
    t.start_vin_request();
    const char *vin17 = "12345678901234567";
    CanFrame ff = {};
    ff.id = 0x7E8;
    ff.dlc = 8;
    ff.data[0] = 0x10;
    ff.data[1] = 20;
    ff.data[2] = 0x49;
    ff.data[3] = 0x02;
    ff.data[4] = 0x01;
    ff.data[5] = static_cast<uint8_t>(vin17[0]);
    ff.data[6] = static_cast<uint8_t>(vin17[1]);
    ff.data[7] = static_cast<uint8_t>(vin17[2]);
    assert(t.on_rx(ff) == IsotpEvent::NeedFlowControl);
    CanFrame fc = {};
    assert(t.wants_tx(fc));
    assert(fc.data[0] == 0x30);

    CanFrame cf = {};
    cf.id = 0x7E8;
    cf.dlc = 8;
    cf.data[0] = 0x21;
    std::memcpy(cf.data + 1, vin17 + 3, 7);
    assert(t.on_rx(cf) == IsotpEvent::None);
    cf.data[0] = 0x22;
    std::memcpy(cf.data + 1, vin17 + 10, 7);
    assert(t.on_rx(cf) == IsotpEvent::Complete);
    char vin[18] = {};
    assert(t.vin(vin));
    assert(std::strcmp(vin, vin17) == 0);
}

int main()
{
    test_candump();
    test_pid_0c();
    test_dlc0();
    test_vin_multiframe();
    std::puts("ok");
    return 0;
}
