idf_component_register(
    SRCS "main.cpp" "bt_service.cpp"
    INCLUDE_DIRS "include"  # Tells the compiler to look for headers in main/include
    REQUIRES nvs_flash bt
)
