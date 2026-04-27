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

static TPS546_CONFIG get_tps546_config(const FamilyConfig * family)
{
    TPS546_CONFIG config = {0};

    // Set family-specific parameters
    switch (family->id) {
    case ZYBER8S:
    case ZYBER8G:
    case ZYBER8GPLUS:
        config.TPS546_INIT_PHASE = TPS546_INIT_PHASE_MULTI;
        config.TPS546_INIT_VIN_ON = 11.0;
        config.TPS546_INIT_VIN_OFF = 10.5;
        config.TPS546_INIT_VIN_UV_WARN_LIMIT = 11.0;
        config.TPS546_INIT_VIN_OV_FAULT_LIMIT = 14.0;
        config.TPS546_INIT_SCALE_LOOP = 0.25;
        config.TPS546_INIT_VOUT_MIN = 1;
        config.TPS546_INIT_VOUT_MAX = 3;
        config.TPS546_INIT_VOUT_COMMAND = 1.2;
        config.TPS546_INIT_IOUT_OC_WARN_LIMIT = 70.00;
        config.TPS546_INIT_IOUT_OC_FAULT_LIMIT = 80.00;
        // Multi-phase stacking configuration for 2 TPS modules
        config.TPS546_INIT_STACK_CONFIG = 0x0001; // 2 modules (One-Slave, 2-phase)
        config.TPS546_INIT_SYNC_CONFIG = 0xD0;    // Enable Auto Detect SYNC
        config.TPS546_INIT_COMPENSATION_CONFIG[0] = 0x12;
        config.TPS546_INIT_COMPENSATION_CONFIG[1] = 0x34;
        config.TPS546_INIT_COMPENSATION_CONFIG[2] = 0x42;
        config.TPS546_INIT_COMPENSATION_CONFIG[3] = 0x21;
        config.TPS546_INIT_COMPENSATION_CONFIG[4] = 0x04;
        break;
    }

    return config;
}

esp_err_t VCORE_init(GlobalState * GLOBAL_STATE)
{
    ESP_RETURN_ON_FALSE(GLOBAL_STATE->DEVICE_CONFIG.family.voltage_domains != 0, ESP_FAIL, TAG, "voltage_domains not defined");

    if (GLOBAL_STATE->DEVICE_CONFIG.TPS546) {
        switch (GLOBAL_STATE->DEVICE_CONFIG.family.id) {
            case ZYBER8S:
            case ZYBER8G:
                ESP_RETURN_ON_ERROR(TPS546_init(get_tps546_config(&GLOBAL_STATE->DEVICE_CONFIG.family)), TAG, "TPS546 init failed!");
                break;
            case ZYBER8GPLUS:
                ESP_RETURN_ON_ERROR(TPS546_init(get_tps546_config(&GLOBAL_STATE->DEVICE_CONFIG.family)), TAG, "TPS546 init failed!");
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
