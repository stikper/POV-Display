//
// Created by kiril on 09.08.2025.
//

#include "Tachometer.h"

Tachometer::Tachometer() : cycle_buffer{} {
    isRunning = false;
    for (auto &i: cycle_buffer) {
        i = 0;
    }
    time_sub = 0;
    k = 0;
    b = 0;

    dataMutex = xSemaphoreCreateMutex();
    hallPosEdgeQueue = xQueueCreate(10, sizeof(int64_t));
    updateTask = nullptr;
}

Tachometer::~Tachometer() {
    vSemaphoreDelete(dataMutex);
    vQueueDelete(hallPosEdgeQueue);
}

void Tachometer::init() {
    xTaskCreate(update, "Tachometer", 4096, this, 10, &updateTask);

    // Configure Hall-sensor GPIO
    gpio_reset_pin(HALL_SENSOR_GPIO);
    gpio_set_direction(HALL_SENSOR_GPIO, GPIO_MODE_INPUT);
    gpio_set_pull_mode(HALL_SENSOR_GPIO, GPIO_PULLDOWN_ONLY);

    // Configure interrupt
    gpio_install_isr_service(0);
    gpio_isr_handler_add(HALL_SENSOR_GPIO, isrHallPosEdge, this);
    gpio_set_intr_type(HALL_SENSOR_GPIO, GPIO_INTR_POSEDGE);
    gpio_intr_enable(HALL_SENSOR_GPIO);

    isRunning = true;
}

void Tachometer::stop() {
    gpio_intr_disable(HALL_SENSOR_GPIO);
    vTaskDelete(updateTask);

    isRunning = false;
}

void Tachometer::isrHallPosEdge(void *arg) {
    BaseType_t xHigherPriorityTaskWoken = pdFALSE;
    auto *tachometer = static_cast<Tachometer *>(arg);

    int64_t timestamp = esp_timer_get_time();

    BaseType_t xResult = xQueueSendFromISR(tachometer->hallPosEdgeQueue, &timestamp, &xHigherPriorityTaskWoken);

    if (xResult == pdPASS) {
        portYIELD_FROM_ISR(xHigherPriorityTaskWoken);
    };
}

void Tachometer::update(void *arg) {
    auto *tachometer = static_cast<Tachometer *>(arg);
    int64_t t = 0;

    while (true) {
        if (xQueueReceive(tachometer->hallPosEdgeQueue, &t, portMAX_DELAY)) {
            tachometer->pushToBuffer(t);
            tachometer->calculateData();
        }
    }
}

// ReSharper disable once CppDFAUnreachableFunctionCall
void Tachometer::pushToBuffer(const int64_t &t) {
    if (xSemaphoreTake(dataMutex, portMAX_DELAY) == pdTRUE) {
        const int64_t t2 = cycle_buffer[1];
        for (int i = 0; i < TACHOMETER_BUFFER_SIZE - 1; i++) {
            cycle_buffer[i] = cycle_buffer[i + 1] - t2;
        }
        time_sub += t2;
        cycle_buffer[TACHOMETER_BUFFER_SIZE - 1] = t - time_sub;
        xSemaphoreGive(dataMutex);
    }
}

// ReSharper disable once CppDFAUnreachableFunctionCall
void Tachometer::calculateData() {
    if (xSemaphoreTake(dataMutex, portMAX_DELAY) == pdTRUE) {
        // y = kx + b (time = k * angle + b)
        // x - angles (1 = 2_PI)
        // y - time
        double xMean = 0, yMean = 0, xxMean = 0, xyMean = 0;
        for (int x = 0; x < TACHOMETER_BUFFER_SIZE; x++) {
            auto y = static_cast<double>(cycle_buffer[x]);
            xMean += x;
            yMean += y;
            xxMean += x * x;
            xyMean += x * y;
        }
        xMean /= TACHOMETER_BUFFER_SIZE;
        yMean /= TACHOMETER_BUFFER_SIZE;
        xxMean /= TACHOMETER_BUFFER_SIZE;
        xyMean /= TACHOMETER_BUFFER_SIZE;

        k = (xyMean - xMean * yMean) / (xxMean - xMean * xMean);
        b = yMean - k * xMean;

        xSemaphoreGive(dataMutex);
    }
}

std::pair<int64_t, int64_t> Tachometer::getData() const {
    std::pair<int64_t, int64_t> result = {};
    if (xSemaphoreTake(dataMutex, portMAX_DELAY) == pdTRUE) {
        auto w = static_cast<int64_t>(k);
        auto t0 = static_cast<int64_t>(k * (TACHOMETER_BUFFER_SIZE - 1) + b) + time_sub;
        result.first = w;
        result.second = t0;
        xSemaphoreGive(dataMutex);
    }
    return result;
}

