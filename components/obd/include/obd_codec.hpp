#pragma once

#include "can_types.hpp"

#include <cstdint>

constexpr uint32_t kObdIdFunctional = 0x7DF;
constexpr uint32_t kObdIdEcmReq = 0x7E0;
constexpr uint32_t kObdIdEcmResp = 0x7E8;

void obd_make_mode01_request(CanFrame &out, uint32_t can_id, uint8_t pid);
void obd_make_mode_request(CanFrame &out, uint32_t can_id, uint8_t mode, uint8_t pid);
bool obd_parse_nrc(const CanFrame &frame, uint8_t *sid, uint8_t *code);
bool obd_parse_mode01(const CanFrame &frame, uint8_t *pid, uint8_t *a, uint8_t *b, uint8_t *payload_len);
