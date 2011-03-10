#ifndef _U2_WEB_WINDOW_H_
#define _U2_WEB_WINDOW_H_

#include <U2Core/global.h>
#include <U2Gui/MainWindow.h>

namespace U2 {

class U2VIEW_EXPORT WebWindow : public MWMDIWindow {
    Q_OBJECT
public:
    WebWindow(const QString& title, const QString& content);
};

}//namespace
#endif
