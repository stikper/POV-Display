//
// Created by kiril on 01.08.2025.
//
#pragma once
#ifndef POV_CONVERTER_H
#define POV_CONVERTER_H

#include <opencv2/opencv.hpp>
#include <vector>

class POV_Converter {
public:
    struct POV_config_t {
        int leds = 144;
        int sectors = 140;
        bool direction = false; // false = CCW; true = CW
        double length = 1000; // In millimeters
        double center_pos = 500; // From the first led
    };

private:
    POV_config_t config;
    double POV_diameter;
    double mm2pix;
    double led2mm;

    cv::Mat image_bgr;
    std::vector<std::vector<cv::Vec3b>> pov_data; // [angle][led] = RGB

    std::pair<double, double> calcLedPosition(int led_num, int sector_num) const; // coords, mm (from the top left corner)
    std::vector<std::pair<double, double>> calcLedsPositions(int sector_num) const;
    cv::Vec3b getPixel(std::pair<double, double> pos) const;

public:
    explicit POV_Converter(const POV_config_t& cfg);
    ~POV_Converter();

    int loadImage(const cv::String& filename); // load image to cv::Mat
    void prepareImage(); // Crop image to square

    void showImage() const; // Show image_bgr

    void convert(); // Calculate POV data
    std::vector<std::vector<cv::Vec3b>> getPOVData();

    void simulate() const;
    void savePOVData();

};



#endif //POV_CONVERTER_H
