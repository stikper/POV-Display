#include <esp_log.h>
#include <freertos/FreeRTOS.h>

// #include "LED_Strip.h"
// #include "Tachometer.h"
#include "POV_Display.h"

static auto TAG = "POV_Controller";


extern "C" [[noreturn]] void app_main()
{
    POV_Display display;
    ESP_LOGI(TAG, "Initializing...");
    display.init();
    ESP_LOGI(TAG, "Initialized!");
    ESP_LOGI(TAG, "Starting POV display controller");
    display.start();

    while (true) {
        vTaskDelay(pdMS_TO_TICKS(500)); // вывод каждые 2 секунды
    }
}
