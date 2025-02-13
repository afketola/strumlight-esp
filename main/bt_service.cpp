#include "bt_service.h"
#include <cstdio>
#include "nvs_flash.h"
#include "esp_bt.h"
#include "esp_bt_main.h"
#include "esp_gap_bt_api.h"
#include "esp_spp_api.h"
#include "esp_err.h"

#define DEVICE_NAME "StrumLight_WROVER"

// Forward-declare a function that sends data; implemented below
static void sendDummyData(void);

// Flow control state
static bool s_congested = false;
static uint32_t s_spp_handle = 0; // SPP handle for writing data

class BluetoothService {
public:
    static void initBT();
    static void btEventHandler(esp_spp_cb_event_t event, esp_spp_cb_param_t *param);
};

extern "C" void initBT(void) {
    BluetoothService::initBT();
}

extern "C" void bt_event_handler(esp_spp_cb_event_t event, esp_spp_cb_param_t *param) {
    BluetoothService::btEventHandler(event, param);
}

//------------------------------------------------------------------------------

void BluetoothService::initBT() {
    std::printf("🚀 Initializing Classic Bluetooth...\n");
    esp_err_t ret;

    // 1) Initialize NVS
    ret = nvs_flash_init();
    if (ret == ESP_ERR_NVS_NO_FREE_PAGES || ret == ESP_ERR_NVS_NEW_VERSION_FOUND) {
        ESP_ERROR_CHECK(nvs_flash_erase());
        ret = nvs_flash_init();
    }
    if (ret != ESP_OK) {
        std::printf("❌ Failed to initialize NVS! Error: %d\n", ret);
        return;
    }

    // 2) Reset existing Bluetooth
    if (esp_bluedroid_get_status() == ESP_BLUEDROID_STATUS_ENABLED) {
        std::printf("🔄 Deinitializing existing Bluetooth instance...\n");
        esp_bluedroid_disable();
        esp_bluedroid_deinit();
        esp_bt_controller_disable();
        esp_bt_controller_deinit();
    }

    // 3) Initialize the BT controller
    esp_bt_controller_config_t bt_cfg = BT_CONTROLLER_INIT_CONFIG_DEFAULT();
    ret = esp_bt_controller_init(&bt_cfg);
    if (ret != ESP_OK) {
        std::printf("❌ Failed to initialize Bluetooth controller! Error: %d\n", ret);
        return;
    }

    // 4) Enable Dual-mode (for a moment; we’ll release BLE next if not needed)
    ret = esp_bt_controller_enable(ESP_BT_MODE_BTDM);
    if (ret != ESP_OK) {
        std::printf("❌ Failed to enable Bluetooth controller! Error: %d\n", ret);
        return;
    }

    // ---- Release BLE memory if you ONLY use Classic Bluetooth ----
    // This can free up some heap used by BLE, mitigating memory fragmentation.
    ret = esp_bt_controller_mem_release(ESP_BT_MODE_BLE);
    if (ret != ESP_OK) {
        std::printf("❌ Failed to release BLE memory! Error: %d\n", ret);
        // Not fatal, but try to continue
    }

    // 5) Initialize Bluedroid
    ret = esp_bluedroid_init();
    if (ret != ESP_OK) {
        std::printf("❌ Failed to initialize Bluedroid stack! Error: %d\n", ret);
        return;
    }

    ret = esp_bluedroid_enable();
    if (ret != ESP_OK) {
        std::printf("❌ Failed to enable Bluedroid stack! Error: %d\n", ret);
        return;
    }

    // 6) Set device name and discovery mode
    ret = esp_bt_gap_set_device_name(DEVICE_NAME);
    if (ret != ESP_OK) {
        std::printf("❌ Failed to set Bluetooth device name! Error: %d\n", ret);
        return;
    }

    ret = esp_bt_gap_set_scan_mode(ESP_BT_CONNECTABLE, ESP_BT_GENERAL_DISCOVERABLE);
    if (ret != ESP_OK) {
        std::printf("❌ Failed to set Bluetooth scan mode! Error: %d\n", ret);
        return;
    }

    // 7) Register SPP event handler
    ret = esp_spp_register_callback(bt_event_handler);
    if (ret != ESP_OK) {
        std::printf("❌ Failed to register SPP event handler! Error: %d\n", ret);
        return;
    }

    // 8) Initialize SPP with a defined TX buffer size
    // You can try increasing or decreasing this value.
    esp_spp_cfg_t spp_cfg = {
        .mode = ESP_SPP_MODE_CB,
        .enable_l2cap_ertm = false,
        .tx_buffer_size = 512  // Example: 512 bytes
    };
    ret = esp_spp_enhanced_init(&spp_cfg);
    if (ret != ESP_OK) {
        std::printf("❌ Failed to initialize SPP! Error: %d\n", ret);
        return;
    }

    std::printf("✅ Classic Bluetooth initialized as '%s' and is now DISCOVERABLE!\n", DEVICE_NAME);
}

void BluetoothService::btEventHandler(esp_spp_cb_event_t event, esp_spp_cb_param_t *param) {
    switch (event) {
        case ESP_SPP_OPEN_EVT:
            std::printf("✅ Bluetooth device connected!\n");
            // Save the SPP handle so we can write data
            s_spp_handle = param->srv_open.handle;
            // Reset congestion
            s_congested = false;
            // Optionally, start sending some test data
            sendDummyData();
            break;

        case ESP_SPP_CLOSE_EVT:
            std::printf("❌ Bluetooth device disconnected!\n");
            s_spp_handle = 0;
            break;

        case ESP_SPP_DATA_IND_EVT:
            std::printf("📥 Received Data: %.*s\n", param->data_ind.len, param->data_ind.data);
            break;

        case ESP_SPP_CONG_EVT:
            // Called when the connection becomes congested/uncongested
            if (param->cong.cong) {
                std::printf("⚠️ SPP congested; can't send more data now\n");
                s_congested = true;
            } else {
                std::printf("✅ SPP uncongested; resume sending\n");
                s_congested = false;
                // Send again or continue from where we left off
                sendDummyData();
            }
            break;

        default:
            break;
    }
}

//------------------------------------------------------------------------------

/**
 * @brief Example function that attempts to write data over SPP.
 *        Checks congestion state before writing.
 */
static void sendDummyData(void) {
    if (s_congested || s_spp_handle == 0) {
        return; // Wait until uncongested or connected
    }

    const char* msg = "Hello from ESP32!\n";
    size_t len = strlen(msg);

    esp_err_t err = esp_spp_write(s_spp_handle, len, (uint8_t*)msg);
    if (err == ESP_OK) {
        std::printf("📤 SPP data sent: %s", msg);
    } else {
        std::printf("❌ esp_spp_write() failed, err= %d\n", err);
    }
}
