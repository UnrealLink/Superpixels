#pragma once

#include <string>

#include <gtkmm/window.h>
#include <gtkmm/box.h>

#include "image.h"

#include "image_area.h"
#include "color_window.h"

using namespace std;


class MainWindow : public Gtk::Window {
 public:
    MainWindow(string filename);
    ~MainWindow();

 private:
    Gtk::HBox* hbox;
    ImageArea* imgArea;
    ColorWindow* colorWin;
    
};
