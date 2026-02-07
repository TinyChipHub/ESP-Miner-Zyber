#include <string.h>
#include <esp_heap_caps.h>
#include <math.h>
#include "esp_log.h"
#include "esp_timer.h"
#include "system.h"
#include "common.h"
#include "asic.h"
#include "utils.h"
#include "asic_init.h"
#include "driver/uart.h"
#include "vcore.h"
#include "nvs_config.h"

#define EPSILON 0.0001f

#define POLL_RATE 5000

#define HASHRATE_UNIT 0x100000uLL // Hashrate register unit (2^24 hashes)

static const char *TAG = "hashrate_monitor";
static uint8_t hashrateErrorCount = 0;
static int reinitiateCount = 0;
static float lowerThresholdHashratePercent = 0.18f; // 82% of expected hashrate


static bool is_reinitialize=false;
static bool is_just_reinitialized=false;
static float expected_chip_hashrate;
static float expected_total_hashrate;
static float expected_domain_hashrate;

static float sum_hashrates(measurement_t * measurement, int asic_count)
{
    if (asic_count == 1) return measurement[0].hashrate;

    float total = 0;
    for (int asic_nr = 0; asic_nr < asic_count; asic_nr++) {
        total += measurement[asic_nr].hashrate;
    }
    return total;
}

static void clear_measurements(GlobalState * GLOBAL_STATE)
{
    HashrateMonitorModule * HASHRATE_MONITOR_MODULE = &GLOBAL_STATE->HASHRATE_MONITOR_MODULE;

    int asic_count = GLOBAL_STATE->DEVICE_CONFIG.family.asic_count;
    int hash_domains = GLOBAL_STATE->DEVICE_CONFIG.family.asic.hash_domains;

    memset(HASHRATE_MONITOR_MODULE->total_measurement, 0, asic_count * sizeof(measurement_t));
    if (hash_domains > 0) {
        memset(HASHRATE_MONITOR_MODULE->domain_measurements[0], 0, asic_count * hash_domains * sizeof(measurement_t));
    }
    memset(HASHRATE_MONITOR_MODULE->error_measurement, 0, asic_count * sizeof(measurement_t));
}

static void update_hashrate(uint32_t value, measurement_t * measurement, int asic_nr)
{
    uint8_t flag_long = (value & 0x80000000) >> 31;
    uint32_t hashrate_value = value & 0x7FFFFFFF;    

    if (hashrate_value != 0x007FFFFF && !flag_long) {
        float hashrate = hashrate_value * (float)HASHRATE_UNIT; // Make sure it stays in float
        float diff = fabsf(hashrate - expected_chip_hashrate)/expected_chip_hashrate;
        measurement[asic_nr].hashrate =  hashrate / 1e9f; // Convert to Gh/s
    }
}

static void update_hash_counter(uint32_t time_ms, uint32_t value, measurement_t * measurement)
{
    uint32_t previous_time_ms = measurement->time_ms;
    if (previous_time_ms != 0) {
        uint32_t duration_ms = time_ms - previous_time_ms;
        uint32_t counter = value - measurement->value; // Compute counter difference, handling uint32_t wraparound
        measurement->hashrate = hashCounterToGhs(duration_ms, counter);
    }

    measurement->value = value;
    measurement->time_ms = time_ms;
}

bool check_anomaly(void *pvParameters, float hashrate, float expected)
{
    if(is_reinitialize) return false;
    if(is_just_reinitialized) return false;

    GlobalState * GLOBAL_STATE = (GlobalState *)pvParameters;

    float diffPercent = fabs(expected - hashrate) / expected;
    if(hashrate==0 || diffPercent > lowerThresholdHashratePercent){
        hashrateErrorCount++;
    }else{
        return true;
    }

    if (hashrateErrorCount >= 5) { // If low hashrate detected 5 times consecutively
        is_reinitialize = true;
        reinitiateCount++;

        GLOBAL_STATE->ASIC_initalized = false;
        // Give tasks time to complete any current UART operation and notice the flag
        vTaskDelay(500 / portTICK_PERIOD_MS);
        ESP_LOGI(TAG, "Flushing UART buffers...");
        // flush driver to clear any stale data
        uart_flush(UART_NUM_1);
        vTaskDelay(100 / portTICK_PERIOD_MS);
        //clear_measurements(GLOBAL_STATE);
        
        //ESP_RETURN_ON_ERROR(TPS546_set_vout(core_voltage * voltage_domains), TAG, "TPS546 set voltage failed!");
        VCORE_set_voltage(GLOBAL_STATE, nvs_config_get_u16(NVS_CONFIG_ASIC_VOLTAGE) / 1000.0);

        // Perform live recovery
        // Stabilization delay of 2000ms prevents race conditions where tasks are just
        // starting to use ASIC while power management loop tries to change frequency
        uint8_t chip_count = asic_initialize(GLOBAL_STATE, ASIC_INIT_RECOVERY, 2000);
        
        is_reinitialize = false;
        is_just_reinitialized = true;
        hashrateErrorCount = 0; // Reset counter after reinitialization
        if(reinitiateCount%10==0){
            ESP_LOGI(TAG, "Hashrate anomaly detected and recovery performed %d times", reinitiateCount);
        }
    }

}

