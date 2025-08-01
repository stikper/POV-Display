//
// Created by kiril on 01.08.2025.
//

#include "POV_Converter.h"

POV_Converter::POV_Converter(const POV_config_t& cfg) {
    config = cfg;
    POV_diameter = std::max(cfg.length - cfg.center_pos, cfg.center_pos) * 2;
    led2mm = cfg.length / (cfg.leds - 1);
    mm2pix = 1;
    pov_data = std::vector<std::vector<cv::Vec3b>>(cfg.sectors);
}

POV_Converter::~POV_Converter() = default;

int POV_Converter::loadImage(const cv::String& filename) {
    image_bgr = cv::imread(filename, cv::IMREAD_COLOR_BGR);

    if (image_bgr.empty()) {
        return -1;
    }
    return 0;
}

void POV_Converter::prepareImage() {
    // Crop
    int width = image_bgr.cols;
    int height = image_bgr.rows;

    cv::Rect roi;
    if (width > height) {
        roi = cv::Rect((width - height) / 2, 0, height, height);
    } else {
        roi = cv::Rect(0, (height - width) / 2, width, width);
    }
    image_bgr = image_bgr(roi);

    // Calculate mm2pix
    height = image_bgr.rows;
    mm2pix = static_cast<double>(height) / POV_diameter;
}

void POV_Converter::showImage() const {
    cv::imshow("POV Converter image", image_bgr);
    cv::waitKey(0);
}



void POV_Converter::convert() {
    for (int i = 0; i < config.sectors; i++) {
        std::vector<std::pair<double, double>> positions = calcLedsPositions(i);

        for (int j = 0; j < positions.size(); j++) {
            positions[j].first *= mm2pix;
            positions[j].second *= mm2pix;
        }

        std::vector<cv::Vec3b> line;
        for (int j = 0; j < positions.size(); j++) {
            line.push_back(getPixel(positions[j]));
        }
        pov_data[i] = line;
    }

}

std::pair<double, double> POV_Converter::calcLedPosition(int led_num, int sector_num) const {
    double angle = static_cast<double>(sector_num) / config.sectors * CV_2PI;
    if (config.direction) angle *= -1;

    double coords_from_first_led = led2mm * led_num;
    double coords_from_center = coords_from_first_led - config.center_pos;

    std::pair<double, double> result;
    result.first = coords_from_center * cos(angle) + POV_diameter / 2;
    result.second = -1 * coords_from_center * sin(angle) + POV_diameter / 2;

    return result;
}

std::vector<std::pair<double, double>> POV_Converter::calcLedsPositions(int sector_num) const {
    std::vector<std::pair<double, double>> result(config.leds);
    for (int i = 0; i < config.leds; i++) {
        result[i] = calcLedPosition(i, sector_num);
    }
    return result;
}


// TODO: FIX BORDER
cv::Vec3b POV_Converter::getPixel(std::pair<double, double> pos) const {
    int x1 = floor(pos.first) - 1;
    int y1 = floor(pos.second) - 1;
    int x2 = x1 + 1;
    int y2 = y1 + 1;

    if (x1 < 0) x1 = x2;
    if (y1 < 0) y1 = y2;
    if (x2 >= image_bgr.cols) x2 = x1;
    if (y2 >= image_bgr.rows) y2 = y1;

    double wx = pos.first - 1 - x1;
    double wy = pos.second - 1 - y1;

    cv::Vec3b color = image_bgr.at<cv::Vec3b>(y1, x1) * (1-wx) * (1-wy) + image_bgr.at<cv::Vec3b>(y1, x2) * wx * (1-wy) +
            image_bgr.at<cv::Vec3b>(y2, x1) * wy * (1-wx) + image_bgr.at<cv::Vec3b>(y2, x2) * wx * wy;
    return color;
}

void POV_Converter::simulate() const {
    int img_size = POV_diameter * mm2pix;
    cv::Mat simulation = cv::Mat::zeros(img_size, img_size, CV_8UC3);

    const int dot_radius_mm = 1.5;
    const int dot_radius_pix = std::max(1, static_cast<int>(dot_radius_mm * mm2pix));

    for (int sector = 0; sector < config.sectors; ++sector) {
        std::vector<std::pair<double, double>> positions = calcLedsPositions(sector);
        for (int led = 0; led < config.leds; ++led) {
            double x_mm = positions[led].first;
            double y_mm = positions[led].second;

            int x = static_cast<int>(x_mm * mm2pix);
            int y = static_cast<int>(y_mm * mm2pix);

            if (x >= 0 && y >= 0 && x < simulation.cols && y < simulation.rows) {
                cv::circle(simulation, cv::Point(x, y), dot_radius_pix, pov_data[sector][led], -1, cv::LINE_AA);
            }
        }
    }

    cv::imshow("POV Simulation", simulation);
    cv::waitKey(0);
}







