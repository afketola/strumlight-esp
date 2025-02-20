#include "ble_server.h"
#include "esp_log.h"
#include "driver/gpio.h"
#include "NimBLEDevice.h"

#define LED_GPIO_PIN GPIO_NUM_2  
#define DEVICE_NAME  "Strumlight-ESP"
#define SERVICE_UUID "12345678-1234-5678-1234-56789abcdef0"
#define CHARACTERISTIC_UUID "abcd1234-5678-1234-5678-abcdef123456"

static const char *TAG = "BLE_Server";

bool deviceConnected = false;  // Define it only in this file

// BLE Connection Callback
class MyServerCallbacks : public NimBLEServerCallbacks {
    void onConnect(NimBLEServer* pServer, NimBLEConnInfo& connInfo) {
        deviceConnected = true;
        ESP_LOGI(TAG, "Device connected: %s", connInfo.getAddress().toString().c_str());
    }

    void onDisconnect(NimBLEServer* pServer, NimBLEConnInfo& connInfo, int reason) {
        deviceConnected = false;
        ESP_LOGI(TAG, "Device disconnected. Restarting advertising...");
        pServer->getAdvertising()->start();
    }
};

// BLE Write Callback
class MyCallbacks : public NimBLECharacteristicCallbacks {
    void onWrite(NimBLECharacteristic* pCharacteristic, NimBLEConnInfo& connInfo, NimBLEAttValue& value) {
        std::string command = std::string((char*)value.data(), value.length());
        ESP_LOGI(TAG, "Received: %s", command.c_str());

        if (command == "LED_ON") {
            gpio_set_level(LED_GPIO_PIN, 1);
            ESP_LOGI(TAG, "LED turned ON");
        } else if (command == "LED_OFF") {
            gpio_set_level(LED_GPIO_PIN, 0);
            ESP_LOGI(TAG, "LED turned OFF");
        }
    }
};

// Function to start BLE server
void startBLEServer() {
    ESP_LOGI(TAG, "Starting BLE Server");

    gpio_reset_pin(LED_GPIO_PIN);
    gpio_set_direction(LED_GPIO_PIN, GPIO_MODE_OUTPUT);
    gpio_set_level(LED_GPIO_PIN, 0);  

    NimBLEDevice::init(DEVICE_NAME);

    NimBLEServer* pServer = NimBLEDevice::createServer();
    pServer->setCallbacks(new MyServerCallbacks());

    NimBLEService* pService = pServer->createService(SERVICE_UUID);

    NimBLECharacteristic* pCharacteristic = pService->createCharacteristic(
        CHARACTERISTIC_UUID, 
        NIMBLE_PROPERTY::WRITE
    );

    pCharacteristic->setCallbacks(new MyCallbacks());

    pService->start();

    NimBLEAdvertising* pAdvertising = NimBLEDevice::getAdvertising();
    NimBLEAdvertisementData advertisementData;

    advertisementData.setName(DEVICE_NAME);
    advertisementData.addServiceUUID(SERVICE_UUID);

    pAdvertising->setAdvertisementData(advertisementData);
    pAdvertising->start();

    ESP_LOGI(TAG, "BLE Server Ready & Advertising!");
}
