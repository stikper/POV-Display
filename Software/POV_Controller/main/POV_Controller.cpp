#include <esp_log.h>
#include <freertos/FreeRTOS.h>

#include "LED_Strip.h"
#include "Tachometer.h"

static auto TAG = "POV_Controller";


extern "C" [[noreturn]] void app_main()
{
    Tachometer tach;
    tach.init();

    while (true) {
        auto [w, t0] = tach.getData();

        double period_ms = w / 1000.0;    // полный оборот в мс
        double last_avg_ms = t0 / 1000.0; // усреднённое время последнего пролёта

        ESP_LOGI(TAG,"Период оборота: %.3f мс, Усреднённое время пролёта: %.3f мс\n",
               period_ms, last_avg_ms);

        vTaskDelay(pdMS_TO_TICKS(500)); // вывод каждые 2 секунды
    }
}
