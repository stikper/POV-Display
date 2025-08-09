//
// Created by kiril on 09.08.2025.
//

#include "Tachometer.h"

Tachometer::Tachometer() : buffer{} {
    isRunning = false;
    for (auto & i : buffer) {
        i = 0;
    }
    time_sub = 0;
    k = 0;
    b = 0;

    dataMutex = xSemaphoreCreateMutex();
    hallPosEdgeQueue = xQueueCreate(10, sizeof(int64_t));
    updateBufferTask = nullptr;
    updateDataTask = nullptr;
}

Tachometer::~Tachometer() {
    vSemaphoreDelete(dataMutex);
    vQueueDelete(hallPosEdgeQueue);
}

void Tachometer::init() {
    xTaskCreate(updateBuffer, "Tacho buf", 2048, this, 10, &updateBufferTask);
    xTaskCreate(updateData, "Tacho data", 4096, this, 7, &updateDataTask);

    // Configure Hall-sensor GPIO
    gpio_reset_pin(HALL_SENSOR_GPIO);
    gpio_set_direction(HALL_SENSOR_GPIO, GPIO_MODE_INPUT);
    gpio_set_pull_mode(HALL_SENSOR_GPIO, GPIO_FLOATING);

    // Configure interrupt
    gpio_install_isr_service(0);
    gpio_isr_handler_add(HALL_SENSOR_GPIO, isrHallPosEdge, this);
    gpio_set_intr_type(HALL_SENSOR_GPIO, GPIO_INTR_POSEDGE);
    gpio_intr_enable(HALL_SENSOR_GPIO);

    isRunning = true;
}

void Tachometer::stop() {
    gpio_intr_disable(HALL_SENSOR_GPIO);
    vTaskDelete(updateDataTask);
    vTaskDelete(updateBufferTask);

    isRunning = false;
}


