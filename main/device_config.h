#ifndef DEVICE_CONFIG_H_
#define DEVICE_CONFIG_H_

#include <stdint.h>
#include <stdbool.h>
#include "esp_err.h"

#define THERMAL_MAX_SENSORS 2

typedef enum
{
    BM1397,
    BM1366,
    BM1368,
    BM1370,
} Asic;

typedef struct {
    Asic id;
    const char * name;
    uint16_t chip_id;
    uint16_t default_frequency_mhz;
    const uint16_t* frequency_options;
    uint16_t default_voltage_mv;
    const uint16_t* voltage_options;
    uint16_t hashrate_target;
    uint16_t difficulty;
    uint16_t core_count;
    uint16_t small_core_count;
    uint8_t hash_domains;
    // test values
    float hashrate_test_percentage_target;
} AsicConfig;

typedef enum
{
    ZYBER8S,
    ZYBER8G,
    ZYBER8GPLUS,
} Family;

typedef struct {
    Family id;
    const char * name;
    AsicConfig asic;
    uint8_t asic_count;
    uint16_t max_power;
    uint16_t power_offset;
    uint16_t nominal_voltage;
    uint16_t voltage_domains;
    const char * swarm_color;
} FamilyConfig;

typedef struct {
    const char * board_version;
    FamilyConfig family;
    bool plug_sense;
    bool asic_enable;
    bool EMC2101 : 1;
    bool EMC2103 : 1;
    bool EMC2302 : 1;
    bool has_three_fan: 1;
    bool emc_internal_temp : 1;
    bool TMP1075 : 1;
    uint8_t emc_ideality_factor;
    uint8_t emc_beta_compensation;
    int8_t temp_offset;
    bool DS4432U : 1;
    bool INA260  : 1;
    bool TPS546  : 1;
    // test values
    uint16_t power_consumption_target;
} DeviceConfig;

static const uint16_t BM1397_FREQUENCY_OPTIONS[] = {400, 425, 450, 475, 485, 500, 525, 550, 575, 600, 0};
static const uint16_t BM1366_FREQUENCY_OPTIONS[] = {400, 425, 450, 475, 485, 500, 525, 550, 575,      0};
static const uint16_t BM1368_FREQUENCY_OPTIONS[] = {400, 425, 450, 475, 485, 490, 500, 525, 550, 575, 0};
static const uint16_t BM1370_FREQUENCY_OPTIONS[] = {400, 490, 525, 550, 600, 625,                     0};

static const uint16_t BM1397_VOLTAGE_OPTIONS[] = {1100, 1150, 1200, 1250, 1300, 1350, 1400, 1450, 1500, 0};
static const uint16_t BM1366_VOLTAGE_OPTIONS[] = {1100, 1150, 1200, 1250, 1300,                         0};
static const uint16_t BM1368_VOLTAGE_OPTIONS[] = {1100, 1150, 1166, 1200, 1250, 1300,                   0};
static const uint16_t BM1370_VOLTAGE_OPTIONS[] = {1000, 1060, 1100, 1150, 1200, 1250,                   0};

static const AsicConfig ASIC_BM1397 = { .id = BM1397, .name = "BM1397", .chip_id = 1397, .default_frequency_mhz = 425, .frequency_options = BM1397_FREQUENCY_OPTIONS, .default_voltage_mv = 1400, .voltage_options = BM1397_VOLTAGE_OPTIONS, .difficulty = 256, .core_count = 168, .small_core_count =  672, .hash_domains = 0, .hashrate_test_percentage_target = 0.85, };
static const AsicConfig ASIC_BM1366 = { .id = BM1366, .name = "BM1366", .chip_id = 1366, .default_frequency_mhz = 485, .frequency_options = BM1366_FREQUENCY_OPTIONS, .default_voltage_mv = 1200, .voltage_options = BM1366_VOLTAGE_OPTIONS, .difficulty = 256, .core_count = 112, .small_core_count =  894, .hash_domains = 4, .hashrate_test_percentage_target = 0.85, };
static const AsicConfig ASIC_BM1368 = { .id = BM1368, .name = "BM1368", .chip_id = 1368, .default_frequency_mhz = 490, .frequency_options = BM1368_FREQUENCY_OPTIONS, .default_voltage_mv = 1166, .voltage_options = BM1368_VOLTAGE_OPTIONS, .difficulty = 256, .core_count =  80, .small_core_count = 1276, .hash_domains = 4, .hashrate_test_percentage_target = 0.80, };
static const AsicConfig ASIC_BM1370 = { .id = BM1370, .name = "BM1370", .chip_id = 1370, .default_frequency_mhz = 525, .frequency_options = BM1370_FREQUENCY_OPTIONS, .default_voltage_mv = 1150, .voltage_options = BM1370_VOLTAGE_OPTIONS, .difficulty = 256, .core_count = 128, .small_core_count = 2040, .hash_domains = 4, .hashrate_test_percentage_target = 0.85, };

static const AsicConfig default_asic_configs[] = {
    ASIC_BM1397,
    ASIC_BM1366,
    ASIC_BM1368,
    ASIC_BM1370,
};

static const FamilyConfig FAMILY_ZYBER8S    = { .id = ZYBER8S,    .name = "Zyber8S",   .asic = ASIC_BM1368, .asic_count = 8, .max_power = 160, .power_offset = 8,  .nominal_voltage = 12,  .voltage_domains = 4, .swarm_color = "blue",};
static const FamilyConfig FAMILY_ZYBER8G    = { .id = ZYBER8G,    .name = "Zyber8G",   .asic = ASIC_BM1370, .asic_count = 8, .max_power = 220, .power_offset = 8,  .nominal_voltage = 12,  .voltage_domains = 4, .swarm_color = "purple",};
static const FamilyConfig FAMILY_ZYBER8GPLUS    = { .id = ZYBER8GPLUS,    .name = "Zyber8GPlus",   .asic = ASIC_BM1370, .asic_count = 12, .max_power = 255, .power_offset = 10,  .nominal_voltage = 12,  .voltage_domains = 4, .swarm_color = "cherry",};

static const FamilyConfig default_families[] = {
    FAMILY_ZYBER8S,
    FAMILY_ZYBER8G,
    FAMILY_ZYBER8GPLUS,
};

static const DeviceConfig default_configs[] = {
    { .board_version = "1000",  .family = FAMILY_ZYBER8S,     .EMC2302 = true, .TMP1075 = true,                                            .temp_offset = 2,  .TPS546 = true,                                                           .power_consumption_target = 110, },
    { .board_version = "1100",  .family = FAMILY_ZYBER8G,     .EMC2302 = true, .TMP1075 = true,                                            .temp_offset = 2,  .TPS546 = true,                                                           .power_consumption_target = 135, },
    { .board_version = "1110",  .family = FAMILY_ZYBER8G,     .EMC2101 = true, .EMC2302 = true, .emc_ideality_factor = 0x32, .emc_beta_compensation = 0x00, .TMP1075 = true, .has_three_fan=true,      .temp_offset = 2,  .TPS546 = true,  .emc_internal_temp=false,   .power_consumption_target = 165, },
    { .board_version = "1200",  .family = FAMILY_ZYBER8GPLUS, .EMC2101 = true, .EMC2302 = true, .emc_ideality_factor = 0x32, .emc_beta_compensation = 0x00, .TMP1075 = true, .has_three_fan=true,      .temp_offset = 2,  .TPS546 = true,  .emc_internal_temp=true,   .power_consumption_target = 235, },
};

esp_err_t device_config_init(void * pvParameters);

#endif /* DEVICE_CONFIG_H_ */
