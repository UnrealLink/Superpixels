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
    Image<Vec3b> colorPalette;
    if (argc < 2) {
        image = (Image<Vec3b>)imread("../data/loic.jpg");
        colorPalette = (Image<Vec3b>)imread("../data/fruits.jpg");
    }
    else {
        image = (Image<Vec3b>)imread(argv[1]);
        colorPalette = (Image<Vec3b>)imread(argv[2]);
    }
	std::cout << "Dimension of image : " << image.width() << "x" << image.height() << std::endl;
    std::cout << "Dimension of color palette : " << colorPalette.width() << "x" << colorPalette.height() << std::endl;
	imshow("Image", image);
    Image<Vec3b> imageLab = convertBGRToLab(image);
    Slic slicImage(imageLab, 1000);
    slicImage.showSuperpixels("Image Superpixels");
    imshow("Color Palette", colorPalette);
    Image<Vec3b> colorPaletteLab = convertBGRToLab(colorPalette);
    Slic slicColorPalette(colorPaletteLab, 500);
    slicColorPalette.showSuperpixels("Color Palette Superpixels");
    SuperPatchMatcher spm(slicImage, slicColorPalette);
    spm.computeANNs();

	waitKey(0);
	return 0;
}
