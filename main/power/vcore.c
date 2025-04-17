#include <stdio.h>
#include <math.h>
#include "esp_log.h"
#include "esp_check.h"

#include "vcore.h"
#include "adc.h"
#include "TPS546.h"


#define GPIO_ASIC_ENABLE CONFIG_GPIO_ASIC_ENABLE
#define GPIO_ASIC_RESET  CONFIG_GPIO_ASIC_RESET
#define GPIO_PLUG_SENSE  CONFIG_GPIO_PLUG_SENSE


static TPS546_CONFIG TPS546_CONFIG_ZYBER = {
    /* vin voltage */
    .TPS546_INIT_VIN_ON = 11.5,
    .TPS546_INIT_VIN_OFF = 11.0,
    .TPS546_INIT_VIN_UV_WARN_LIMIT = 0, //Set to 0 to ignore. TI Bug in this register
    .TPS546_INIT_VIN_OV_FAULT_LIMIT = 14.0,
    /* vout voltage */
    .TPS546_INIT_SCALE_LOOP = 0.125,
    .TPS546_INIT_VOUT_MIN = 2.5,
    .TPS546_INIT_VOUT_MAX = 5.0,
    .TPS546_INIT_VOUT_COMMAND = 3.6,
    /* iout current */
    .TPS546_INIT_IOUT_OC_WARN_LIMIT = 80.00, /* A */
    .TPS546_INIT_IOUT_OC_FAULT_LIMIT = 85.00 /* A */
};

static const char *TAG = "vcore.c";

esp_err_t VCORE_init(GlobalState * GLOBAL_STATE) {
    switch (GLOBAL_STATE->device_model) {
        case DEVICE_ZYBER8S:
        case DEVICE_ZYBER8G:
            ESP_RETURN_ON_ERROR(TPS546_init(TPS546_CONFIG_ZYBER), TAG, "TPS546 init failed!");
            break;
        default:
    }

    return ESP_OK;
}

esp_err_t VCORE_set_voltage(float core_voltage, GlobalState * global_state)
{
    switch (global_state->device_model) {
        case DEVICE_ZYBER8S:
        case DEVICE_ZYBER8G:
            ESP_LOGI(TAG, "Set ASIC voltage = %.3fV", core_voltage);
            ESP_RETURN_ON_ERROR(TPS546_set_vout(core_voltage * 4), TAG, "TPS546 set voltage failed!");
        default:
    }

    return ESP_OK;
}

int16_t VCORE_get_voltage_mv(GlobalState * global_state) {

    switch (global_state->device_model) {
        case DEVICE_ZYBER8S:
        case DEVICE_ZYBER8G:
            return (TPS546_get_vout() * 1000) / 4;
        // case DEVICE_HEX:
        default:
    }
    return -1;
}

esp_err_t VCORE_check_fault(GlobalState * global_state) {

    switch (global_state->device_model) {
        case DEVICE_ZYBER8S:
        case DEVICE_ZYBER8G:
            ESP_RETURN_ON_ERROR(TPS546_check_status(global_state), TAG, "TPS546 check status failed!");
            break;
        // case DEVICE_HEX:
        default:
    }
    return ESP_OK;
}

const char* VCORE_get_fault_string(GlobalState * global_state) {
    switch (global_state->device_model) {
        case DEVICE_ZYBER8S:
        case DEVICE_ZYBER8G:
            return TPS546_get_error_message();
        default:
    }
    return NULL;
}

