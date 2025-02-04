#include <stdio.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "ble_service.h"

extern "C" void app_main(void) {
    printf("🚀 StrumLight ESP32 BLE Starting...\n");

    initBLE();

    while (true) {
        printf("📡 BLE Running...\n");
        vTaskDelay(pdMS_TO_TICKS(5000));
    }
}
