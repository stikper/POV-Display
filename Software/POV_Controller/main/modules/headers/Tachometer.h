//
// Created by kiril on 09.08.2025.
//

#pragma once
#ifndef TACHOMETER_H
#define TACHOMETER_H

#include <esp_attr.h>
#include <bits/stl_pair.h>
#include <driver/gpio.h>
#include <freertos/FreeRTOS.h>
#include <freertos/queue.h>
#include <esp_timer.h>
#include <esp_log.h>

#define HALL_SENSOR_GPIO GPIO_NUM_4
#define TACHOMETER_BUFFER_SIZE 3

class Tachometer {
    bool isRunning;

    int64_t cycle_buffer[TACHOMETER_BUFFER_SIZE];
    int64_t time_sub; // To make first event time=0
    double k, b; // time = k * angle + b; [time] = us, [angle] = rad/2_PI

    SemaphoreHandle_t dataMutex;
    QueueHandle_t hallPosEdgeQueue;
    TaskHandle_t updateTask;

    static void IRAM_ATTR isrHallPosEdge(void* arg);

    _Noreturn static void update(void *arg);

    void pushToBuffer(const int64_t &t);
    void calculateData();
public:
    Tachometer();
    ~Tachometer();

    void init();
    void stop();

    [[nodiscard]] std::pair<int64_t, int64_t> getData() const;
};



#endif //TACHOMETER_H
