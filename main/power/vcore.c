#include <stdio.h>
#include <math.h>
#include "esp_check.h"
#include "esp_log.h"

#include "vcore.h"
#include "adc.h"
#include "TPS546.h"
#include "driver/gpio.h"

#define GPIO_ASIC_ENABLE CONFIG_GPIO_ASIC_ENABLE
#define GPIO_PLUG_SENSE  CONFIG_GPIO_PLUG_SENSE

static const char *TAG = "vcore";

static TPS546_CONFIG TPS546_CONFIG_DEFAULT = {
    /* vin voltage */
    .TPS546_INIT_VIN_ON = 11.5,
    .TPS546_INIT_VIN_OFF = 10.5,
    .TPS546_INIT_VIN_UV_WARN_LIMIT = 11.0,
    .TPS546_INIT_VIN_OV_FAULT_LIMIT = 14.0,
    /* vout voltage */
    .TPS546_INIT_SCALE_LOOP = 0.125,
    .TPS546_INIT_VOUT_MIN = 2.5,
    .TPS546_INIT_VOUT_MAX = 5.5,
    .TPS546_INIT_VOUT_COMMAND = 4.8,
    /* iout current */
    .TPS546_INIT_IOUT_OC_WARN_LIMIT = 80.00, /* A */
    .TPS546_INIT_IOUT_OC_FAULT_LIMIT = 85.00 /* A */
};

esp_err_t VCORE_init(GlobalState * GLOBAL_STATE)
{
    ESP_RETURN_ON_FALSE(GLOBAL_STATE->DEVICE_CONFIG.family.voltage_domains != 0, ESP_FAIL, TAG, "voltage_domains not defined");

    if (GLOBAL_STATE->DEVICE_CONFIG.TPS546) {
        switch (GLOBAL_STATE->DEVICE_CONFIG.family.id) {
            case ZYBER8S:
            case ZYBER8G:
                ESP_RETURN_ON_ERROR(TPS546_init(TPS546_CONFIG_DEFAULT), TAG, "TPS546 init failed!");
                break;
        }
    }

    if (GLOBAL_STATE->DEVICE_CONFIG.plug_sense) {
        gpio_config_t barrel_jack_conf = {
            .pin_bit_mask = (1ULL << GPIO_PLUG_SENSE),
            .mode = GPIO_MODE_INPUT,
        };
        gpio_config(&barrel_jack_conf);
        int barrel_jack_plugged_in = gpio_get_level(GPIO_PLUG_SENSE);

        gpio_set_direction(GPIO_ASIC_ENABLE, GPIO_MODE_OUTPUT);
        if (barrel_jack_plugged_in == 1 || GLOBAL_STATE->DEVICE_CONFIG.asic_enable) {
            gpio_set_level(GPIO_ASIC_ENABLE, 0);
        } else {
            // turn ASIC off
            gpio_set_level(GPIO_ASIC_ENABLE, 1);
        }
    }

    return ESP_OK;
}

esp_err_t VCORE_set_voltage(GlobalState * GLOBAL_STATE, float core_voltage)
{
    ESP_LOGI(TAG, "Set ASIC voltage = %.3fV", core_voltage);
 
    uint16_t voltage_domains = GLOBAL_STATE->DEVICE_CONFIG.family.voltage_domains;
    ESP_RETURN_ON_ERROR(TPS546_set_vout(core_voltage * voltage_domains), TAG, "TPS546 set voltage failed!");

    if (core_voltage == 0.0f && GLOBAL_STATE->DEVICE_CONFIG.asic_enable) {
        gpio_set_level(GPIO_ASIC_ENABLE, 1);
    }

    return ESP_OK;
}

int16_t VCORE_get_voltage_mv(GlobalState * GLOBAL_STATE) 
{
    if (GLOBAL_STATE->DEVICE_CONFIG.TPS546) {
        return TPS546_get_vout() / GLOBAL_STATE->DEVICE_CONFIG.family.voltage_domains * 1000;
    }
    return ADC_get_vcore();
}

esp_err_t VCORE_check_fault(GlobalState * GLOBAL_STATE) 
{
    if (GLOBAL_STATE->DEVICE_CONFIG.TPS546) {
        ESP_RETURN_ON_ERROR(TPS546_check_status(GLOBAL_STATE), TAG, "TPS546 check status failed!");
    }
    return ESP_OK;
}

const char* VCORE_get_fault_string(GlobalState * GLOBAL_STATE)
{
    if (GLOBAL_STATE->DEVICE_CONFIG.TPS546) {
        return TPS546_get_error_message();
    }
    return NULL;
}
