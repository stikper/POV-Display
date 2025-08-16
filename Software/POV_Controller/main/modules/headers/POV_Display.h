//
// Created by kiril on 15.08.2025.
//

#ifndef POV_CONTROLLER_POV_DISPLAY_H
#define POV_CONTROLLER_POV_DISPLAY_H

#include "LED_Strip.h"
#include "Tachometer.h"

#include "driver/gptimer.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/queue.h"


class POV_Display {
    static constexpr int num_leds = 144;
    static constexpr int num_sectors = 60;

    bool isRunning;

    static const uint8_t pov_data[num_sectors][num_leds][3];

    Tachometer tachometer;
    LED_Strip ledStrip;

    SemaphoreHandle_t dataMutex;
    gptimer_handle_t updateTimer;
    QueueHandle_t updateQueue;
    TaskHandle_t updateTask;

    static bool IRAM_ATTR gptimer_alarm_callback(gptimer_handle_t timer, const gptimer_alarm_event_data_t *edata, void *user_data);

    _Noreturn static void update(void *arg);

public:
    POV_Display();
    ~POV_Display();

    void init();

    void start();
    void stop();
};


#endif //POV_CONTROLLER_POV_DISPLAY_H