#include "ScaleBar.h"

#include <QtGui/QVBoxLayout>

namespace U2 {

    ScaleBar::ScaleBar(QWidget* parent) : QWidget(parent)    
    {
        scaleBar = new QSlider(Qt::Vertical);
        scaleBar->setTracking(true);
        scaleBar->setRange(100,2000);
        scaleBar->setTickPosition(QSlider::TicksLeft);
        scaleBar->setTickInterval(100);
        connect(scaleBar,SIGNAL(valueChanged(int)),SIGNAL(valueChanged(int)));

        minusButton = new QToolButton();
        //icon
        minusButton->setText(QString(tr("-")));
        minusButton->setIcon(QIcon(":core/images/minus.png"));
        minusButton->setFixedSize(20,20);
        minusButton->setAutoRepeat(true);
        minusButton->setAutoRepeatInterval(20);
        plusButton = new QToolButton();
        //icon
        plusButton->setText(QString(tr("+")));
        plusButton->setIcon(QIcon(":core/images/plus.png"));
        plusButton->setAutoRepeat(true);
        plusButton->setAutoRepeatInterval(20);
        plusButton->setFixedSize(20,20);
        connect(minusButton,SIGNAL(clicked()),SLOT(sl_minusButtonClicked()));
        connect(plusButton,SIGNAL(clicked()),SLOT(sl_plusButtonClicked()));

        //layout
        QVBoxLayout *zoomLayout = new QVBoxLayout();
        zoomLayout->addWidget(plusButton);
        zoomLayout->addWidget(scaleBar);
        zoomLayout->addWidget(minusButton);
        zoomLayout->setMargin(0);
        zoomLayout->setSpacing(0);
        setLayout(zoomLayout);
        setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Preferred);
    }

    void ScaleBar::sl_minusButtonClicked() {
        scaleBar->setValue(scaleBar->value()-scaleBar->pageStep());
    }

    void ScaleBar::sl_plusButtonClicked() {
        scaleBar->setValue(scaleBar->value()+scaleBar->pageStep());
    }

}//namespace
