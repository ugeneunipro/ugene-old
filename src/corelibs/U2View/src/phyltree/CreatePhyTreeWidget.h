#ifndef _CREATEPHYTREEWIDGET_H_
#define _CREATEPHYTREEWIDGET_H_

#include <U2Core/global.h>
#include <U2Algorithm/CreatePhyTreeSettings.h>
#include <QtGui/QWidget>

namespace U2{

    class U2VIEW_EXPORT CreatePhyTreeWidget : public QWidget 
    {
    public:
        CreatePhyTreeWidget(QWidget* parent) : QWidget(parent) { }
        virtual void fillSettings(CreatePhyTreeSettings& settings) {Q_UNUSED(settings); } 
    };
}

#endif