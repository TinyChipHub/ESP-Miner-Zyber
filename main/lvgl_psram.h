#ifndef LVGL_PSRAM_H_
#define LVGL_PSRAM_H_

#include "esp_heap_caps.h"
#include "esp_log.h"

// Custom malloc function using PSRAM
static void * lv_psram_alloc(size_t size)
{
    ESP_LOGI("LVGL_PSRAM", "Allocating %d bytes from PSRAM", size);
    return heap_caps_malloc(size, MALLOC_CAP_SPIRAM);
}

// Custom free function
static void lv_psram_free(void * ptr)
{
    heap_caps_free(ptr);
}

static void * lv_psram_realloc(void * ptr, size_t new_size)
{
    if (new_size == 0) {
        heap_caps_free(ptr);
        return NULL;
    }

    if (!ptr) {
        return heap_caps_malloc(new_size, MALLOC_CAP_SPIRAM);
    }

    // Try to expand in place (not possible with ESP-IDF heap_caps)
    void * new_ptr = heap_caps_malloc(new_size, MALLOC_CAP_SPIRAM);
    if (new_ptr) {
        // Copy old data to new location
        memcpy(new_ptr, ptr, new_size); // Note: Might copy too much if shrinking
        heap_caps_free(ptr);
    }

    return new_ptr;
}

#endif /* LVGL_PSRAM_H_ */