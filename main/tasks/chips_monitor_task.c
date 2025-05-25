#include <sys/time.h>
#include <limits.h>
#include "driver/uart.h"

#include "work_queue.h"
#include "global_state.h"
#include "esp_log.h"
#include "esp_system.h"
#include "frequency_transition_bmXX.h"
#include "asic.h"
#include "serial.h"
#include "chips_monitor_task.h"
#include "nvs_config.h"

static const char *TAG = "chip_monitor_task";
static uint16_t reset_count = 0;
static uint32_t temp_chips_count[8];
static uint32_t temp_chips_fail_count[8];

void reset_all_chip(void * pvParameters){
    GlobalState *GLOBAL_STATE = (GlobalState *)pvParameters;
    reset_count++;

    nvs_config_set_u16(NVS_CONFIG_CHIPS_CAUSE_RESTART,nvs_config_get_u16(NVS_CONFIG_CHIPS_CAUSE_RESTART,0)+1);

    vTaskSuspend(GLOBAL_STATE->asic_result_task_h);
    vTaskSuspend(GLOBAL_STATE->asic_task_h);
    vTaskSuspend(GLOBAL_STATE->job_task_h);

    reset_current_frequency(2.2f);

    // Disable UART
    uart_disable_rx_intr(UART_NUM_1);
    uart_disable_tx_intr(UART_NUM_1);
    
    // Stop and uninstall UART driver
    uart_driver_delete(UART_NUM_1);
    
    SERIAL_init();
    ASIC_init(GLOBAL_STATE);

    SERIAL_set_baud(ASIC_set_max_baud(GLOBAL_STATE));
    SERIAL_clear_buffer();

    GLOBAL_STATE->ASIC_initalized=true;

    vTaskResume(GLOBAL_STATE->job_task_h);
    vTaskResume(GLOBAL_STATE->asic_task_h);
    vTaskResume(GLOBAL_STATE->asic_result_task_h);
}

void chip_monitor_task(void *pvParameters){
    GlobalState *GLOBAL_STATE = (GlobalState *)pvParameters;
    ESP_LOGI(TAG, "Starting chips fail detection task");
    while(1){
        if(reset_count>0){
            ESP_LOGI(TAG, "Chips reset attemp = %u", reset_count);
            if(reset_count>50){
                ESP_LOGW(TAG, "Chips reset attemp > 50, It is better to restart the miner again");
                exit(EXIT_FAILURE);
            }
        }

        if(!GLOBAL_STATE->is_chips_fail_detected&&GLOBAL_STATE->ASIC_initalized){
            for(int a=0;a<8;a++){
                if(temp_chips_count[a]<GLOBAL_STATE->chip_submit[a]){
                    temp_chips_fail_count[a]=0;
                    temp_chips_count[a]=GLOBAL_STATE->chip_submit[a];
                }else{
                    if(temp_chips_count[a]>0){
                        temp_chips_fail_count[a]++;
                        if(temp_chips_fail_count[a]>=5){
                            GLOBAL_STATE->is_chips_fail_detected=true;
                            for(int a=0;a<8;a++){
                                temp_chips_fail_count[a]=0;
                            }
                        }
                    }
                }
            }
        }

        if(GLOBAL_STATE->is_chips_fail_detected){
            GLOBAL_STATE->ASIC_initalized=false;
            reset_all_chip(GLOBAL_STATE);
            GLOBAL_STATE->is_chips_fail_detected=false;
        }

        if(GLOBAL_STATE->chip_submit[0]>=1000000){
            ESP_LOGI(TAG,"Asic Chip Submit Counts Reset!");
            for(int a=0;a<8;a++){
                temp_chips_fail_count[a]=0;
                temp_chips_count[a]=0;
                GLOBAL_STATE->chip_submit[a]=0;
            }
        }
        vTaskDelay(5000/ portTICK_PERIOD_MS);
    }
}