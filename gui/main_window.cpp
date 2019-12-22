#include "main_window.h"

#include <queue>
#include <iostream>

#include <opencv2/highgui/highgui.hpp>

using namespace std;
using namespace cv;


MainWindow::MainWindow(string filename) {
    // initialize SLIC
    Image<Vec3b> imageLab = (Image<Vec3b>)imread(filename);
    slic = new Slic(imageLab, 1000);
    
    // initialize widgets & store in horizontal box
    resize(1333, 1000);
    
    hbox = new Gtk::HBox(true, 10);
    
    imgArea = new ImageArea();
    imgArea->load_image(filename);
    imgArea->add_events(Gdk::BUTTON_PRESS_MASK);
    imgArea->signal_button_press_event().connect([this](GdkEventButton* button_event)
						 {
						     update_cluster_color();
						     imgArea->queue_draw();
						     return true;
						 });
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

void MainWindow::update_cluster_color(){
    Vec3b color;
    color[0] = (int)colorWin->getR();
    color[1] = (int)colorWin->getG();
    color[2] = (int)colorWin->getB();
    
    int x0, y0;
    imgArea->get_pointer(x0, y0);
    int c = slic->getCluster(x0, y0);
    
    queue<Point> Q;
    Q.push(Point(x0, y0));

    int w = slic->getImage().width();
    int h = slic->getImage().height();
    Image<uchar> seen(w, h);
    seen.setTo(0);
    seen(x0, y0) = 1;

    while (!Q.empty()) {
	int x = Q.front().x;
	int y = Q.front().y;
	Q.pop();
	imgArea->set_pixel(x, y, color);

	if (x > 0 && !seen(x-1, y) && slic->getCluster(x-1, y) == c) {
	    seen(x-1, y) = 1;
	    Q.push(Point(x-1, y));
	}
	if (x < (w-1) && !seen(x+1, y) && slic->getCluster(x+1, y) == c) {
	    seen(x+1, y) = 1;
	    Q.push(Point(x+1, y));
	}
	if (y > 0 && !seen(x, y-1) && slic->getCluster(x, y-1) == c) {
	    seen(x, y-1) = 1;
	    Q.push(Point(x, y-1));
	}
	if (y < (h-1) && !seen(x, y+1) && slic->getCluster(x, y+1) == c) {
	    seen(x, y+1) = 1;
	    Q.push(Point(x, y+1));
	}
    }
}
