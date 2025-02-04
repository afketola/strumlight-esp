idf_component_register(
    SRCS "main.cpp" "ble_service.cpp" "protocol_parser.cpp"
    INCLUDE_DIRS "include"
    REQUIRES esp-nimble-cpp
)
