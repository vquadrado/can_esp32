idf_component_register(
    SRCS
        "app_main.cpp"
        "reader_ctx.cpp"
        "tasks/can_rx_task.cpp"
        "tasks/can_tx_task.cpp"
        "tasks/parser_task.cpp"
        "tasks/isotp_task.cpp"
        "tasks/obd_client_task.cpp"
        "tasks/logger_task.cpp"
        "tasks/monitor_task.cpp"
        "tasks/cli_task.cpp"
        "tasks/bitrate_probe_task.cpp"
    INCLUDE_DIRS "." "tasks"
    REQUIRES can obd parser isotp diagnostics nvs_flash esp_timer heap esp_driver_gpio
)
