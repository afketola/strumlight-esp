#include "ble_server.h"
#include "esp_log.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

static const char *TAG = "Main";

extern "C" void app_main() {
    ESP_LOGI(TAG, "Initializing System...");

    startBLEServer();  // Start the BLE server

    while (true) {
        if (deviceConnected) {
            ESP_LOGI(TAG, "Client is still connected...");
        } else {
            ESP_LOGI(TAG, "Waiting for connection...");
        }
        vTaskDelay(pdMS_TO_TICKS(5000));
    }
}
