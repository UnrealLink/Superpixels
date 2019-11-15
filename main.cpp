#include "image.h"
#include "cielab.h"
#include "slic.h"
#include "spm.h"
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
    Slic slic(imageLab, 1000);
    slic.showSuperpixels();
    std::cout << "check1" << std::endl;
    SuperPatchMatcher spm(slic, slic);
    std::cout << "check2" << std::endl;
    spm.computeANNs();
    std::cout << "done" << std::endl;

	waitKey(0);
	return 0;
}
