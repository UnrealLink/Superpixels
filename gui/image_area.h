#pragma once

#include <string>

#include <gtkmm/drawingarea.h>
#include <cairomm/context.h>
#include <gdkmm/pixbuf.h>

#include "image.h"

using namespace std;


class ImageArea : public Gtk::DrawingArea {
 public:
    ImageArea();
    ~ImageArea();

    void load_image(string filename);
    void set_pixel(int x, int y, const Vec3b& color);

 protected:
    bool on_draw(const Cairo::RefPtr<Cairo::Context>& cr);

 private:
    Image<Vec3b> img;
    
};
