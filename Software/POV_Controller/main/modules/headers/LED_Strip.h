//
// Created by kiril on 07.08.2025.
//

#pragma once
#ifndef LED_STRIP_H
#define LED_STRIP_H

#include <cstdint>
#include "driver/spi_master.h"
#include "driver/gpio.h"
#include "esp_err.h"

#define LED_STRIP_BUFFER_SIZE 144

#define LED_STRIP_SPI_BUS SPI3_HOST
#define LED_STRIP_SDI GPIO_NUM_23
#define LED_STRIP_CKI GPIO_NUM_18
#define LED_STRIP_CLOCK_SPEED (10 * 1000 * 1000)


class LED_Strip {
public:
	struct RGB_Color {
		uint8_t red = 0;
		uint8_t green = 0;
		uint8_t blue = 0;
	};
	struct HSV_Color {
		uint8_t hue = 0;
		uint8_t saturation = 0;
		uint8_t value = 0;
	};
	static RGB_Color hsv2rgb(HSV_Color hsv);
	static HSV_Color rgb2hsv(RGB_Color rgb);

	struct HD107s_Color {
		uint8_t global = 0;
		uint8_t blue = 0;
		uint8_t green = 0;
		uint8_t red = 0;
	};

private:
	static const char* TAG;
	int num_leds;
	uint8_t default_global;
	HD107s_Color buffer[LED_STRIP_BUFFER_SIZE];
	uint8_t tx_buffer[LED_STRIP_BUFFER_SIZE * 4 + 4 * 2];
	spi_device_handle_t device_handle;

public:
	explicit LED_Strip(int num_leds);
	~LED_Strip();

	esp_err_t init();
	void stop() const;

	void set_pixel(int num, HD107s_Color color);
	void set_pixel(int num, RGB_Color rgb);
	void set_pixel(int num, HSV_Color hsv);

	void set_default_global(uint8_t new_default_global);

	void set_multi_pixel(const HD107s_Color *new_buffer, size_t size = LED_STRIP_BUFFER_SIZE);
	void set_multi_pixel(const RGB_Color *new_buffer, size_t size = LED_STRIP_BUFFER_SIZE);
	void set_multi_pixel(const HSV_Color *new_buffer, size_t size = LED_STRIP_BUFFER_SIZE);

	void update();
};



#endif //LED_STRIP_H
