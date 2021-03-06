#include "image_area.h"

#include <cassert>
#include <iostream>

#include <gdkmm/general.h>

#include <opencv2/highgui/highgui.hpp>

using namespace std;
using namespace cv;


ImageArea::ImageArea(){}

ImageArea::~ImageArea(){}

void ImageArea::load_image(string filename) {
    Image<Vec3b> I;
    I = (Image<Vec3b>)imread(filename);
    cvtColor(I, img, COLOR_BGR2RGB);
}

void ImageArea::set_pixel(int x, int y, const Vec3b& color){
    assert((x >= 0) && (x < img.width()));
    assert((y >= 0) && (y < img.height()));

    img(x, y) = color;
}

bool ImageArea::on_draw(const Cairo::RefPtr<Cairo::Context>& cr){
    Gtk::Allocation allocation = get_allocation();
    Gdk::Cairo::set_source_pixbuf(cr,
				  Gdk::Pixbuf::create_from_data(img.data,
								Gdk::COLORSPACE_RGB, false, 8,
								img.cols, img.rows, img.step)
				  );
    cr->paint();
    return true;
}
