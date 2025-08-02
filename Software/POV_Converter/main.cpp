#include <opencv2/opencv.hpp>
#include <iostream>
#include <string>
#include "POV_Converter.h"

using namespace std;

int main() {
    POV_Converter::POV_config_t pov_config;
    pov_config.leds = 144;
    pov_config.sectors = 141;
    pov_config.length = 1000;
    pov_config.center_pos = 501.75;
    pov_config.direction = false;

    POV_Converter converter(pov_config);

    converter.loadImage("../image.png");
    converter.showImage();
    converter.prepareImage();
    converter.showImage();
    converter.convert();
    converter.simulate();

    return 0;
}
