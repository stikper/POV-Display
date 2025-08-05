#include <esp_log.h>
#include <freertos/FreeRTOS.h>

static auto TAG = "POV_Controller";

extern "C" [[noreturn]] void app_main()
{
    ESP_LOGI(TAG, "Hello, World");
    int i = 0;
    while (true) {
        ESP_LOGI(TAG, "%d", i);
        vTaskDelay(pdMS_TO_TICKS(1000));
        i++;
    }
}
