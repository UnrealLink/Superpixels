#include <gtkmm/main.h>
#include <gtkmm/window.h>

#include "main_window.h"

int main(int argc, char* argv[]) {
    // initialize GUI
    Gtk::Main app(argc, argv);

    // create window
    MainWindow window("../data/fruits.jpg");

    // run GUI
    Gtk::Main::run(window);
    return 0;
}
