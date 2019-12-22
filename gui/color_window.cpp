#include "color_window.h"


/*
  ColorPatch
*/

ColorPatch::ColorPatch(){
    r = 0.; g = 0.; b = 0.;
}

ColorPatch::~ColorPatch(){}

void ColorPatch::setColor(double _r, double _g, double _b) {
    r = _r; g = _g; b = _b;
}

bool ColorPatch::on_draw(const Cairo::RefPtr<Cairo::Context>& cr){
    Gtk::Allocation allocation = get_allocation();
    
    cr->set_source_rgb(r, g, b);
    cr->paint();

    return true;
}

/* 
   ColorWindow
*/

ColorWindow::ColorWindow() : VBox(false, 10)
{
    // sliders
    scaleR = new Gtk::HScale(0, 256, 1);
    pack_start(*scaleR);
    scaleR->signal_value_changed().connect([this]() {updateColor();});
    
    scaleG = new Gtk::HScale(0, 256, 1);
    pack_start(*scaleG);
    scaleG->signal_value_changed().connect([this]() {updateColor();});
    
    scaleB = new Gtk::HScale(0, 256, 1);
    pack_start(*scaleB);
    scaleB->signal_value_changed().connect([this]() {updateColor();});

    // color patch
    patch = new ColorPatch();
    pack_start(*patch);

}

ColorWindow::~ColorWindow(){
    delete scaleR;
    delete scaleG;
    delete scaleB;
    delete patch;
}

void ColorWindow::updateColor(){
    patch->setColor(getR()/255, getG()/255, getB()/255);
    patch->queue_draw();
}
