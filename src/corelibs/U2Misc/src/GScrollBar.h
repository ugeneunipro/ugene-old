#ifndef _U2_GSCROLL_BAR_H_
#define _U2_GSCROLL_BAR_H_

#include <U2Core/global.h>
#include <QtGui/QScrollBar>

namespace U2 {

class U2MISC_EXPORT GScrollBar : public QScrollBar {
public:
    GScrollBar(Qt::Orientation o, QWidget *p=0) : QScrollBar(o, p){}

    void setupRepeatAction(SliderAction action, int thresholdTime=100, int repeatTime=50) {
        QScrollBar::setRepeatAction(action, thresholdTime, repeatTime);
    }

    SliderAction getRepeatAction() const {return repeatAction();}
};


} //namespace

#endif
