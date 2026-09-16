#pragma once

void task_can_rx(void *arg);
void task_can_tx(void *arg);
void task_parser(void *arg);
void task_isotp(void *arg);
void task_obd_client(void *arg);
void task_logger(void *arg);
void task_monitor(void *arg);
void task_cli(void *arg);
void task_bitrate_probe(void *arg);
