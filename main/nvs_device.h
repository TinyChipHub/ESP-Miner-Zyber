#ifndef MAIN_NVS_DEVICE_H
#define MAIN_NVS_DEVICE_H

#include <stdbool.h>
#include "esp_err.h"
#include "global_state.h"

typedef struct RunningModeSetting{
    uint16_t frequency;
    uint16_t core_voltage;
}RunningModeSettings[3];

esp_err_t NVSDevice_init(void);
esp_err_t NVSDevice_parse_config(GlobalState *);
esp_err_t NVSDevice_get_wifi_creds(GlobalState *, char **, char **, char **);
esp_err_t NVSDevice_get_running_mode_setting(GlobalState *, uint16_t mode, uint16_t *freq, uint16_t *cv);

#endif // MAIN_NVS_DEVICE_H
