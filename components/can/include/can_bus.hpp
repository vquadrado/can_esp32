#pragma once

#include "can_types.hpp"
#include "esp_err.h"
#include "freertos/FreeRTOS.h"
#include "freertos/queue.h"

struct CanBusConfig {
    int tx_gpio;
    int rx_gpio;
    uint32_t bitrate;
    bool listen_only;
    QueueHandle_t rx_isr_queue;
};

esp_err_t can_bus_start(const CanBusConfig &cfg);
esp_err_t can_bus_restart(uint32_t bitrate, bool listen_only);
esp_err_t can_bus_transmit(const CanFrame &frame, int timeout_ms);
void can_bus_recover_if_bus_off();
uint32_t can_bus_error_count();
bool can_bus_is_bus_off();
void can_bus_stop();
uint32_t can_bus_take_isr_drops();
uint32_t can_bus_take_isr_err();
uint32_t can_bus_take_bus_off();
