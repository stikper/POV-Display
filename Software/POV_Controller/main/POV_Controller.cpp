#include <esp_log.h>
#include <freertos/FreeRTOS.h>

#include "LED_Strip.h"
#include "Tachometer.h"

static auto TAG = "POV_Controller";

// Animation, 144 steps
void animateRainbow(LED_Strip::HSV_Color *buffer, int step = 0) {
    for (int i = 0; i < 144; i++) {
        buffer[(i + step) % 144].hue = (int)((float) i / 144.0f * 255) % 255;
        buffer[(i + step) % 144].saturation = 255;
        buffer[(i + step) % 144].value = 100;
    }
}

void animateRainbowVibe(LED_Strip::RGB_Color *buffer, int step = 0) {
    const int num_leds = 144;
    const int sector_size = num_leds / 6; // 24 пикселя

    LED_Strip::RGB_Color colors[6] = {
        LED_Strip::RGB_Color(128, 0, 255),    // Фиолетовый
        LED_Strip::RGB_Color(0, 0, 255),     // Синий
        LED_Strip::RGB_Color(0, 255, 0),     // Зелёный
        LED_Strip::RGB_Color(255, 120, 0),   // Жёлтый
        LED_Strip::RGB_Color(255, 20, 0),   // Оранжевый
        LED_Strip::RGB_Color(255, 0, 0),     // Красный
    };

    for (int i = 0; i < num_leds; i++) {
        int pos = (i + step) % num_leds;
        int color_index = i / sector_size;
        buffer[pos] = colors[color_index];
    }
}

void solidColor(LED_Strip::RGB_Color *buffer, LED_Strip::RGB_Color color) {
    for (int i = 0; i < 144; i++) {
        buffer[i].red = color.red;
        buffer[i].green = color.green;
        buffer[i].blue = color.blue;
    }
}

extern "C" [[noreturn]] void app_main()
{
    ESP_LOGI(TAG, "Starting POV_Controller...");
    LED_Strip strip(144);
    strip.init();
    strip.set_default_global(1); // n/31

    /*
    LED_Strip::RGB_Color buffer[144] = {};
    LED_Strip::RGB_Color red = LED_Strip::RGB_Color(255, 0, 0);
    LED_Strip::RGB_Color blue = LED_Strip::RGB_Color(0, 0, 255);
    while (true) {
        solidColor(buffer, red);
        strip.set_multi_pixel(buffer, 145);
        strip.update();
        solidColor(buffer, blue);
        strip.set_multi_pixel(buffer, 145);
        strip.update();
    }
    */
    LED_Strip::RGB_Color buffer[144];
    int step = 0;

    while (true) {
        animateRainbowVibe(buffer, step);
        step += 1;
        step %= 144;
        strip.set_multi_pixel(buffer, 144);
        strip.update();
        vTaskDelay(50 / portTICK_PERIOD_MS);
    }
}
