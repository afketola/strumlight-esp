#include "bt_service.h"
#include "nvs_flash.h"           // Required for NVS initialization
#include "esp_bt.h"
#include "esp_bt_main.h"
#include "esp_gap_bt_api.h"      // Required for GAP functions
#include "esp_spp_api.h"
#include "esp_err.h"             // Required for error handling

#define DEVICE_NAME "StrumLight_BT"

// Declare the Bluetooth event handler before use
void bt_event_handler(esp_spp_cb_event_t event, esp_spp_cb_param_t *param);

void initBT() {
    printf("🚀 Initializing Classic Bluetooth...\n");
    esp_err_t ret;

    // 🔄 Initialize NVS
    ret = nvs_flash_init();
    if (ret == ESP_ERR_NVS_NO_FREE_PAGES || ret == ESP_ERR_NVS_NEW_VERSION_FOUND) {
        ESP_ERROR_CHECK(nvs_flash_erase());
        ret = nvs_flash_init();
    }
    ESP_ERROR_CHECK(ret);

    // 🔄 Reset Bluetooth if already enabled
    if (esp_bluedroid_get_status() == ESP_BLUEDROID_STATUS_ENABLED) {
        printf("🔄 Deinitializing existing Bluetooth instance...\n");
        esp_bluedroid_disable();
        esp_bluedroid_deinit();
        esp_bt_controller_disable();
        esp_bt_controller_deinit();
    }

    // Initialize the Bluetooth controller
    esp_bt_controller_config_t bt_cfg = BT_CONTROLLER_INIT_CONFIG_DEFAULT();
    ret = esp_bt_controller_init(&bt_cfg);
    if (ret != ESP_OK) {
        printf("❌ Failed to initialize Bluetooth controller! Error: %d\n", ret);
        return;
    }

    ret = esp_bt_controller_enable(ESP_BT_MODE_BTDM);  // Dual mode (Classic + BLE)
    if (ret != ESP_OK) {
        printf("❌ Failed to enable Bluetooth controller! Error: %d\n", ret);
        return;
    }

    // Initialize Bluedroid
    ret = esp_bluedroid_init();
    if (ret != ESP_OK) {
        printf("❌ Failed to initialize Bluedroid stack! Error: %d\n", ret);
        return;
    }

    ret = esp_bluedroid_enable();
    if (ret != ESP_OK) {
        printf("❌ Failed to enable Bluedroid stack! Error: %d\n", ret);
        return;
    }

    // ✅ Set Bluetooth device name
    ret = esp_bt_gap_set_device_name(DEVICE_NAME);
    if (ret != ESP_OK) {
        printf("❌ Failed to set Bluetooth device name! Error: %d\n", ret);
        return;
    }

    // 🔥 **Enable Discovery Mode (Make ESP32 Visible)**
    
    ret = esp_bt_gap_set_scan_mode(ESP_BT_CONNECTABLE, ESP_BT_GENERAL_DISCOVERABLE);
    if (ret != ESP_OK) {
        printf("❌ Failed to set Bluetooth scan mode! Error: %d\n", ret);
        return;
    }
    

    // Register the SPP event handler
    ret = esp_spp_register_callback(bt_event_handler);
    if (ret != ESP_OK) {
        printf("❌ Failed to register SPP event handler! Error: %d\n", ret);
        return;
    }

    // ✅ **Use `esp_spp_enhanced_init()` instead of deprecated `esp_spp_init()`**
    esp_spp_cfg_t spp_cfg = {
        .mode = ESP_SPP_MODE_CB,
        .enable_l2cap_ertm = false,
        .tx_buffer_size = 0, // Default buffer size
    };
    ret = esp_spp_enhanced_init(&spp_cfg);
    if (ret != ESP_OK) {
        printf("❌ Failed to initialize SPP! Error: %d\n", ret);
        return;
    }

    printf("✅ Classic Bluetooth initialized as '%s' and is now DISCOVERABLE!\n", DEVICE_NAME);
}

// Bluetooth event handler (Make sure this function is defined!)
void bt_event_handler(esp_spp_cb_event_t event, esp_spp_cb_param_t *param) {
    switch (event) {
        case ESP_SPP_OPEN_EVT:
            printf("✅ Bluetooth device connected!\n");
            break;
        case ESP_SPP_CLOSE_EVT:
            printf("❌ Bluetooth device disconnected!\n");
            break;
        case ESP_SPP_DATA_IND_EVT:
            printf("📥 Received Data: %.*s\n", param->data_ind.len, param->data_ind.data);
            break;
        default:
            break;
    }
}