void hashrate_monitor_task(void *pvParameters)
{
    GlobalState * GLOBAL_STATE = (GlobalState *)pvParameters;
    HashrateMonitorModule * HASHRATE_MONITOR_MODULE = &GLOBAL_STATE->HASHRATE_MONITOR_MODULE;
    SystemModule * SYSTEM_MODULE = &GLOBAL_STATE->SYSTEM_MODULE;

    int asic_count = GLOBAL_STATE->DEVICE_CONFIG.family.asic_count;
    int hash_domains = GLOBAL_STATE->DEVICE_CONFIG.family.asic.hash_domains;

    expected_total_hashrate =0; 

    lowerThresholdHashratePercent = 1.0f-((expected_hashrate/asic_count/hash_domains*2.0f)/expected_hashrate);

    ESP_LOGI(TAG, "Calculated lower threshold hashrate percent: %.2f%%", lowerThresholdHashratePercent * 100.0f);

    HASHRATE_MONITOR_MODULE->total_measurement = heap_caps_malloc(asic_count * sizeof(measurement_t), MALLOC_CAP_SPIRAM);
    if (hash_domains > 0) {
        measurement_t* data = heap_caps_malloc(asic_count * hash_domains * sizeof(measurement_t), MALLOC_CAP_SPIRAM);
        HASHRATE_MONITOR_MODULE->domain_measurements = heap_caps_malloc(asic_count * sizeof(measurement_t*), MALLOC_CAP_SPIRAM);
        for (size_t asic_nr = 0; asic_nr < asic_count; asic_nr++) {
            HASHRATE_MONITOR_MODULE->domain_measurements[asic_nr] = data + (asic_nr * hash_domains);
        }
    }
    HASHRATE_MONITOR_MODULE->error_measurement = heap_caps_malloc(asic_count * sizeof(measurement_t), MALLOC_CAP_SPIRAM);

    clear_measurements(GLOBAL_STATE);

    HASHRATE_MONITOR_MODULE->is_initialized = true;

    TickType_t taskWakeTime = xTaskGetTickCount();
    while (1) {
        if(is_reinitialize){
            vTaskDelay(200 / portTICK_PERIOD_MS);
            continue;
        }
        if(is_just_reinitialized){
            is_just_reinitialized = false;
            vTaskDelay(POLL_RATE / portTICK_PERIOD_MS);
            continue;
        }

        if(expected_total_hashrate!=GLOBAL_STATE->POWER_MANAGEMENT_MODULE.expected_hashrate){
            expected_total_hashrate = GLOBAL_STATE->POWER_MANAGEMENT_MODULE.expected_hashrate;
            expected_chip_hashrate = expected_total_hashrate / (float)asic_count;
            expected_domain_hashrate = expected_hashrate;
            if(hash_domains>0){
                expected_domain_hashrate = expected_hashrate/(float)asic_count/(float)/hash_domains;
            }
        }

        ASIC_read_registers(GLOBAL_STATE);

        vTaskDelay(100 / portTICK_PERIOD_MS);

        float current_hashrate = sum_hashrates(HASHRATE_MONITOR_MODULE->total_measurement, asic_count);
        float error_hashrate = sum_hashrates(HASHRATE_MONITOR_MODULE->error_measurement, asic_count);

        SYSTEM_MODULE->current_hashrate = current_hashrate;
        // SYSTEM_MODULE->error_percentage = current_hashrate > 0 ? error_hashrate / current_hashrate * 100.f : 0;

        vTaskDelayUntil(&taskWakeTime, POLL_RATE / portTICK_PERIOD_MS);
    }
}

void hashrate_monitor_register_read(void *pvParameters, register_type_t register_type, uint8_t asic_nr, uint32_t value)
{
    uint32_t time_ms = esp_timer_get_time() / 1000;

    GlobalState * GLOBAL_STATE = (GlobalState *)pvParameters;
    HashrateMonitorModule * HASHRATE_MONITOR_MODULE = &GLOBAL_STATE->HASHRATE_MONITOR_MODULE;

    int asic_count = GLOBAL_STATE->DEVICE_CONFIG.family.asic_count;

    if (asic_nr >= asic_count) {
        ESP_LOGE(TAG, "Asic nr out of bounds [%d]", asic_nr);
        return;
    }

    switch(register_type) {
        case REGISTER_HASHRATE:
            update_hashrate(value, HASHRATE_MONITOR_MODULE->total_measurement, asic_nr);
            break;
        case REGISTER_TOTAL_COUNT:
            update_hash_counter(time_ms, value, &HASHRATE_MONITOR_MODULE->total_measurement[asic_nr]);
            break;
        case REGISTER_DOMAIN_0_COUNT:
            update_hash_counter(time_ms, value, &HASHRATE_MONITOR_MODULE->domain_measurements[asic_nr][0]);
            break;
        case REGISTER_DOMAIN_1_COUNT:
            update_hash_counter(time_ms, value, &HASHRATE_MONITOR_MODULE->domain_measurements[asic_nr][1]);
            break;
        case REGISTER_DOMAIN_2_COUNT:
            update_hash_counter(time_ms, value, &HASHRATE_MONITOR_MODULE->domain_measurements[asic_nr][2]);
            break;
        case REGISTER_DOMAIN_3_COUNT:
            update_hash_counter(time_ms, value, &HASHRATE_MONITOR_MODULE->domain_measurements[asic_nr][3]);
            break;
        case REGISTER_ERROR_COUNT:
            update_hash_counter(time_ms, value, &HASHRATE_MONITOR_MODULE->error_measurement[asic_nr]);
            break;
        case REGISTER_INVALID:
            ESP_LOGE(TAG, "Invalid register type");
            break;
    }
}

/*
    // From NerdAxe codebase, temparature conversion?
    if (asic_result.data & 0x80000000) {
        float ftemp = (float) (asic_result.data & 0x0000ffff) * 0.171342f - 299.5144f;
        ESP_LOGI(TAG, "asic %d temp: %.3f", (int) asic_result.asic_nr, ftemp);
        board->setChipTemp(asic_result.asic_nr, ftemp);
    }
    break;
*/
