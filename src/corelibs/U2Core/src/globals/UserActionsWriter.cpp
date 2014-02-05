/**
 * UGENE - Integrated Bioinformatics Tools.
 * Copyright (C) 2008-2014 UniPro <ugene@unipro.ru>
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

#include "UserActionsWriter.h"
#include <QtGui/QMouseEvent>
#include <QtGui/QApplication>
#include <QtGui/QWidget>
#include <QtGui/QLabel>
#include <QtGui/QPushButton>
#include <QtGui/QLineEdit>
#include <QtGui/QMenu>
#include <QtGui/QMenuBar>
#include <QtGui/QTreeWidget>
#include <QtGui/QListWidget>
#include <QtGui/QMainWindow>
#include <U2Gui/MainWindow.h>
#include <QtGui/QDialog>
#include <QtGui/QMessageBox>
#include <QtGui/QAbstractSpinBox>
#include "Log.h"
#include "U2SafePoints.h"

#define BUFFER_CONDITION(keyEvent) (keyEvent->key()<=Qt::Key_QuoteLeft && keyEvent->key()>=Qt::Key_Space && (keyEvent->modifiers().testFlag(Qt::NoModifier) || keyEvent->modifiers().testFlag(Qt::KeypadModifier)))

namespace U2{

bool UserActionsWriter::eventFilter(QObject *obj, QEvent *event){
    message = "";
    k = dynamic_cast<QKeyEvent*>(event);
    m = dynamic_cast<QMouseEvent*>(event);

    //dialog info
    appendDialogInfo();

    if(m && (event->type()==QEvent::MouseButtonPress || event->type()==QEvent::MouseButtonRelease || event->type()==QEvent::MouseButtonDblClick)){
        generateMouseMeassage();
        return false;
    }
    else if( k ){
        generateKeyMessage();
        return false;
    }
    else{
        // standard event processing
        return QObject::eventFilter(obj, event);
    }
}

void UserActionsWriter::generateMouseMeassage(){
    CHECK_EXT(m,userActLog.error(QString("MouseEvent is NULL %1:%2").arg(__FILE__).arg(__LINE__)),);

    //button type
    appendMouseButtonInfo();

    QMainWindow* mainWindow = AppContext::getMainWindow()->getQMainWindow();
    CHECK_EXT(mainWindow ,userActLog.error(QString("Main window is NULL %1:%2").arg(__FILE__).arg(__LINE__)),);

    //window size info
    QPoint p = mainWindow->mapFromGlobal(mainWindow->geometry().bottomRight());
    if(p!=windowSize){
        windowSize =p;
        userActLog.trace(QString("WINDOW SIZE: %1x%2").arg(windowSize.x()).arg(windowSize.y()));
    }

    //coordinates
    QPoint windowP = mainWindow->mapFromGlobal(m->globalPos());
    message.append(QString("%1 %2 ").arg(windowP.x()).arg(windowP.y()));

    //widget info
    QWidget* w = QApplication::widgetAt(m->globalPos());
    if(w){
        QString className = w->metaObject()->className();

        //tree widget and list widget
        //sometimes QWidget is on top. it does not give any information, but it's parent does
        if(className == "QWidget"){
            QWidget* parent = qobject_cast<QWidget*>(w->parent());
            if(parent){
                appendTreeWidgetInfo(parent);
                w = parent;
            }
        }else{
            message.append("CLASS_NAME: ").append(className);
        }
        message.append(" ");

        //additional information
        appendAdditionalWidgetInfo(w);

    }
    else{
        message.append("Widget under corsor is NULL");
    }
    filterMessages();
}

void UserActionsWriter::appendMouseButtonInfo(){
    CHECK_EXT(m,userActLog.error(QString("MouseEvent is NULL %1:%2").arg(__FILE__).arg(__LINE__)),);
    switch(m->button()){
    case Qt::RightButton: {
        message.append("Right_button ");
        break;
    }
    case Qt::LeftButton: {
        message.append("Left_button ");
        break;
    }
    default:{
       message.append("Other_button ");
    }
    }
}

void UserActionsWriter::appendTreeWidgetInfo(QWidget *parent){
    CHECK_EXT(m,userActLog.error(QString("MouseEvent is NULL %1:%2").arg(__FILE__).arg(__LINE__)),);
    CHECK_EXT(parent,userActLog.error(QString("argument is NULL %1:%2").arg(__FILE__).arg(__LINE__)),);

    message.append("CLASS_NAME: ").append(parent->metaObject()->className());
    QTreeWidget* tree = qobject_cast<QTreeWidget*>(parent);
    if(tree){
        QTreeWidgetItem* item = tree->itemAt(m->pos());
        if(item){
            message.append(" TREE_ITEM: " + item->text(0));
        }
    }

    QListWidget* list = qobject_cast<QListWidget*>(parent);
    if(list){
        QListWidgetItem* item = list->itemAt(list->mapFromGlobal(m->globalPos()));
        if (item){
            message.append(" LIST_ITEM: " + item->text());
        }
    }
}

void UserActionsWriter::appendAdditionalWidgetInfo(QWidget *w){
    CHECK_EXT(m,userActLog.error(QString("MouseEvent is NULL %1:%2").arg(__FILE__).arg(__LINE__)),);
    CHECK_EXT(w,userActLog.error(QString("argument is NULL %1:%2").arg(__FILE__).arg(__LINE__)),);

    QString text = getWidgetText(w);
    QString objectName = w->objectName();
    QString tooltip = w->toolTip();

    if(!text.isEmpty() && (text != "...")){
        message.append("TEXT: " + text);
    }else if(!tooltip.isEmpty()){
        message.append("TOOLTIP: " + tooltip);
    }else if(!objectName.isEmpty()){
        message.append("OBJECT_NAME: " + objectName);
    }
    QAbstractSpinBox* spin = qobject_cast<QAbstractSpinBox*>(w);
    if(spin){
        message.append(" " + spin->text());
    }
}

QString UserActionsWriter::getWidgetText(QWidget* w){
    CHECK_EXT(w,userActLog.error(QString("argument is NULL %1:%2").arg(__FILE__).arg(__LINE__)), "Widget is NULL");

    QString text("");

    QLabel* l = qobject_cast<QLabel*>(w);
    if(l){
        text.append(l->text());
    }

    QAbstractButton* b=qobject_cast<QAbstractButton*>(w);
    if(b){
        text.append(b->text());
    }

    QMenu* menu = qobject_cast<QMenu*>(w);
    if(menu){
        QAction* menuAct = menu->actionAt(menu->mapFromGlobal(m->globalPos()));
        if (menuAct){
            text.append(menuAct->text());
        }
    }

    QMenuBar* menuBar = qobject_cast<QMenuBar*>(w);
    if(menuBar){
        QAction* menuBarAct = menuBar->actionAt(menuBar->mapFromGlobal(m->globalPos()));
        if(menuBarAct){
            text.append(menuBarAct->text());
        }
    }

    QLineEdit* lineEdit = qobject_cast<QLineEdit*>(w);
    if(lineEdit){
        text.append(lineEdit->text());
    }

    return text;
}

void UserActionsWriter::generateKeyMessage(){
    CHECK_EXT(k,userActLog.error(QString("key event is NULL %1:%2").arg(__FILE__).arg(__LINE__)),);

    QString text = k->text();
    QString s = keys.value(Qt::Key(k->key()));

    appendModifiersInfo();

    if(!s.isEmpty()){
        message.append(QString("%1").arg(s));
    }else if(!text.isEmpty()){
        message.append(text).append(QString(" code: %1").arg(k->key()));
    }else{
        message.append(QString("Undefined key, code: %1").arg(k->key()));
    }

    filterMessages();
}

void UserActionsWriter::filterMessages(){
    if(m){
        filterMouseMessages();
    }else if(k){
        filterKeyboardMessages();
    }
}

void UserActionsWriter::filterMouseMessages(){
    CHECK_EXT(m , userActLog.error(QString("mouse event is NULL %1:%2").arg(__FILE__).arg(__LINE__)),);

    message.prepend(typeMap.value(m->type()) + QString(" "));
    if(message != prevMessage){
        if(!buffer.isEmpty()){
            userActLog.trace(QString("Typed string. Length=%1").arg(buffer.length()));
            buffer = "";
        }

        if(counter!=0){
            userActLog.trace(QString("pressed %1 times").arg(counter+1));
            counter = 0;
        }

        /*Do not duplicate event information when logging mouse release event*/
        if(prevMessage.right(prevMessage.length()-typeMap.value(QEvent::MouseButtonPress).length()) ==
                message.right(message.length()-typeMap.value(QEvent::MouseButtonRelease).length())){
            userActLog.trace("mouse_release");
            prevMessage = message;
            return;
        }

        prevMessage = message;

        userActLog.trace(message);

        return;
    }
}

