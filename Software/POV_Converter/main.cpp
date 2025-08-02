#include <opencv2/opencv.hpp>
#include "POV_Converter.h"

using namespace std;

int main() {
    POV_Converter::POV_config_t pov_config;
    pov_config.leds = 144;
    pov_config.sectors = 141;
    pov_config.direction = false;
    pov_config.length = 1000;
    pov_config.center_pos = 501.75;

    POV_Converter converter(pov_config);

    converter.loadImage("../image.png");
    converter.showImage();
    converter.convert();
    converter.simulate();
    converter.showPOV();
    converter.savePOVImage("../simulation.png");
    converter.savePOVData("../pov_data.txt");

    converter.runCameraLoop(0);

    return 0;
}
