#include "cielab.h"
#include <opencv2/highgui/highgui.hpp>
#include <iostream>

using namespace cv;
using namespace std;

Image<Vec3b> convertBGRToLab(Image<Vec3b> imageBGR){
    int w = imageBGR.width();
    int h = imageBGR.height();
    Image<Vec3b> imageLab(w, h);
    cvtColor(imageBGR, imageLab, COLOR_BGR2Lab);
    return imageLab;
}