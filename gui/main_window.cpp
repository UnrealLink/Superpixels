#include "main_window.h"

using namespace std;


MainWindow::MainWindow(string filename) {
    resize(1333, 1000);
    
    hbox = new Gtk::HBox(true, 10);
    
    imgArea = new ImageArea();
    imgArea->load_image(filename);
    hbox->pack_start(*imgArea);
    
    colorWin = new ColorWindow();
    hbox->pack_start(*colorWin);

    add(*hbox);
    show_all();
}

MainWindow::~MainWindow(){
    delete hbox;
    delete imgArea;
    delete colorWin;
}
