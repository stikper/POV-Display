#include <opencv2/opencv.hpp>
#include <iostream>
#include <string>

using namespace std;
using namespace cv;

int main() {
    string file_name = "../image.png";

    Mat image = imread(file_name, IMREAD_COLOR_RGB);

    if (image.empty())
    {
        cout << "Could not open or find the image" << endl;
        cin.get(); //wait for any key press
        return -1;
    }

    String windowName = "Image"; //Name of the window

    namedWindow(windowName); // Create a window

    imshow(windowName, image, IM); // Show our image inside the created window.

    waitKey(0); // Wait for any keystroke in the window

    destroyWindow(windowName); //destroy the created window


    return 0;
}
