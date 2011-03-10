#ifndef _U2_SCALE_BAR_H_
#define _U2_SCALE_BAR_H_

#include <U2Core/global.h>

#include <QtGui/QSlider>
#include <QtGui/QToolButton>

namespace U2 {

class U2MISC_EXPORT ScaleBar : public QWidget {
    Q_OBJECT
public:
    ScaleBar(QWidget* parent = 0);
    QSlider* slider() const {return scaleBar;}
    int value() const {return scaleBar->value();}
    void setValue(int v) {scaleBar->setValue(v);}

signals:
    void valueChanged(int);

private slots:
    void sl_minusButtonClicked();
    void sl_plusButtonClicked();

protected:

    QSlider*                    scaleBar;
    QToolButton*                minusButton;
    QToolButton*                plusButton;
};

} // namespace

#endif