void UserActionsWriter::filterKeyboardMessages(){
    CHECK_EXT(k,userActLog.error(QString("key event is NULL %1:%2").arg(__FILE__).arg(__LINE__)),);

    message.prepend(typeMap.value(k->type()) + QString(" "));


    if(message != prevMessage && typeMap.value(k->type()) != NULL){
        /*Do not duplicate event information when logging key release event*/
        if(prevMessage.right(prevMessage.length()-typeMap.value(QEvent::KeyPress).length()) ==
                message.right(message.length()-typeMap.value(QEvent::KeyRelease).length())){
            prevMessage = message;
            return;
        }
        /*If one key pressed several times, count presses*/
        if(prevMessage.right(prevMessage.length()-typeMap.value(QEvent::KeyRelease).length()) ==
                message.right(message.length()-typeMap.value(QEvent::KeyPress).length()) && !BUFFER_CONDITION(k)){
            prevMessage = message;
            counter++;
            return;
        }

        if(counter!=0){
            userActLog.trace(QString("pressed %1 times").arg(counter+1));
            counter = 0;
        }

        prevMessage = message;


        if(BUFFER_CONDITION(k)){
            buffer.append(k->text());
            return;
        }

        if(!buffer.isEmpty()){
            userActLog.trace(QString("Typed string. Length=%1").arg(buffer.length()));
            buffer = "";
        }

        userActLog.trace(message);
    }
}

