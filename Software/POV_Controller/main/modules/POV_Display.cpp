//
// Created by kiril on 15.08.2025.
//

#include "headers/POV_Display.h"

#include "esp_log.h"


POV_Display::POV_Display() : tachometer(), ledStrip(num_leds) {
    isRunning = false;

    dataMutex = nullptr;
    updateTimer = nullptr;
    updateQueue = nullptr;
    updateTask = nullptr;
}

POV_Display::~POV_Display() {
    vSemaphoreDelete(dataMutex);
    vQueueDelete(updateQueue);
    gptimer_del_timer(updateTimer);
}


void POV_Display::init() {
    tachometer.init();
    ledStrip.init();

    dataMutex = xSemaphoreCreateMutex();

    gptimer_config_t timer_config = {};
    timer_config.clk_src = GPTIMER_CLK_SRC_DEFAULT;
    timer_config.direction = GPTIMER_COUNT_UP;
    timer_config.resolution_hz = 1000000; // 1MHz, 1 tick=1us
    gptimer_new_timer(&timer_config, &updateTimer);
    gptimer_event_callbacks_t cb_config;
    cb_config.on_alarm = gptimer_alarm_callback;
    gptimer_register_event_callbacks(updateTimer, &cb_config, this);
    gptimer_set_raw_count(updateTimer, 0);

    updateQueue = xQueueCreate(10, sizeof(int64_t));
}

void POV_Display::start() {
    isRunning = true;
    xTaskCreate(update, "Display", 4096, this, 5, &updateTask);
}

void POV_Display::stop() {
    isRunning = false;
    vTaskDelete(updateTask);
}

LED_Strip & POV_Display::getLEDStrip() { return ledStrip; }

Tachometer & POV_Display::getTachometer() { return tachometer; }

void POV_Display::update(void *arg) {
    auto *display = static_cast<POV_Display *>(arg);
    int64_t t_irpt = 0;
    bool timer_enabled = false;

    while (true) {
        auto t = esp_timer_get_time();
        auto [w, t0] = display->tachometer.getData();
        auto w_one = w / num_sectors;
        if (w_one == 0) {
            vTaskDelay(1000 / portTICK_PERIOD_MS);
            continue;
        }
        auto sector_next = static_cast<int>((t - t0) / w_one + 1);
        int64_t time_until_sector_next = t0 + w_one * sector_next - t;
        if (time_until_sector_next < 0) {
            vTaskDelay(1);
            continue;
        }
        if (time_until_sector_next > 1000000) {
            vTaskDelay(1000 / portTICK_PERIOD_MS);
            continue;
        }
        sector_next %= num_sectors;
        if (sector_next < 0) sector_next += num_sectors;

        gptimer_alarm_config_t alarm_config = {};
        alarm_config.alarm_count = time_until_sector_next;
        alarm_config.reload_count = 0;
        alarm_config.flags.auto_reload_on_alarm = true;
        gptimer_set_alarm_action(display->updateTimer, &alarm_config);
        if (!timer_enabled) {
            gptimer_enable(display->updateTimer);
            timer_enabled = true;
        }
        gptimer_start(display->updateTimer);


        if (xQueueReceive(display->updateQueue, &t_irpt, portMAX_DELAY)) {

            LED_Strip::RGB_Color buffer[num_leds];
            for (int i = 0; i < num_leds; i++) {
                buffer[i].red = pov_data[sector_next][i][0];
                buffer[i].green = pov_data[sector_next][i][1];
                buffer[i].blue = pov_data[sector_next][i][2];
            }

            display->ledStrip.set_multi_pixel(buffer, num_leds);
            display->ledStrip.update();
        }
    }

}

bool POV_Display::gptimer_alarm_callback(gptimer_handle_t timer, const gptimer_alarm_event_data_t *edata, void *user_data) {
    gptimer_stop(timer);
    gptimer_set_raw_count(timer, 0);
    auto *display = static_cast<POV_Display *>(user_data);
    auto t_irpt = esp_timer_get_time();
    BaseType_t xHigherPriorityTaskWoken = pdFALSE;
    BaseType_t xResult = xQueueSendFromISR(display->updateQueue, &t_irpt, &xHigherPriorityTaskWoken);
    if (xResult == pdTRUE) {
        if (xHigherPriorityTaskWoken == pdTRUE) return true;
    }
    return false;
}


#include "pov_data.txt"
