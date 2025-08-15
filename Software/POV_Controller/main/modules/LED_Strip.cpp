//
// Created by kiril on 07.08.2025.
//

#include "LED_Strip.h"

#include "driver/gpio.h"


const char* LED_Strip::TAG = "LED_Strip";

LED_Strip::LED_Strip(int num_leds) : buffer{}, tx_buffer{} {
    if (num_leds > LED_STRIP_BUFFER_SIZE) num_leds = LED_STRIP_BUFFER_SIZE;
    this->num_leds = num_leds;
    default_global = 0b00011110;
    for (auto & i : buffer) {
        i.global = default_global;
        i.red = 0;
        i.green = 0;
        i.blue = 0;
    }
    device_handle = nullptr;
}

LED_Strip::~LED_Strip() = default;


esp_err_t LED_Strip::init() {
    spi_bus_config_t bus_cfg = {};
    bus_cfg.miso_io_num = -1;
    bus_cfg.mosi_io_num = LED_STRIP_SDI;
    bus_cfg.sclk_io_num = LED_STRIP_CKI;
    bus_cfg.quadwp_io_num = -1,
    bus_cfg.quadhd_io_num = -1,
    bus_cfg.max_transfer_sz = SOC_SPI_MAXIMUM_BUFFER_SIZE;
    esp_err_t ret = spi_bus_initialize(LED_STRIP_SPI_BUS, &bus_cfg, SPI_DMA_CH_AUTO);
    if (ret != ESP_OK) return ret;

    spi_device_interface_config_t dev_cfg = {};
    dev_cfg.clock_speed_hz = LED_STRIP_CLOCK_SPEED;
    dev_cfg.mode = 0;
    dev_cfg.queue_size = 7;
    ret = spi_bus_add_device(LED_STRIP_SPI_BUS, &dev_cfg, &device_handle);
    if (ret != ESP_OK) return ret;

    return ret;
}

void LED_Strip::stop() const {
    spi_bus_remove_device(device_handle);
    spi_bus_free(LED_STRIP_SPI_BUS);
}

void LED_Strip::update() {
    // Prepare tx_buffer
    for (int i = 0; i < 4; ++i) {
        tx_buffer[i] = 0;
    }
    for (int i = 0; i < num_leds; ++i) {
        tx_buffer[4 + i * 4] = 0b11100000;
        tx_buffer[4 + i * 4] |= default_global;
        tx_buffer[4 + i * 4 + 1] = buffer[i].blue;
        tx_buffer[4 + i * 4 + 2] = buffer[i].green;
        tx_buffer[4 + i * 4 + 3] = buffer[i].red;
    }
    for (int i = (4 + num_leds * 4); i < (4 + num_leds * 4) + 4; ++i) {
        tx_buffer[i] = 255;
    }

    // Создаём транзакцию
    spi_transaction_t t = {};
    t.length = (4 * 2 + num_leds * 4) * 8;  // в битах
    t.tx_buffer = tx_buffer;
    t.rx_buffer = nullptr;
    t.rxlength = 0;

    spi_device_transmit(device_handle, &t);
}

void LED_Strip::set_pixel(int num, HD107s_Color color) {
    if (num >= num_leds) return;
    buffer[num] = color;
}

void LED_Strip::set_pixel(int num, RGB_Color rgb) {
    if (num >= num_leds) return;
    buffer[num].global = default_global;
    buffer[num].red = rgb.red;
    buffer[num].green = rgb.green;
    buffer[num].blue = rgb.blue;
}

void LED_Strip::set_pixel(int num, HSV_Color hsv) {
    if (num >= num_leds) return;
    const RGB_Color rgb = hsv2rgb(hsv);
    set_pixel(num, rgb);
}

void LED_Strip::set_default_global(uint8_t new_default_global) {
    if (new_default_global > 31) new_default_global = 31;
    default_global = new_default_global;
}

void LED_Strip::set_multi_pixel(const HD107s_Color *new_buffer, size_t size) {
    if (size > num_leds) size = num_leds;
    for (int i = 0; i < size; ++i) {
        buffer[i] = new_buffer[i];
    }
}