void UserActionsWriter::appendDialogInfo(){
    QDialog* dialog = qobject_cast<QDialog*>(QApplication::activeModalWidget());
    if(dialog){
        message.append(QString("DIALOG: \"%1\" ").arg(dialog->windowTitle()));
        QMessageBox* messageBox = qobject_cast<QMessageBox*>(dialog);
        if(messageBox){
            message.append("MESSAGEBOX_TEXT: ").append(messageBox->text()).append(" ");
        }
    }
}

void UserActionsWriter::appendModifiersInfo(){
    CHECK_EXT(k,userActLog.error(QString("key event is NULL %1:%2").arg(__FILE__).arg(__LINE__)),);

    Qt::KeyboardModifiers m = k->modifiers();
    if(m.testFlag(Qt::ShiftModifier) && k->key()!= Qt::Key_Shift)
        message.append(QString("shift + "));
    if(m.testFlag(Qt::AltModifier)&& k->key()!= Qt::Key_Alt)
        message.append(QString("alt + "));
    if(m.testFlag(Qt::ControlModifier) && k->key()!= Qt::Key_Control)
        message.append(QString("ctrl + "));
    if(m.testFlag(Qt::MetaModifier) && k->key()!= Qt::Key_Meta)
        message.append(QString("meta + "));
    if(m.testFlag(Qt::GroupSwitchModifier) && k->key()!= Qt::Key_Mode_switch)
        message.append(QString("switch + "));
}

