#pragma once

#include <gtkmm/window.h>
#include <gtkmm/box.h>
#include <gtkmm/scale.h>

#include <gtkmm/drawingarea.h>
#include <cairomm/context.h>


class ColorPatch : public Gtk::DrawingArea {
 public:
    ColorPatch();
    ~ColorPatch();

    void setColor(double _r, double _g, double _b);

 protected:
    bool on_draw(const Cairo::RefPtr<Cairo::Context>& cr);

 private:
    double r, g, b;
    
};


class ColorWindow : public Gtk::Window {
 public:
    ColorWindow();
    ~ColorWindow();

    // RGB values on sliders (from 0 to 255)
    double getR(){ return scaleR->get_value(); }
    double getG(){ return scaleG->get_value(); }
    double getB(){ return scaleB->get_value(); }

 private:
    // vertical box for display
    Gtk::VBox* vbox;

    // RGB sliders
    Gtk::HScale* scaleR;
    Gtk::HScale* scaleG;
    Gtk::HScale* scaleB;

    // patch to show resulting color
    ColorPatch* patch;
    void updateColor();
    
};
