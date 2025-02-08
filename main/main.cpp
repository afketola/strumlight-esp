#include <stdio.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "bt_service.h"

extern "C" void app_main(void) {
    printf("🚀 StrumLight ESP32 Classic Bluetooth Starting...\n");

    initBT();

    while (true) {
        printf("📡 Bluetooth Running...\n");
        vTaskDelay(pdMS_TO_TICKS(5000));
    }
}
