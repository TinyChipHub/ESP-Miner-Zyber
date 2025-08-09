#include "thermal.h"
#include "EMC2101.h"
#include "esp_log.h"

#define INTERNAL_OFFSET 5 //degrees C

static const char * TAG = "Thermal";

esp_err_t Thermal_init(DeviceModel device_model, int board_version, bool polarity) {
        //init the EMC2101, if we have one
    switch (device_model) {
        case DEVICE_ZYBER8S:
        case DEVICE_ZYBER8G:
            TMP1075_init(board_version);
            EMC2302_init(true);
            if(board_version == 1110){
                EMC2101_init(true);
            }
        default:
    }
    return ESP_OK;
    
}

//percent is a float between 0.0 and 1.0
esp_err_t Thermal_set_fan_percent(DeviceModel device_model, int board_version, float fan_percent, float beneath_fan_percent) {

    switch (device_model) {
        case DEVICE_ZYBER8S:
        case DEVICE_ZYBER8G:
            if(board_version == 1110){
                EMC2302_set_fan_speed(1,fan_percent);
                EMC2302_set_fan_speed(0,fan_percent);
                EMC2101_set_fan_speed(beneath_fan_percent); //We set the rear fan to 85% fix rate
            }else{
                EMC2302_set_fan_speed(1,fan_percent);
                EMC2302_set_fan_speed(0,beneath_fan_percent);  //We set the rear fan to 85% fix rate
            }
            break;
        default:
    }
    return ESP_OK;
}

uint16_t Thermal_get_fan_speed(DeviceModel device_model) {
    switch (device_model) {
        case DEVICE_ZYBER8S:
        case DEVICE_ZYBER8G:
            return EMC2302_get_fan_speed(0);    // May not be the best way to only get the first fan speed;
        default:
    }
    return 0;
}

float Thermal_get_chip_temp(GlobalState * GLOBAL_STATE) {
    if (!GLOBAL_STATE->ASIC_initalized) {
        return -1;
    }

    switch (GLOBAL_STATE->device_model) {
        case DEVICE_ZYBER8S:
        case DEVICE_ZYBER8G:
            if(GLOBAL_STATE->board_version == 1110){
                //uint8_t intt = EMC2101_get_external_temp();
                //ESP_LOGW(TAG, "External temp: %d", (int)intt);
                return TMP1075_read_temperature(0)+INTERNAL_OFFSET; //TMP1075 is on the front side
            }
            return TMP1075_read_temperature_weighted();
        default:
    }

    return -1;
}