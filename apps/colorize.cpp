#include <gtkmm/main.h>

#include "color_window.h"

int main(int argc, char* argv[]) {
    // initialize GUI
    Gtk::Main app(argc, argv);

    // create window
    ColorWindow window;

    // run GUI
    Gtk::Main::run(window);
    return 0;
}
