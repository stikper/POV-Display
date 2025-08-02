//
// Created by kiril on 01.08.2025.
//

#include "POV_Converter.h"

#include <iostream>
#include <fstream>
#include <format>

POV_Converter::POV_Converter(const POV_config_t& cfg) {
    config = cfg;
    init();
}

POV_Converter::~POV_Converter() {
    cv::destroyAllWindows();
};


// Private methods

void POV_Converter::init() {
    POV_diameter = std::max(config.length - config.center_pos, config.center_pos) * 2;
    led2mm = config.length / (config.leds - 1);
    mm2pix = 1;
    pov_data = std::vector(config.sectors, std::vector<cv::Vec3b>(config.leds));
    leds_pos_pix = std::vector(config.sectors, std::vector<std::pair<double, double>>(config.leds));
    calcLedsPos();
}

std::pair<double, double> POV_Converter::getLedPos(int led_num, int sector_num) const {
    double angle = static_cast<double>(sector_num) / config.sectors * CV_2PI;
    if (config.direction) angle *= -1;

    double coords_from_first_led = led_num * led2mm;
    double coords_from_center = coords_from_first_led - config.center_pos;

    std::pair<double, double> result;
    result.first = coords_from_center * cos(angle) + POV_diameter / 2;
    result.second = -1 * coords_from_center * sin(angle) + POV_diameter / 2;

    return result;
}

std::vector<std::pair<double, double>> POV_Converter::getLineLedsPos(int sector_num) const {
    std::vector<std::pair<double, double>> result(config.leds);
    for (int i = 0; i < config.leds; i++) {
        result[i] = getLedPos(i, sector_num);
    }
    return result;
}

void POV_Converter::calcLedsPos() {
    leds_pos = std::vector<std::vector<std::pair<double, double>>>(config.sectors);
    for (int i = 0; i < config.sectors; i++) {
        leds_pos[i] = getLineLedsPos(i);
    }
}

void POV_Converter::recalcLedsPosPix() {
    for (int i = 0; i < config.sectors; i++) {
        for (int j = 0; j < config.leds; j++) {
            leds_pos_pix[i][j].first = leds_pos[i][j].first * mm2pix;
            leds_pos_pix[i][j].second = leds_pos[i][j].second * mm2pix;
        }
    }
}

cv::Vec3b POV_Converter::getPixel(std::pair<double, double> pos) const {
    int x1 = floor(pos.first);
    int y1 = floor(pos.second);
    int x2 = x1 + 1;
    int y2 = y1 + 1;

    if (x2 >= image.cols) x2 = x1;
    if (y2 >= image.rows) y2 = y1;

    double wx = pos.first - x1;
    double wy = pos.second - y1;

    cv::Vec3b color = image.at<cv::Vec3b>(y1, x1) * (1-wx) * (1-wy) + image.at<cv::Vec3b>(y1, x2) * wx * (1-wy) +
            image.at<cv::Vec3b>(y2, x1) * wy * (1-wx) + image.at<cv::Vec3b>(y2, x2) * wx * wy;
    return color;
}

void POV_Converter::processImage() {
    // Crop
    int width = image.cols;
    int height = image.rows;

    cv::Rect roi;
    if (width > height) {
        roi = cv::Rect((width - height) / 2, 0, height, height);
    } else {
        roi = cv::Rect(0, (height - width) / 2, width, width);
    }
    image = image(roi);

    // Calculate mm2pix
    height = image.rows;
    mm2pix = static_cast<double>(height - 1) / POV_diameter;

    // Recalculate Leds positions
    recalcLedsPosPix();
}


// Public methods

int POV_Converter::loadImage(const cv::String& filename) {
    image = cv::imread(filename, cv::IMREAD_COLOR_BGR);

    if (image.empty()) {
        return -1;
    }
    processImage();
    return 0;
}



void POV_Converter::convert() {
    for (int i = 0; i < config.sectors; i++) {
        for (int j = 0; j < config.leds; j++) {
            pov_data[i][j] = getPixel(leds_pos_pix[i][j]);
        }
    }
}




void POV_Converter::simulate() {
    int upscale_factor = 4;
    int img_size = POV_diameter * upscale_factor;

    constexpr int dot_radius_mm = 1.5;
    const int dot_radius_pix = std::max(1, dot_radius_mm * upscale_factor);
    int border_size = POV_diameter / 10;
    img_size += border_size * 2;
    simulation = cv::Mat::zeros(img_size, img_size, CV_8UC3);

    for (int i = 0; i < config.sectors; i++) {
        for (int j = 0; j < config.leds; j++) {
            double x_mm = leds_pos[i][j].first;
            double y_mm = leds_pos[i][j].second;

            int x = static_cast<int>(x_mm  * upscale_factor) + border_size;
            int y = static_cast<int>(y_mm * upscale_factor) + border_size;

            if (x >= 0 && y >= 0 && x < simulation.cols && y < simulation.rows) {
                cv::circle(simulation, cv::Point(x, y), dot_radius_pix, pov_data[i][j], -1, cv::LINE_AA);
            }
        }
    }
}

void POV_Converter::showImage() const {
    cv::namedWindow("POV Converter image", cv::WINDOW_KEEPRATIO);
    cv::imshow("POV Converter image", image);
    cv::resizeWindow("POV Converter image", 1000, 1000);
    cv::waitKey(0);
}

void POV_Converter::showPOV() const {
    cv::namedWindow("POV Simulation", cv::WINDOW_KEEPRATIO);
    cv::imshow("POV Simulation", simulation);
    cv::resizeWindow("POV Simulation", 1000, 1000);
    cv::waitKey(0);
}

void POV_Converter::savePOVImage(const cv::String &filename) const {
    cv::imwrite(filename, simulation);
}

void POV_Converter::savePOVData(const std::string& filename) const {
    std::ofstream file(filename);
    file << std::format("const uint8_t pov_data[{}][{}][3] = {{", config.sectors, config.leds) << "\n";
    for (int i = 0; i < config.sectors; i++) {
        file << "  {\n";
        for (int j = 0; j < config.leds; j++) {
            file << std::format("    {{{}, {}, {}}}", pov_data[i][j][2], pov_data[i][j][1], pov_data[i][j][0]);
            file << ",\n";
        }
        file << "  },\n";
    }
    file << "};\n";
    file.close();
}


void POV_Converter::runCameraLoop(int camera_id) {
    cv::VideoCapture cap(camera_id);
    if (!cap.isOpened()) {
        std::cerr << "Ошибка: не удалось открыть камеру " << camera_id << std::endl;
        return;
    }
    cv::Mat frame;
     while (true) {
        cap >> frame;
        if (frame.empty()) break;

        // Копируем кадр
        image = frame.clone();

        // Обработка
        processImage();  // crop и recalc
        convert();       // преобразуем в pov_data
        simulate();      // отрисовываем симуляцию

        // Показываем обрезанное изображение
        cv::imshow("Camera Input (cropped)", image);

        // Показываем симуляцию POV
        cv::imshow("POV Simulation", simulation);

        int key = cv::waitKey(1);
        if (key == 27) break;
    }

    cap.release();
    cv::destroyAllWindows();
}







