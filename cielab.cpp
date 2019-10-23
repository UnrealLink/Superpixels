#include "cielab.h"
#include <opencv2/highgui/highgui.hpp>
#include <iostream>

using namespace cv;
using namespace std;

Image<Vec3f> convertBGRToLab(Image<Vec3b> imageBGR){
    int w = imageBGR.width();
    int h = imageBGR.height();
    Image<Vec3f> imageLab(w, h);
    imageBGR.convertTo(imageLab, COLOR_BGR2Lab);
    imshow("testLab", imageLab);
    return imageLab;
}