idf_component_register(
    SRCS "main.cpp" "bt_service.cpp" "protocol_parser.cpp"
    INCLUDE_DIRS "include"
    REQUIRES nvs_flash bt
)
