#include "MainWindow.h"

namespace U2 {

//thread safe design in main thread model
static int windowCounter = 0;

MWMDIWindow::MWMDIWindow(const QString& windowName) : windowId(++windowCounter) 
{ 
    setWindowTitle(windowName);
}

} //namespace
