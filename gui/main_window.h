#pragma once

#include <string>

#include <gtkmm/window.h>
#include <gtkmm/box.h>

#include "image.h"
#include "slic.h"

#include "image_area.h"
#include "color_window.h"

using namespace std;


class MainWindow : public Gtk::Window {
 public:
    MainWindow(string filename);
    ~MainWindow();

 private:
    Slic* slic;
    Gtk::HBox* hbox;
    ImageArea* imgArea;
    ColorWindow* colorWin;

    void update_cluster_color();
    
};