UserActionsWriter::UserActionsWriter(){
    windowSize = QPoint(0,0);
    counter = 0;
    buffer = "";

    typeMap.insert(QEvent::MouseButtonPress, "mouse_press");
    typeMap.insert(QEvent::MouseButtonRelease, "mouse_release");
    typeMap.insert(QEvent::MouseButtonDblClick, "mouse_double_click");
    typeMap.insert(QEvent::KeyPress,"press");
    typeMap.insert(QEvent::KeyRelease,"release");

    modMap.insert(Qt::ShiftModifier,"Shift");
    modMap.insert(Qt::ControlModifier,"Ctrl");
    modMap.insert(Qt::AltModifier,"Alt");

    keys.insert(Qt::Key_Return,"enter");
    keys.insert(Qt::Key_Escape,"esc");
    keys.insert(Qt::Key_Tab,"tab");
    keys.insert(Qt::Key_Backtab,"back_tab");
    keys.insert(Qt::Key_Backspace,"backspace");
    keys.insert(Qt::Key_Insert,"insert");
    keys.insert(Qt::Key_Delete,"delete");
    keys.insert(Qt::Key_Enter,"keypad_enter");
    keys.insert(Qt::Key_Home,"home");
    keys.insert(Qt::Key_End,"end");
    keys.insert(Qt::Key_Left,"left_arrow");
    keys.insert(Qt::Key_Up,"up_arrow");
    keys.insert(Qt::Key_Right,"right_arrow");
    keys.insert(Qt::Key_Down,"down_arrow");
    keys.insert(Qt::Key_PageUp,"page_up");
    keys.insert(Qt::Key_PageDown,"page_down");
    keys.insert(Qt::Key_Shift,"shift");
    keys.insert(Qt::Key_Control,"ctrl");
    keys.insert(Qt::Key_Alt,"alt");
    keys.insert(Qt::Key_CapsLock,"caps_lock");
    keys.insert(Qt::Key_NumLock,"num_lock");
    keys.insert(Qt::Key_F1,"F1");
    keys.insert(Qt::Key_F2,"F2");
    keys.insert(Qt::Key_F3,"F3");
    keys.insert(Qt::Key_F4,"F4");
    keys.insert(Qt::Key_F5,"F5");
    keys.insert(Qt::Key_F6,"F6");
    keys.insert(Qt::Key_F7,"F7");
    keys.insert(Qt::Key_F8,"F8");
    keys.insert(Qt::Key_F9,"F9");
    keys.insert(Qt::Key_F10,"F10");
    keys.insert(Qt::Key_F11,"F11");
    keys.insert(Qt::Key_F12,"F12");
    keys.insert(Qt::Key_Space,"space");
    keys.insert(Qt::Key_0,"0");
    keys.insert(Qt::Key_1,"1");
    keys.insert(Qt::Key_2,"2");
    keys.insert(Qt::Key_3,"3");
    keys.insert(Qt::Key_4,"4");
    keys.insert(Qt::Key_5,"5");
    keys.insert(Qt::Key_6,"6");
    keys.insert(Qt::Key_7,"7");
    keys.insert(Qt::Key_8,"8");
    keys.insert(Qt::Key_9,"9");
    keys.insert(Qt::Key_A,"a");
    keys.insert(Qt::Key_B,"b");
    keys.insert(Qt::Key_C,"c");
    keys.insert(Qt::Key_D,"d");
    keys.insert(Qt::Key_E,"e");
    keys.insert(Qt::Key_F,"f");
    keys.insert(Qt::Key_G,"g");
    keys.insert(Qt::Key_H,"h");
    keys.insert(Qt::Key_I,"i");
    keys.insert(Qt::Key_J,"j");
    keys.insert(Qt::Key_K,"k");
    keys.insert(Qt::Key_L,"l");
    keys.insert(Qt::Key_M,"m");
    keys.insert(Qt::Key_N,"n");
    keys.insert(Qt::Key_O,"o");
    keys.insert(Qt::Key_P,"p");
    keys.insert(Qt::Key_Q,"q");
    keys.insert(Qt::Key_R,"r");
    keys.insert(Qt::Key_S,"s");
    keys.insert(Qt::Key_T,"t");
    keys.insert(Qt::Key_U,"u");
    keys.insert(Qt::Key_V,"v");
    keys.insert(Qt::Key_W,"w");
    keys.insert(Qt::Key_X,"x");
    keys.insert(Qt::Key_Y,"y");
    keys.insert(Qt::Key_Z,"z");
}
}
