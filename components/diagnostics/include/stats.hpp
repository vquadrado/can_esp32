#pragma once

#include <cstdint>

void stats_inc_rx();
void stats_inc_err();
void stats_inc_drops_rx();
void stats_inc_drops_log();
void stats_inc_obd_timeout();
void stats_inc_obd_nrc();
void stats_inc_bus_off();
void stats_note_q_rx_depth(uint32_t depth);

uint32_t stats_rx();
uint32_t stats_err();
uint32_t stats_drops_rx();
uint32_t stats_drops_log();
uint32_t stats_obd_timeout();
uint32_t stats_obd_nrc();
uint32_t stats_bus_off();
uint32_t stats_q_rx_hwm();
