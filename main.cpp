#include "image.h"
#include "cielab.h"
#include "slic.h"
#include <opencv2/highgui/highgui.hpp>
#include <iostream>

using namespace cv;
using namespace std;

int main(int argc, char** argv)
{
    Image<Vec3b> image;
    if (argc < 2) {
        image = (Image<Vec3b>)imread("../data/fruits.jpg");
    }
    else {
        image = (Image<Vec3b>)imread(argv[1]);
    }
	std::cout << "Dimension of image : " << image.width() << "," << image.height() << std::endl;
	imshow("I1", image);
    Image<Vec3b> imageLab = convertBGRToLab(image);
    Slic slic(imageLab, 500);
    slic.showSuperpixels();

	waitKey(0);
	return 0;
}
