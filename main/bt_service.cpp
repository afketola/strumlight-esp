#include "bt_service.h"
#include "esp_bt.h"
#include "esp_bt_main.h"
#include "esp_gap_bt_api.h"    // For GAP functions: set device name and scan mode
#include "esp_spp_api.h"
#include "esp_err.h"
#include "esp_log.h"
#include "nvs_flash.h"
#include "esp_bt_device.h"  // ✅ Required for esp_bt_dev_set_device_name


#define DEVICE_NAME "StrumLight_WROVER"

static const char *TAG = "BT_SERVICE";

void initBT(void) {
    esp_err_t ret;

    ESP_LOGI(TAG, "Initializing Classic Bluetooth...");

    // Step 1: Initialize NVS (Required for Bluetooth)
    ret = nvs_flash_init();
    if (ret == ESP_ERR_NVS_NO_FREE_PAGES || ret == ESP_ERR_NVS_NEW_VERSION_FOUND) {
        ESP_ERROR_CHECK(nvs_flash_erase());
        ret = nvs_flash_init();
    }
    ESP_ERROR_CHECK(ret);

    // Step 2: Release BLE memory (Only using Classic Bluetooth)
    ret = esp_bt_controller_mem_release(ESP_BT_MODE_BLE);
    if (ret != ESP_OK) {
        ESP_LOGE(TAG, "Failed to release BLE memory! Error: %s", esp_err_to_name(ret));
    }

    // Step 3: Initialize the Bluetooth Controller
    esp_bt_controller_config_t bt_cfg = BT_CONTROLLER_INIT_CONFIG_DEFAULT();
    ret = esp_bt_controller_init(&bt_cfg);
    if (ret != ESP_OK) {
        ESP_LOGE(TAG, "Bluetooth Controller initialization failed! Error: %s", esp_err_to_name(ret));
        return;
    }

    // Step 4: Enable the Bluetooth Controller in Classic Mode (BR/EDR)
    ret = esp_bt_controller_enable(ESP_BT_MODE_CLASSIC_BT);
    if (ret != ESP_OK) {
        ESP_LOGE(TAG, "Bluetooth Controller enabling failed! Error: %s", esp_err_to_name(ret));
        return;
    }

    // Step 5: Initialize the Bluedroid stack
    ret = esp_bluedroid_init();
    if (ret != ESP_OK) {
        ESP_LOGE(TAG, "Bluedroid initialization failed! Error: %s", esp_err_to_name(ret));
        return;
    }
    ret = esp_bluedroid_enable();
    if (ret != ESP_OK) {
        ESP_LOGE(TAG, "Bluedroid enabling failed! Error: %s", esp_err_to_name(ret));
        return;
    }

    // Step 6: Set the Bluetooth Device Name (Correct API for ESP-IDF v5.1)
    ret = esp_bt_dev_set_device_name(DEVICE_NAME);
    if (ret != ESP_OK) {
        ESP_LOGE(TAG, "Failed to set device name! Error: %s", esp_err_to_name(ret));
        return;
    }

    // Step 7: Set Discoverability & Connectability
    ret = esp_bt_gap_set_scan_mode(ESP_BT_CONNECTABLE, ESP_BT_GENERAL_DISCOVERABLE);
    if (ret != ESP_OK) {
        ESP_LOGE(TAG, "Failed to set scan mode! Error: %s", esp_err_to_name(ret));
        return;
    }

    ESP_LOGI(TAG, "Bluetooth initialized successfully as '%s'!", DEVICE_NAME);
}

void restartBT(void) {
    ESP_LOGW(TAG, "Restarting Bluetooth...");
    esp_bluedroid_disable();
    esp_bluedroid_deinit();
    esp_bt_controller_disable();
    esp_bt_controller_deinit();
    initBT();  // Reinitialize Bluetooth
}
