#include "TPS546.h"

#include "power.h"
#include "vcore.h"

#define GPIO_ASIC_ENABLE CONFIG_GPIO_ASIC_ENABLE

#define SUPRA_POWER_OFFSET 5 //Watts
#define GAMMA_POWER_OFFSET 5 //Watts
#define GAMMATURBO_POWER_OFFSET 5 //Watts

// max power settings
#define ZYBER8S_MAX_POWER 180 //Watts

// nominal voltage settings
#define NOMINAL_VOLTAGE_5 5 //volts
#define NOMINAL_VOLTAGE_12 12//volts

esp_err_t Power_disable(GlobalState * GLOBAL_STATE) {

    switch (GLOBAL_STATE->device_model) {
        case DEVICE_ZYBER8S:
        case DEVICE_ZYBER8G:
            // Turn off core voltage
            VCORE_set_voltage(0.0, GLOBAL_STATE);
            break;
        default:
    }
    return ESP_OK;

}

float Power_get_max_settings(GlobalState * GLOBAL_STATE) {

    switch (GLOBAL_STATE->device_model) {
        case DEVICE_ZYBER8S:
            return ZYBER8S_MAX_POWER;
        default:
            return ZYBER8S_MAX_POWER;
    }
}

float Power_get_current(GlobalState * GLOBAL_STATE) {
    float current = 0.0;

    switch (GLOBAL_STATE->device_model) {
        case DEVICE_ZYBER8S:
            current = TPS546_get_iout() * 1000.0;
            break;
        default:
    }

    return current;
}

float Power_get_power(GlobalState * GLOBAL_STATE) {
    float power = 0.0;
    float current = 0.0;
    PowerManagementModule * power_management = &GLOBAL_STATE->POWER_MANAGEMENT_MODULE;
    switch (GLOBAL_STATE->device_model) {
        case DEVICE_ZYBER8S:
        case DEVICE_ZYBER8G:
                current = TPS546_get_iout() * 1000.0;
                power_management->current=current;
                // calculate regulator power (in milliwatts)
                power = (TPS546_get_vout() * current) / 1000.0;
                // The power reading from the TPS546 is only it's output power. So the rest of the Bitaxe power is not accounted for.
                power += GAMMATURBO_POWER_OFFSET; // Add offset for the rest of the Bitaxe power. TODO: this better.
            break;
        default:
    }

    return power;
}

float Power_get_input_voltage(GlobalState * GLOBAL_STATE) {

    switch (GLOBAL_STATE->device_model) {
        case DEVICE_ZYBER8S:
        case DEVICE_ZYBER8G:
                return TPS546_get_vin() * 1000.0;
            break;
        default:
    }

    return 0.0;
}

float Power_get_output_voltage(GlobalState * GLOBAL_STATE) {
    switch (GLOBAL_STATE->device_model) {
        case DEVICE_ZYBER8S:
        case DEVICE_ZYBER8G:
                return TPS546_get_vout();
            break;
        default:
    }

    return 0.0;
}

int Power_get_nominal_voltage(GlobalState * GLOBAL_STATE) {
    switch (GLOBAL_STATE->device_model)
    {
        case DEVICE_ZYBER8S:
        case DEVICE_ZYBER8G:
            return NOMINAL_VOLTAGE_12;
        default:
        return NOMINAL_VOLTAGE_12;
    }
}

float Power_get_vreg_temp(GlobalState * GLOBAL_STATE) {

    switch (GLOBAL_STATE->device_model) {
        case DEVICE_ZYBER8S:
        case DEVICE_ZYBER8G:
                return TPS546_get_temperature();
            break;
        default:
    }

    return 0.0;
}
