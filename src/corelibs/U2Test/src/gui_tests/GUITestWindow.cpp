/**
 * UGENE - Integrated Bioinformatics Tools.
 * Copyright (C) 2008-2013 UniPro <ugene@unipro.ru>
 * http://ugene.unipro.ru
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston,
 * MA 02110-1301, USA.
 */

#include "GUITestWindow.h"
#include <U2Core/U2SafePoints.h>

#define CLASS_NAME_CN 0
#define OBJ_NAME_CN 1
#define ACT_NAME_CN 2
#define ACT_TEXT_CN 3

namespace U2 {

GUITestingWindow::GUITestingWindow(){
    setParent(AppContext::getMainWindow()->getQMainWindow(), Qt::Window);
    setObjectName("GUITestingWindow");
    prepareGUI();

    filter = new EventFilter(this);
    filter->setParent(this);
    QApplication::instance()->installEventFilter(filter);
}

void GUITestingWindow::prepareGUI(){
    setupUi(this);

    tableWidget->setItem(CLASS_NAME_CN, 0, new QTableWidgetItem());
    tableWidget->setItem(OBJ_NAME_CN, 0, new QTableWidgetItem());
    tableWidget->setItem(ACT_NAME_CN, 0, new QTableWidgetItem());
    tableWidget->setItem(ACT_TEXT_CN, 0, new QTableWidgetItem());

    connect(pushButton, SIGNAL(clicked()), this, SLOT(sl_getParentInfo()));
    show();
}

void GUITestingWindow::updateTable(){
    tableWidget->item(CLASS_NAME_CN,0)->setText(filter->getClassName());
    tableWidget->item(OBJ_NAME_CN,0)->setText(filter->getObjName());
    tableWidget->item(ACT_NAME_CN,0)->setText(filter->getActionName());
    tableWidget->item(ACT_TEXT_CN,0)->setText(filter->getActionText());
    label->setText("");
}

void GUITestingWindow::sl_getParentInfo(){
    QObject* o = filter->getBufferObject();
    if(o == NULL || (o->parent())==NULL){
        label->setText("parent is NULL!");
        return;
    }
    filter->getInfo(qobject_cast<QWidget*>(filter->getBufferObject()->parent()));
    updateTable();
}

bool EventFilter::eventFilter(QObject *obj, QEvent *event){
    m = dynamic_cast<QMouseEvent*>(event);

    if(m && (event->type()==QEvent::MouseButtonPress)){
        QObject* o = obj;
        while(o!=NULL){
            if (o == gtw){return QObject::eventFilter(obj, event);}
            o = o->parent();
        }
        generateMouseMeassage();
    }
    // standard event processing
    return QObject::eventFilter(obj, event);
}

void EventFilter::generateMouseMeassage(){
    CHECK_EXT(m,coreLog.error(QString("MouseEvent is NULL %1:%2").arg(__FILE__).arg(__LINE__)),);

    //widget info
    QWidget* w = QApplication::widgetAt(m->globalPos());
    if(w){
        getInfo(w);
        gtw->updateTable();
        gtw->textEdit->setText(gtw->textEdit->toPlainText().append(QString("\nGTWidget::click(os, %1);").arg(objName)));
    }
}

void EventFilter::getInfo(QWidget* w){
    CHECK(w,);
    setBufferObject(w);
    QMenu* menu = qobject_cast<QMenu*>(w);
    if(menu){
        CHECK(m,)
        QAction* menuAct = menu->actionAt(menu->mapFromGlobal(m->globalPos()));
        if (menuAct){
            className = menuAct->metaObject()->className();
            objName = menuAct->objectName();
            actionName = menuAct->objectName();
            text = menuAct->text();
            return;
        }
    }

    QMenuBar* menuBar = qobject_cast<QMenuBar*>(w);
    if(menuBar){
        CHECK(m,)
        QAction* menuBarAct = menuBar->actionAt(menuBar->mapFromGlobal(m->globalPos()));
        if(menuBarAct){
            className = menuBarAct->metaObject()->className();
            objName = menuBarAct->objectName();
            actionName = menuBarAct->objectName();
            text = menuBarAct->text();
            return;
        }
    }

    className = w->metaObject()->className();
    objName = w->objectName();
    QToolButton* toolButton = qobject_cast<QToolButton*>(w);
    if(toolButton && toolButton->defaultAction()){
       actionName = toolButton->defaultAction()->objectName();
       text = toolButton->defaultAction()->text();
       return;
    }
    actionName = "";
    text = "";
}

EventFilter::EventFilter(GUITestingWindow *_w): gtw(_w),bufferObj(NULL){
}

}
