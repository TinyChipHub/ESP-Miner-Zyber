#include <string.h>

#include <esp_log.h>

#include "bm1397.h"
#include "bm1366.h"
#include "bm1368.h"
#include "bm1370.h"

#include "asic.h"


static const char *TAG = "asic";

// .init_fn = BM1366_init,
uint8_t ASIC_init(GlobalState * GLOBAL_STATE) {
    switch (GLOBAL_STATE->device_model) {
        case DEVICE_ZYBER8G:
            return BM1370_init(GLOBAL_STATE->POWER_MANAGEMENT_MODULE.frequency_value, ZYBER8G_ASIC_COUNT);
        case DEVICE_ZYBER8S:
            return BM1368_init(GLOBAL_STATE->POWER_MANAGEMENT_MODULE.frequency_value, ZYBER8S_ASIC_COUNT); 
        default:
    }
    return ESP_OK;
}

uint8_t ASIC_get_asic_count(GlobalState * GLOBAL_STATE) {
    switch (GLOBAL_STATE->device_model) {
        case DEVICE_ZYBER8G:
            return ZYBER8G_ASIC_COUNT;
        case DEVICE_ZYBER8S:
            return ZYBER8S_ASIC_COUNT;
        default:
    }
    return 0;
}

uint16_t ASIC_get_small_core_count(GlobalState * GLOBAL_STATE) {
    switch (GLOBAL_STATE->device_model) {
        case DEVICE_ZYBER8S:
            return BM1368_SMALL_CORE_COUNT;
        case DEVICE_ZYBER8G:
            return BM1370_SMALL_CORE_COUNT;
        default:
    }
    return 0;
}

// .receive_result_fn = BM1366_process_work,
task_result * ASIC_process_work(GlobalState * GLOBAL_STATE) {
    switch (GLOBAL_STATE->device_model) {
        case DEVICE_ZYBER8S:
            return BM1368_process_work(GLOBAL_STATE);
        case DEVICE_ZYBER8G:
            return BM1370_process_work(GLOBAL_STATE);
        default:
    }
    return NULL;
}

// .set_max_baud_fn = BM1366_set_max_baud,
int ASIC_set_max_baud(GlobalState * GLOBAL_STATE) {
    switch (GLOBAL_STATE->device_model) {
        case DEVICE_ZYBER8S:
            return BM1368_set_max_baud();
        case DEVICE_ZYBER8G:
            return BM1370_set_max_baud();
        default:
    return 0;
    }
}

// .set_difficulty_mask_fn = BM1366_set_job_difficulty_mask,
void ASIC_set_job_difficulty_mask(GlobalState * GLOBAL_STATE, uint8_t mask) {
    switch (GLOBAL_STATE->device_model) {
        case DEVICE_ZYBER8S:
            BM1368_set_job_difficulty_mask(mask);
            break;
        case DEVICE_ZYBER8G:
            BM1370_set_job_difficulty_mask(mask);
            break;
        default:
    }
}

// .send_work_fn = BM1366_send_work,
void ASIC_send_work(GlobalState * GLOBAL_STATE, void * next_job) {
    switch (GLOBAL_STATE->device_model) {
        case DEVICE_ZYBER8S:
            BM1368_send_work(GLOBAL_STATE, next_job);
            break;
        case DEVICE_ZYBER8G:
            BM1370_send_work(GLOBAL_STATE, next_job);
            break;
        default:
    return;
    }
}

// .set_version_mask = BM1366_set_version_mask
void ASIC_set_version_mask(GlobalState * GLOBAL_STATE, uint32_t mask) {
    switch (GLOBAL_STATE->device_model) {
        case DEVICE_ZYBER8S:
            BM1368_set_version_mask(mask);
            break;
        case DEVICE_ZYBER8G:
            BM1370_set_version_mask(mask);
            break;
        default:
    return;
    }
}

bool ASIC_set_frequency(GlobalState * GLOBAL_STATE, float target_frequency) {
    ESP_LOGI(TAG, "Setting ASIC frequency to %.2f MHz", target_frequency);
    bool success = false;
    
    switch (GLOBAL_STATE->asic_model) {
        case ASIC_BM1366:
            success = BM1366_set_frequency(target_frequency);
            break;
        case ASIC_BM1368:
            success = BM1368_set_frequency(target_frequency);
            break;
        case ASIC_BM1370:
            success = BM1370_set_frequency(target_frequency);
            break;
        case ASIC_BM1397:
            // BM1397 doesn't have a set_frequency function yet
            ESP_LOGE(TAG, "Frequency transition not implemented for BM1397");
            success = false;
            break;
        default:
            ESP_LOGE(TAG, "Unknown ASIC model, cannot set frequency");
            success = false;
            break;
    }
    
    if (success) {
        ESP_LOGI(TAG, "Successfully transitioned to new ASIC frequency: %.2f MHz", target_frequency);
    } else {
        ESP_LOGE(TAG, "Failed to transition to new ASIC frequency: %.2f MHz", target_frequency);
    }
    
    return success;
}

esp_err_t ASIC_set_device_model(GlobalState * GLOBAL_STATE) {

    if (GLOBAL_STATE->device_model_str == NULL) {
        ESP_LOGE(TAG, "No device model string found");
        return ESP_FAIL;
    }

    if (strcmp(GLOBAL_STATE->device_model_str, "zyber8s") == 0) {
        GLOBAL_STATE->asic_model = ASIC_BM1368;
        //GLOBAL_STATE.asic_job_frequency_ms = (NONCE_SPACE / (double) (GLOBAL_STATE.POWER_MANAGEMENT_MODULE.frequency_value * BM1368_CORE_COUNT * 1000)) / (double) BITAXE_SUPRA_ASIC_COUNT; // version-rolling so Small Cores have different Nonce Space
        GLOBAL_STATE->asic_job_frequency_ms = 500; //ms
        GLOBAL_STATE->ASIC_difficulty = BM1368_ASIC_DIFFICULTY;
        ESP_LOGI(TAG, "DEVICE: zyber8s");
        ESP_LOGI(TAG, "ASIC: %dx BM1368 (%" PRIu64 " cores)", ZYBER8S_ASIC_COUNT, BM1368_CORE_COUNT);
        GLOBAL_STATE->device_model = DEVICE_ZYBER8S;

    }   else if (strcmp(GLOBAL_STATE->device_model_str, "zyber8g") == 0) {
        GLOBAL_STATE->asic_model = ASIC_BM1370;
        //GLOBAL_STATE.asic_job_frequency_ms = (NONCE_SPACE / (double) (GLOBAL_STATE.POWER_MANAGEMENT_MODULE.frequency_value * BM1370_CORE_COUNT * 1000)) / (double) BITAXE_GAMMA_ASIC_COUNT; // version-rolling so Small Cores have different Nonce Space
        GLOBAL_STATE->asic_job_frequency_ms = 500; //ms
        GLOBAL_STATE->ASIC_difficulty = BM1370_ASIC_DIFFICULTY;
        ESP_LOGI(TAG, "DEVICE: zyber8g");
        ESP_LOGI(TAG, "ASIC: %dx BM1370 (%" PRIu64 " cores)", ZYBER8G_ASIC_COUNT, BM1370_CORE_COUNT);
        GLOBAL_STATE->device_model = DEVICE_ZYBER8G;

    }  else {
        ESP_LOGE(TAG, "Invalid DEVICE model");
        GLOBAL_STATE->device_model = DEVICE_UNKNOWN;
        return ESP_FAIL;
    }
    return ESP_OK;
}
