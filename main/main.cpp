#include <stdio.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "bt_service.h"

extern "C" void app_main(void) {
    printf("🚀 Starting Strumlight ESP32 Classic Bluetooth...\n");

    // Initialize Bluetooth
    initBT();

    // Main loop: periodically report status.
    while (true) {
        printf("📡 Bluetooth Running...\n");
        vTaskDelay(pdMS_TO_TICKS(5000));
    }
}
