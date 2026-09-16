#include "stats.hpp"

#include <atomic>

static std::atomic<uint32_t> g_rx{0};
static std::atomic<uint32_t> g_err{0};
static std::atomic<uint32_t> g_drops_rx{0};
static std::atomic<uint32_t> g_drops_log{0};
static std::atomic<uint32_t> g_obd_timeout{0};
static std::atomic<uint32_t> g_obd_nrc{0};
static std::atomic<uint32_t> g_bus_off{0};
static std::atomic<uint32_t> g_q_rx_hwm{0};

void stats_inc_rx() { g_rx.fetch_add(1, std::memory_order_relaxed); }
void stats_inc_err() { g_err.fetch_add(1, std::memory_order_relaxed); }
void stats_inc_drops_rx() { g_drops_rx.fetch_add(1, std::memory_order_relaxed); }
void stats_inc_drops_log() { g_drops_log.fetch_add(1, std::memory_order_relaxed); }
void stats_inc_obd_timeout() { g_obd_timeout.fetch_add(1, std::memory_order_relaxed); }
void stats_inc_obd_nrc() { g_obd_nrc.fetch_add(1, std::memory_order_relaxed); }
void stats_inc_bus_off() { g_bus_off.fetch_add(1, std::memory_order_relaxed); }

void stats_note_q_rx_depth(uint32_t depth)
{
    uint32_t prev = g_q_rx_hwm.load(std::memory_order_relaxed);
    while (depth > prev && !g_q_rx_hwm.compare_exchange_weak(prev, depth, std::memory_order_relaxed)) {
    }
}

uint32_t stats_rx() { return g_rx.load(std::memory_order_relaxed); }
uint32_t stats_err() { return g_err.load(std::memory_order_relaxed); }
uint32_t stats_drops_rx() { return g_drops_rx.load(std::memory_order_relaxed); }
uint32_t stats_drops_log() { return g_drops_log.load(std::memory_order_relaxed); }
uint32_t stats_obd_timeout() { return g_obd_timeout.load(std::memory_order_relaxed); }
uint32_t stats_obd_nrc() { return g_obd_nrc.load(std::memory_order_relaxed); }
uint32_t stats_bus_off() { return g_bus_off.load(std::memory_order_relaxed); }
uint32_t stats_q_rx_hwm() { return g_q_rx_hwm.load(std::memory_order_relaxed); }
