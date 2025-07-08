#include <string.h>
#include "esp_event.h"
#include "esp_log.h"
#include "esp_err.h"
#include "esp_check.h"
#include "nvs_flash.h"
#include "nvs_config.h"
#include "nvs_device.h"

#include "connect.h"
#include "global_state.h"

static const char * TAG = "nvs_device";

static RunningModeSettings ZYBER8S_SETTINGS = {
    {490,1166},
    {575,1180},
    {600,1200},
};

static RunningModeSettings ZYBER8G_SETTINGS = {
    {525,1150},
    {600,1150},
    {625,1170},
};



esp_err_t NVSDevice_init(void) {
    esp_err_t err = nvs_flash_init();
    if (err == ESP_ERR_NVS_NO_FREE_PAGES) {
        ESP_ERROR_CHECK(nvs_flash_erase());
        err = nvs_flash_init();
    }
    return err;
}

esp_err_t NVSDevice_get_wifi_creds(GlobalState * GLOBAL_STATE, char ** wifi_ssid, char ** wifi_pass, char ** hostname) {
    // pull the wifi credentials and hostname out of NVS
    *wifi_ssid = nvs_config_get_string(NVS_CONFIG_WIFI_SSID, WIFI_SSID);
    *wifi_pass = nvs_config_get_string(NVS_CONFIG_WIFI_PASS, WIFI_PASS);
    *hostname  = nvs_config_get_string(NVS_CONFIG_HOSTNAME, HOSTNAME);

    // copy the wifi ssid to the global state
    strncpy(GLOBAL_STATE->SYSTEM_MODULE.ssid, *wifi_ssid, sizeof(GLOBAL_STATE->SYSTEM_MODULE.ssid));
    GLOBAL_STATE->SYSTEM_MODULE.ssid[sizeof(GLOBAL_STATE->SYSTEM_MODULE.ssid)-1] = 0;

    return ESP_OK;
}

esp_err_t NVSDevice_parse_config(GlobalState * GLOBAL_STATE) {

    GLOBAL_STATE->POWER_MANAGEMENT_MODULE.frequency_value = nvs_config_get_u16(NVS_CONFIG_ASIC_FREQ, CONFIG_ASIC_FREQUENCY);
    ESP_LOGI(TAG, "NVS_CONFIG_ASIC_FREQ %f", (float)GLOBAL_STATE->POWER_MANAGEMENT_MODULE.frequency_value);

    GLOBAL_STATE->asic_model_str = nvs_config_get_string(NVS_CONFIG_ASIC_MODEL, "");
    GLOBAL_STATE->device_model_str = nvs_config_get_string(NVS_CONFIG_DEVICE_MODEL, "invalid");
    char * board_version = nvs_config_get_string(NVS_CONFIG_BOARD_VERSION, "000");
    GLOBAL_STATE->board_version = atoi(board_version);
    GLOBAL_STATE->screen_saver_time = nvs_config_get_u16(NVS_CONFIG_SCREEN_SAVER_TIME, 10);
    GLOBAL_STATE->chips_reset_retart = nvs_config_get_u16(NVS_CONFIG_CHIP_RESET_RESTART, 1);

    free(board_version);
    ESP_LOGI(TAG, "Found Device Model: %s", GLOBAL_STATE->device_model_str);
    ESP_LOGI(TAG, "Found Board Version: %d", GLOBAL_STATE->board_version);
    
    if (strcmp(GLOBAL_STATE->device_model_str, "hex") == 0||
            strcmp(GLOBAL_STATE->device_model_str, "suprahex") == 0) {
        GLOBAL_STATE->is_multichip=true;
    }

    return ESP_OK;
}

esp_err_t NVSDevice_get_running_mode_setting(GlobalState * GLOBAL_STATE, uint16_t mode, uint16_t *freq, uint16_t *cv){
    switch(GLOBAL_STATE->device_model){
        case DEVICE_ZYBER8S:
            *freq = ZYBER8S_SETTINGS[mode].frequency;
            *cv = ZYBER8S_SETTINGS[mode].core_voltage;
            break;
        case DEVICE_ZYBER8G:
            *freq = ZYBER8G_SETTINGS[mode].frequency;
            *cv = ZYBER8G_SETTINGS[mode].core_voltage;
            break;
        case DEVICE_UNKNOWN:
            ESP_LOGW(TAG, "No setting for Unkonwn device");
            break;
    }
    return ESP_OK;
}