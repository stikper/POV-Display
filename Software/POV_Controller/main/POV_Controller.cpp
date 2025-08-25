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

    int64_t w_min = INT64_MAX;
    while (true) {
        auto [w, t0] = display.getTachometer().getData();
        if (w != 0 && w < w_min) w_min = w;
        ESP_LOGI(TAG, "Max RPM is: %.3f", 60. / (static_cast<double>(w_min) / 1000000));
        ESP_LOGI(TAG, "Current RPM is: %.3f", 60. / (static_cast<double>(w) / 1000000));
        vTaskDelay(pdMS_TO_TICKS(1000)); // вывод каждую секунду
    }
}
