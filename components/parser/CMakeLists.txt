idf_component_register(
    SRCS "obd_decode.cpp" "dbc_yaris.cpp"
    INCLUDE_DIRS "include"
    REQUIRES can obd
)
