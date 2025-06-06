#include "esp_log.h"
#include "esp_err.h"
#include "esp_check.h"
#include "lvgl.h"
#include "esp_lvgl_port.h"
#include "driver/gpio.h"

#define GPIO_BUTTON_BOOT CONFIG_GPIO_BUTTON_BOOT
#define GPIO_BUTTON_DISPLAY (gpio_num_t) 39

#define ESP_INTR_FLAG_DEFAULT  0
#define LONG_PRESS_DURATION_MS 2000

static const char * TAG = "input";

static lv_indev_state_t button_state = LV_INDEV_STATE_RELEASED;
static lv_indev_state_t display_button_state = LV_INDEV_STATE_RELEASED;

static void (*button_short_clicked_fn)(void) = NULL;
static void (*button_long_pressed_fn)(void) = NULL;

static void (*display_button_short_clicked_fn)(void) = NULL;
static void (*display_button_long_pressed_fn)(void) = NULL;

static void button_read(lv_indev_t *indev, lv_indev_data_t *data) 
{
    data->key = LV_KEY_ENTER;
    data->state = button_state;
}

static void IRAM_ATTR button_isr_handler(void *arg) 
{
    bool pressed = gpio_get_level(GPIO_BUTTON_BOOT) == 0; // LOW when pressed
    button_state = pressed ? LV_INDEV_STATE_PRESSED : LV_INDEV_STATE_RELEASED;
}

static void display_button_read(lv_indev_t *indev, lv_indev_data_t *data) 
{
    data->key = LV_KEY_ENTER;
    data->state = display_button_state;
}

static void IRAM_ATTR display_button_isr_handler(void *arg) 
{
    bool pressed = gpio_get_level(GPIO_BUTTON_DISPLAY) == 0; // LOW when pressed
    //ESP_LOGW(TAG,"DIS PRESS = %s",pressed?"TRUE":"FALSE");
    display_button_state = pressed ? LV_INDEV_STATE_PRESSED : LV_INDEV_STATE_RELEASED;
}

static void button_short_clicked_event_cb(lv_event_t *e)
{
    ESP_LOGI(TAG, "Short button click detected");
    button_short_clicked_fn();
}

static void button_long_pressed_event_cb(lv_event_t *e)
{
    ESP_LOGI(TAG, "Long button press detected");
    button_long_pressed_fn();
}

static void display_button_short_clicked_event_cb(lv_event_t *e)
{
    ESP_LOGI(TAG, "Short display button click detected");
    display_button_short_clicked_fn();
}

static void display_button_long_pressed_event_cb(lv_event_t *e)
{
    ESP_LOGI(TAG, "Long display button press detected");
    display_button_long_pressed_fn();
}

esp_err_t input_init(void (*button_short_clicked_cb)(void), void (*button_long_pressed_cb)(void))
{
    ESP_LOGI(TAG, "Install button driver");

    // Button handling
    gpio_config_t io_conf = {
        .pin_bit_mask = (1ULL << GPIO_BUTTON_BOOT),
        .mode = GPIO_MODE_INPUT,
        .pull_up_en = GPIO_PULLUP_ENABLE,
        .intr_type = GPIO_INTR_ANYEDGE
    };
    gpio_config(&io_conf);

    // Install ISR service and hook the interrupt handler
    //ESP_RETURN_ON_ERROR(gpio_install_isr_service(ESP_INTR_FLAG_DEFAULT), TAG, "Error installing ISR service");
    ESP_RETURN_ON_ERROR(gpio_isr_handler_add(GPIO_BUTTON_BOOT, button_isr_handler, NULL), TAG, "Error adding ISR handler");

    lv_group_t * group = lv_group_create();
    lv_group_set_default(group);
    lv_group_add_obj(group, lv_obj_create(NULL)); // dummy screen for event handling, in case no display is attached

    // Create input device
    lv_indev_t * indev = lv_indev_create();
    lv_indev_set_type(indev, LV_INDEV_TYPE_KEYPAD);
    lv_indev_set_long_press_time(indev, LONG_PRESS_DURATION_MS);
    lv_indev_set_read_cb(indev, button_read);
    lv_indev_set_group(indev, group);
    if (button_short_clicked_cb != NULL) {
        button_short_clicked_fn = button_short_clicked_cb;
        lv_indev_add_event_cb(indev, button_short_clicked_event_cb, LV_EVENT_SHORT_CLICKED, NULL);
    }
    if (button_long_pressed_cb != NULL) {
        button_long_pressed_fn = button_long_pressed_cb;
        lv_indev_add_event_cb(indev, button_long_pressed_event_cb, LV_EVENT_LONG_PRESSED, NULL);
    }

    return ESP_OK;
}

esp_err_t display_input_init(void (*display_button_short_clicked_cb)(void), void (*display_button_long_pressed_cb)(void)){
    ESP_LOGI(TAG, "Install display button driver");

    // Button handling
    gpio_config_t io_conf = {
        .pin_bit_mask = (1ULL << GPIO_BUTTON_DISPLAY),
        .mode = GPIO_MODE_INPUT,
        .pull_up_en = GPIO_PULLUP_ENABLE,
        .intr_type = GPIO_INTR_ANYEDGE
    };
    gpio_config(&io_conf);

    // Install ISR service and hook the interrupt handler
    //ESP_RETURN_ON_ERROR(gpio_install_isr_service(ESP_INTR_FLAG_DEFAULT), TAG, "Error installing ISR service");
    ESP_RETURN_ON_ERROR(gpio_isr_handler_add(GPIO_BUTTON_DISPLAY, display_button_isr_handler, NULL), TAG, "Error adding ISR handler");

    lv_group_t * group = lv_group_create();
    lv_group_set_default(group);
    lv_group_add_obj(group, lv_obj_create(NULL)); // dummy screen for event handling, in case no display is attached

    // Create input device
    lv_indev_t * indev = lv_indev_create();
    lv_indev_set_type(indev, LV_INDEV_TYPE_KEYPAD);
    lv_indev_set_long_press_time(indev, LONG_PRESS_DURATION_MS);
    lv_indev_set_read_cb(indev, display_button_read);
    lv_indev_set_group(indev, group);
    if (display_button_short_clicked_cb != NULL) {
        display_button_short_clicked_fn = display_button_short_clicked_cb;
        lv_indev_add_event_cb(indev, display_button_short_clicked_event_cb, LV_EVENT_SHORT_CLICKED, NULL);
    }
    if (display_button_long_pressed_cb != NULL) {
        display_button_long_pressed_fn = display_button_long_pressed_cb;
        lv_indev_add_event_cb(indev, display_button_long_pressed_event_cb, LV_EVENT_LONG_PRESSED, NULL);
    }

    return ESP_OK;
}
