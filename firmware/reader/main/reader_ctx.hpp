#pragma once

#include "can_types.hpp"
#include "freertos/FreeRTOS.h"
#include "freertos/event_groups.h"
#include "freertos/queue.h"

constexpr EventBits_t kEvtTxArmed = 1 << 0;
constexpr EventBits_t kEvtBitrateLocked = 1 << 1;
constexpr EventBits_t kEvtBusOff = 1 << 2;
constexpr EventBits_t kEvtDiscover = 1 << 3;
constexpr EventBits_t kEvtVin = 1 << 4;
constexpr EventBits_t kEvtOneshot = 1 << 5;
constexpr EventBits_t kEvtListen = 1 << 6;

constexpr UBaseType_t kPrioCanRx = 12;
constexpr UBaseType_t kPrioCanTx = 11;
constexpr UBaseType_t kPrioParser = 10;
constexpr UBaseType_t kPrioIsotp = 10;
constexpr UBaseType_t kPrioObd = 8;
constexpr UBaseType_t kPrioMonitor = 6;
constexpr UBaseType_t kPrioLogger = 5;
constexpr UBaseType_t kPrioCli = 4;

enum class LogKind : uint8_t { Frame, Signal, Text };

struct LogMsg {
    LogKind kind;
    CanFrame frame;
    DecodedSignal signal;
    char text[96];
};

struct ReaderIpc {
    QueueHandle_t q_isr;
    QueueHandle_t q_rx;
    QueueHandle_t q_tx;
    QueueHandle_t q_log;
    QueueHandle_t q_signal;
    QueueHandle_t q_isotp;
    EventGroupHandle_t events;
    volatile uint8_t oneshot_pid;
    volatile uint8_t oneshot_mode;
};

ReaderIpc &reader_ipc();
void reader_log_text(const char *fmt, ...);
void reader_enqueue_log_frame(const CanFrame &frame);
void reader_set_tx_armed(bool armed);
bool reader_tx_armed();
void reader_start_tasks();