void LED_Strip::set_multi_pixel(const RGB_Color *new_buffer, size_t size) {
    if (size > num_leds) size = num_leds;
    for (int i = 0; i < size; ++i) {
        buffer[i].global = default_global;
        buffer[i].red = new_buffer[i].red;
        buffer[i].green = new_buffer[i].green;
        buffer[i].blue = new_buffer[i].blue;
    }
}

void LED_Strip::set_multi_pixel(const HSV_Color *new_buffer, size_t size) {
    if (size > num_leds) size = num_leds;
    for (int i = 0; i < size; ++i) {
        RGB_Color rgb = hsv2rgb(new_buffer[i]);
        buffer[i].global = default_global;
        buffer[i].red = rgb.red;
        buffer[i].green = rgb.green;
        buffer[i].blue = rgb.blue;
    }
}


LED_Strip::RGB_Color LED_Strip::hsv2rgb(HSV_Color hsv) {
    RGB_Color rgb;

    uint8_t h = hsv.hue;        // 0-255 (представляет 0-360°)
    uint8_t s = hsv.saturation; // 0-255 (представляет 0-100%)
    uint8_t v = hsv.value;      // 0-255 (представляет 0-100%)

    // Если saturation = 0, то это оттенки серого
    if (s == 0) {
        rgb.red = rgb.green = rgb.blue = v;
        return rgb;
    }

    // Преобразуем hue в сектор 0-5 (каждый сектор = 42.5 единицы в uint8_t)
    uint8_t sector = h / 43;  // 255/6 ≈ 42.5, округляем до 43
    uint8_t remainder = (h - (sector * 43)) * 6; // Остаток в секторе

    // Вычисляем промежуточные значения
    uint8_t p = (v * (255 - s)) >> 8;
    uint8_t q = (v * (255 - ((s * remainder) >> 8))) >> 8;
    uint8_t t = (v * (255 - ((s * (255 - remainder)) >> 8))) >> 8;

    // Выбираем значения RGB в зависимости от сектора
    switch (sector) {
        case 0:
            rgb.red = v;
            rgb.green = t;
            rgb.blue = p;
            break;
        case 1:
            rgb.red = q;
            rgb.green = v;
            rgb.blue = p;
            break;
        case 2:
            rgb.red = p;
            rgb.green = v;
            rgb.blue = t;
            break;
        case 3:
            rgb.red = p;
            rgb.green = q;
            rgb.blue = v;
            break;
        case 4:
            rgb.red = t;
            rgb.green = p;
            rgb.blue = v;
            break;
        default: // case 5:
            rgb.red = v;
            rgb.green = p;
            rgb.blue = q;
            break;
    }

    return rgb;
}

LED_Strip::HSV_Color LED_Strip::rgb2hsv(RGB_Color rgb) {
    HSV_Color hsv;

    uint8_t r = rgb.red;
    uint8_t g = rgb.green;
    uint8_t b = rgb.blue;

    // Находим максимальное и минимальное значения
    uint8_t max_val = r;
    if (g > max_val) max_val = g;
    if (b > max_val) max_val = b;

    uint8_t min_val = r;
    if (g < min_val) min_val = g;
    if (b < min_val) min_val = b;

    uint8_t delta = max_val - min_val;

    // Value - это максимальное значение
    hsv.value = max_val;

    // Saturation
    if (max_val == 0) {
        hsv.saturation = 0;
    } else {
        hsv.saturation = (delta * 255) / max_val;
    }

    // Hue
    if (delta == 0) {
        hsv.hue = 0; // Undefined, устанавливаем в 0
    } else {
        uint16_t hue_temp;

        if (max_val == r) {
            // Красный сектор
            if (g >= b) {
                hue_temp = (43 * (g - b)) / delta;
            } else {
                hue_temp = 255 - (43 * (b - g)) / delta;
            }
        } else if (max_val == g) {
            // Зеленый сектор
            hue_temp = 85 + (43 * (b - r)) / delta;  // 85 = 255/3
        } else {
            // Синий сектор
            hue_temp = 171 + (43 * (r - g)) / delta; // 171 = 255*2/3
        }

        // Приводим к диапазону 0-255
        if (hue_temp >= 255) {
            hsv.hue = hue_temp - 255;
        } else {
            hsv.hue = hue_temp;
        }
    }

    return hsv;
}



