idf_component_register(
    SRCS "can_bus.cpp"
    INCLUDE_DIRS "include"
    REQUIRES esp_driver_twai esp_driver_gpio esp_timer
)
