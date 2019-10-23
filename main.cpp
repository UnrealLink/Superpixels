#include "image.h"
#include "cielab.h"
#include "slic.h"
#include <opencv2/highgui/highgui.hpp>
#include <iostream>

using namespace cv;
using namespace std;

int main(int argc, char** argv)
{
	Image<Vec3b> image = (Image<Vec3b>)imread("../loic.jpg");
	imshow("I1", image);
    Image<Vec3b> imageLab = convertBGRToLab(image);
    imshow("Lab", imageLab);
    Slic slic(imageLab, 10);

	waitKey(0);
	return 0;
}