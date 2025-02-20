#include <stdio.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "driver/gpio.h"
#include "esp_log.h"
#include "NimBLEDevice.h"

#define LED_GPIO_PIN GPIO_NUM_2  // LED GPIO pin
#define DEVICE_NAME  "Strumlight-ESP"
#define SERVICE_UUID "12345678-1234-5678-1234-56789abcdef0"
#define CHARACTERISTIC_UUID "abcd1234-5678-1234-5678-abcdef123456"

static const char *TAG = "BLE_Server";  // Logging tag
bool deviceConnected = false;  // Track BLE connection status

// BLE Connection Callback
class MyServerCallbacks : public NimBLEServerCallbacks {
    void onConnect(NimBLEServer* pServer, NimBLEConnInfo& connInfo) {
        deviceConnected = true;
        ESP_LOGI(TAG, "Device connected: %s", connInfo.getAddress().toString().c_str());
    }

    void onDisconnect(NimBLEServer* pServer, NimBLEConnInfo& connInfo, int reason) {
        deviceConnected = false;
        ESP_LOGI(TAG, "Device disconnected. Restarting advertising...");
        pServer->getAdvertising()->start();  // Restart advertising on disconnect
    }
};

// BLE Write Callback
class MyCallbacks : public NimBLECharacteristicCallbacks {
    void onWrite(NimBLECharacteristic* pCharacteristic, NimBLEConnInfo& connInfo, NimBLEAttValue& value) {
        std::string command = std::string((char*)value.data(), value.length());
        ESP_LOGI(TAG, "Received: %s", command.c_str());

        // LED Control
        if (command == "LED_ON") {
            gpio_set_level(LED_GPIO_PIN, 1);
            ESP_LOGI(TAG, "LED turned ON");
        } else if (command == "LED_OFF") {
            gpio_set_level(LED_GPIO_PIN, 0);
            ESP_LOGI(TAG, "LED turned OFF");
        }
    }
};

// Setup Function
void setup() {
    ESP_LOGI(TAG, "Starting BLE Server");

    // Configure LED pin as output
    gpio_reset_pin(LED_GPIO_PIN);
    gpio_set_direction(LED_GPIO_PIN, GPIO_MODE_OUTPUT);
    gpio_set_level(LED_GPIO_PIN, 0);  // Ensure LED is OFF initially

    // Initialize BLE
    NimBLEDevice::init(DEVICE_NAME);

    // Create BLE Server
    NimBLEServer* pServer = NimBLEDevice::createServer();
    pServer->setCallbacks(new MyServerCallbacks());  // Set connection callbacks

    // Create BLE Service
    NimBLEService* pService = pServer->createService(SERVICE_UUID);

    // Create BLE Characteristic
    NimBLECharacteristic* pCharacteristic = pService->createCharacteristic(
        CHARACTERISTIC_UUID, 
        NIMBLE_PROPERTY::WRITE
    );

    // Assign callback to handle BLE writes
    pCharacteristic->setCallbacks(new MyCallbacks());

    // Start BLE Service
    pService->start();

    // Configure BLE Advertising
    NimBLEAdvertising* pAdvertising = NimBLEDevice::getAdvertising();
    NimBLEAdvertisementData advertisementData;

    advertisementData.setName(DEVICE_NAME);  // Ensure device name is visible
    advertisementData.addServiceUUID(SERVICE_UUID);  // Include service UUID

    pAdvertising->setAdvertisementData(advertisementData);
    pAdvertising->start();

    ESP_LOGI(TAG, "BLE Server Ready & Advertising!");
}

// Main entry point for ESP-IDF applications
extern "C" void app_main() {
    setup();

    // Keep the app running
    while (true) {
        if (deviceConnected) {
            ESP_LOGI(TAG, "Client is still connected...");
        } else {
            ESP_LOGI(TAG, "Waiting for connection...");
        }
        vTaskDelay(pdMS_TO_TICKS(5000));  // Log connection status every 5 seconds
    }
}
