#include "ble_service.h"
#include "protocol_parser.h"
#include "NimBLEDevice.h"

#define SERVICE_UUID         "12345678-1234-5678-1234-56789abcdef0"
#define CHARACTERISTIC_UUID  "12345678-1234-5678-1234-56789abcdef1"

// Callback class to handle characteristic writes and log events
class MyCallbacks : public NimBLECharacteristicCallbacks {
public:
    void onWrite(NimBLECharacteristic* pCharacteristic, NimBLEConnInfo& connInfo) override {
        std::string value = pCharacteristic->getValue();
        printf("📥 Characteristic Write Received!\n");
        printf("🔹 Data Length: %d bytes\n", value.length());
        
        if (!value.empty()) {
            CommandData cmd = parseCommand(value);
            printf("🎸 Parsed Command: chord=%s, fret=%d, duration=%d\n",
                   cmd.chord.c_str(), cmd.fret, cmd.duration);
        } else {
            printf("⚠️ Received empty write command!\n");
        }
    }
};

// Callback class for connection and disconnection events
class ServerCallbacks : public NimBLEServerCallbacks {
    void onConnect(NimBLEServer* pServer, NimBLEConnInfo& connInfo) override {
        printf("✅ Device Connected! MAC: %s\n", connInfo.getAddress().toString().c_str());
        printf("📡 Connected Devices: %d\n", pServer->getConnectedCount());
    }

    void onDisconnect(NimBLEServer* pServer, NimBLEConnInfo& connInfo, int reason) override {
        printf("❌ Device Disconnected! MAC: %s\n", connInfo.getAddress().toString().c_str());
        printf("🔄 Restarting Advertising...\n");
        pServer->startAdvertising();
    }
};

void initBLE() {
    printf("🚀 Initializing BLE...\n");

    // Ensure we fully reset BLE before restarting
    NimBLEDevice::deinit(true);
    vTaskDelay(pdMS_TO_TICKS(500));

    // Initialize NimBLE
    NimBLEDevice::init("StrumLight");
    printf("📡 BLE Initialized as 'StrumLight'\n");

    // Create BLE server
    NimBLEServer* pServer = NimBLEDevice::createServer();
    pServer->setCallbacks(new ServerCallbacks());

    // Create BLE service
    NimBLEService* pService = pServer->createService(SERVICE_UUID);

    // Create writable characteristic
    NimBLECharacteristic* pCharacteristic = pService->createCharacteristic(
        CHARACTERISTIC_UUID,
        NIMBLE_PROPERTY::WRITE
    );
    pCharacteristic->setCallbacks(new MyCallbacks());

    // Start service
    pService->start();

    // Start advertising
    NimBLEAdvertising* pAdvertising = NimBLEDevice::getAdvertising();
    pAdvertising->addServiceUUID(SERVICE_UUID);
    pAdvertising->setMinInterval(160);  // 100ms
    pAdvertising->setMaxInterval(320);  // 200ms
    pAdvertising->start();

    // Print BLE info
    printf("✅ BLE service started and advertising as 'StrumLight'\n");
    printf("📡 ESP32 BLE Address: %s\n", NimBLEDevice::getAddress().toString().c_str());
}
