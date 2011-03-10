#ifndef _U2_HBAR_H_
#define _U2_HBAR_H_

#include <U2Core/global.h>
#include <QtGui/QToolBar>

namespace U2 {

    ///////////////////////////////////////////////////////////////////////////////////////////
    // Header widget toolbar

    class U2MISC_EXPORT HBar : public QToolBar {
    public:
        HBar(QWidget* w) : QToolBar(w){}
    protected:
        void paintEvent(QPaintEvent* pe) {
            Q_UNUSED(pe);
            //do not draw any special toolbar control -> make is merged with parent widget
        }
    };

} //namespace

#endif
