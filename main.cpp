#include "image.h"
#include "cielab.h"
#include <opencv2/highgui/highgui.hpp>
#include <iostream>

using namespace cv;
using namespace std;

int main(int argc, char** argv)
{
	Image<Vec3b> image = (Image<Vec3b>)imread("../loic.jpg");
	imshow("I1", image);

	waitKey(0);
	return 0;
}