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

    typedef std::vector<std::vector<std::pair<double, double> > > leds_pos_t; // [angle_i][led_i] = coords
    typedef std::vector<std::vector<cv::Vec3b> > POV_data_t; // [angle_i][led_i] = BGR

private:
    POV_config_t config;

    double POV_diameter;
    double led2mm; // Distance between two leds (center)

    cv::Mat image;
    cv::Mat simulation;
    double mm2pix;

    leds_pos_t leds_pos;
    leds_pos_t leds_pos_pix;
    POV_data_t pov_data;

    void init();

    std::pair<double, double> getLedPos(int led_num, int sector_num) const; // coords, mm (from the top left corner)
    std::vector<std::pair<double, double> > getLineLedsPos(int sector_num) const;

    void calcLedsPos(); // Calculate in mm coords
    void recalcLedsPosPix(); // Calculate in pixel coords

    cv::Vec3b getPixel(std::pair<double, double> pos) const;

    void processImage();

public:
    explicit POV_Converter(const POV_config_t &cfg);

    ~POV_Converter();

    int loadImage(const cv::String &filename);

    void convert(); // Calculate POV data
    void simulate(); // Simulate POV

    void showImage() const;

    void showPOV() const;

    void savePOVImage(const cv::String &filename = "simulation.png") const;

    void savePOVData(const std::string &filename = "pov_data.txt") const;

    void runCameraLoop(int camera_id = 0);
};


#endif //POV_CONVERTER_H
