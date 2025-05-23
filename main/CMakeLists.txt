idf_component_register(
SRCS
    "adc.c"
    "i2c_bitaxe.c"
    "main.c"
    "nvs_config.c"
    "display.c"
    "screen.c"
    "input.c"
    "system.c"
    "work_queue.c"
    "nvs_device.c"
    "./http_server/http_server.c"
    "./http_server/theme_api.c"
    "./tasks/stratum_task.c"
    "./tasks/create_jobs_task.c"
    "./tasks/asic_task.c"
    "./tasks/asic_result_task.c"
    "./tasks/power_management_task.c"
    "./tasks/chips_monitor_task.c"
    "./thermal/EMC2302.c"
    "./thermal/TMP1075.c"
    "./thermal/thermal.c"
    "./power/TPS546.c"
    "./power/power.c"
    "./power/vcore.c"

    #Fonts
    "./fonts/font_XinYin_reg10.c"
    "./fonts/font_XinYin_reg13.c"
    "./fonts/font_XinYin_reg14.c"
    "./fonts/font_XinYin_reg12.c"
    "./fonts/font_XinYin_reg18.c"
    "./fonts/font_XinYin_reg22.c"

    #Backgroud Images
    "./themes/logo.c"
    "./themes/device_info.c"
    "./themes/mining_stat.c"
    "./themes/network.c"
    "./themes/setup.c"
    "./themes/overheat.c"

INCLUDE_DIRS
    "."
    "tasks"
    "http_server"
    "../components/asic/include"
    "../components/connect/include"
    "../components/dns_server/include"
    "../components/stratum/include"
    "thermal"
    "power"

PRIV_REQUIRES
    "app_update"
    "driver"
    "esp_adc"
    "esp_app_format"
    "esp_event"
    "esp_http_server"
    "esp_netif"
    "esp_psram"
    "esp_timer"
    "esp_wifi"
    "json"
    "nvs_flash"
    "spiffs"
    "vfs"
    "esp_driver_i2c"

EMBED_FILES "http_server/recovery_page.html"
)

idf_build_set_property(COMPILE_OPTIONS "-DLV_CONF_INCLUDE_SIMPLE=1" APPEND)
idf_build_set_property(COMPILE_OPTIONS "-DLV_CONF_PATH= ${CMAKE_SOURCE_DIR}/main/lv_conf.h" APPEND)

set(WEB_SRC_DIR "${CMAKE_CURRENT_SOURCE_DIR}/http_server/axe-os")

if("$ENV{GITHUB_ACTIONS}" STREQUAL "true")
    message(STATUS "Running on GitHub Actions. Web ui will be prebuilt.")

    spiffs_create_partition_image(www ${WEB_SRC_DIR}/dist/axe-os FLASH_IN_PROJECT)
else()
    find_program(NPM_EXECUTABLE npm)
    if(NOT NPM_EXECUTABLE AND NOT EXISTS ${WEB_SRC_DIR}/dist)
        message(FATAL_ERROR "npm is not found! Please install it to proceed.")
    endif()

    ExternalProject_Add(
        web_ui_dist
        PREFIX ${CMAKE_BINARY_DIR}/web_ui_dist
        SOURCE_DIR ${WEB_SRC_DIR}
        DOWNLOAD_COMMAND ""
        CONFIGURE_COMMAND
            ${NPM_EXECUTABLE}  i
        USES_TERMINAL_BUILD true
        BUILD_COMMAND
            ${NPM_EXECUTABLE} run build
        INSTALL_COMMAND ""
        BUILD_ALWAYS OFF
        BUILD_IN_SOURCE TRUE
        BUILD_BYPRODUCTS
            "${WEB_SRC_DIR}/dist/axe-os/index.html"
    )

    add_dependencies(${COMPONENT_LIB} web_ui_dist)

    spiffs_create_partition_image(www ${WEB_SRC_DIR}/dist/axe-os FLASH_IN_PROJECT DEPENDS web_ui_dist)
endif()
