/**
 * UGENE - Integrated Bioinformatics Tools.
 * Copyright (C) 2008-2012 UniPro <ugene@unipro.ru>
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

#include "QtUtils.h"
#include <U2Test/GUITestBase.h>
#include <U2Core/Task.h>

#include <U2Core/U2SafePoints.h>
#include <U2Gui/GUIUtils.h>
#include <QtTest/QSpontaneKeyEvent>
#include "api/GTKeyboardDriver.h"

namespace U2 {

#define WAIT_TIMEOUT    2000

QWidget* QtUtils::findWidgetByName(U2OpStatus &os, const QString &widgetName, QWidget *parentWidget, bool errorIfNull) {
    checkThread();
    QWidget *widget = NULL;
    if (parentWidget == NULL) {
        parentWidget = AppContext::getMainWindow()->getQMainWindow();
    } 
    widget = parentWidget->findChild<QWidget*>(widgetName);

    if (errorIfNull) {
        CHECK_SET_ERR_RESULT(widget != NULL, "Widget " + widgetName + " not found", NULL);
    }

    return widget;
}

bool QtUtils::isWidgetExists(const QString &widgetName) {
    checkThread();
    QMainWindow *mw = AppContext::getMainWindow()->getQMainWindow();
    if (!mw) {
        return false;
    }

    QWidget *w = mw->findChild<QWidget*>(widgetName);
    return (w != NULL && w->isVisible());
}

QWidget* QtUtils::findWidgetByTitle(U2OpStatus &os, const QString &title) {
    checkThread();
    QMainWindow *mw = AppContext::getMainWindow()->getQMainWindow();
    QList<QWidget *>wList = mw->findChildren<QWidget*>();
    foreach(QWidget *w, wList) {
        QString t = w->windowTitle();
        if(w->windowTitle() == title) {
            return w;
        }
    }

    QString errString = QString("Widget %1 not found").arg(title);
    CHECK_SET_ERR_RESULT(false, errString, NULL);
}


void QtUtils::moveTo(U2OpStatus &os, const QString &widgetName, const QPoint &_pos) {
    QWidget * w = findWidgetByName(os, widgetName);
    assert(w != NULL);
    if(!(w && w->isVisible())) {
        return;
    }
    QPoint pos = _pos;
    if(pos.isNull()) {
        pos = w->rect().center();
    }

    QPoint begin = QCursor::pos();
    QPoint end = w->mapToGlobal(pos);
    QCursor::setPos(end);
    return;
    float k = (float)(end.ry() - begin.ry())/(end.rx() - begin.rx());
    float b = begin.ry() - k * begin.rx();


    int x1 = begin.rx();
    int x2 = end.rx();
    
    if(x1 < x2) {
        for(int i = x1; i <= x2; i++) {
            int y = k*i + b;
            sleep(10);
            QCursor::setPos(i, y);
        }
    } else {
        for(int i = x1; i >= x2; i--) {
            int y = k*i + b;
            sleep(10);
            QCursor::setPos(i, y);
        }
    }
}

void QtUtils::mousePress(U2OpStatus &os, const QString &widgetName, Qt::MouseButton button, const QPoint &_pos) {
    QWidget * w = findWidgetByName(os, widgetName);
    QPoint pos = _pos;
    if(pos.isNull()) {
        pos = w->rect().center();
    }

    QMouseEvent *me = new QMouseEvent(QEvent::MouseButtonPress, pos, button, button, 0); 
    sendEvent(w, me);
}

void QtUtils::mouseRelease(U2OpStatus &os, const QString &widgetName, Qt::MouseButton button, const QPoint &_pos) {
    QWidget * w = findWidgetByName(os, widgetName);
    QPoint pos = _pos;
    if(pos.isNull()) {
        pos = w->rect().center();
    }

    QMouseEvent *me = new QMouseEvent(QEvent::MouseButtonRelease, pos, button, button, 0); 
    sendEvent(w, me);
}

void QtUtils::mouseClick(U2OpStatus &os, const QString &widgetName, Qt::MouseButton button, const QPoint &_pos) {
    mousePress(os, widgetName, button, _pos);
    QtUtils::sleep(500);
    mouseRelease(os, widgetName, button, _pos);
}

void QtUtils::mouseDbClick(U2OpStatus &os, const QString &widgetName, const QPoint &_pos) {
    QWidget * w = findWidgetByName(os, widgetName);
    QPoint pos = _pos;
    if(pos.isNull()) {
        pos = w->rect().center();
    }

    QMouseEvent *me = new QMouseEvent(QEvent::MouseButtonDblClick, pos, Qt::LeftButton, Qt::LeftButton, 0);
    sendEvent(w, me);
}

void QtUtils::mousePress(QWidget *w, Qt::MouseButton button, const QPoint &_pos) {
    if (!w) {
        return;
    }
    QPoint pos = _pos;
    if(pos.isNull()) {
        pos = w->rect().center();
    }

    QMouseEvent *me = new QMouseEvent(QEvent::MouseButtonPress, pos, button, button, 0); 
    sendEvent(w, me);
}

void QtUtils::mouseRelease(QWidget *w, Qt::MouseButton button, const QPoint &_pos) {
    if (!w) {
        return;
    }
    QPoint pos = _pos;
    if(pos.isNull()) {
        pos = w->rect().center();
    }

    QMouseEvent *me = new QMouseEvent(QEvent::MouseButtonRelease, pos, button, button, 0); 
    sendEvent(w, me);
}

void QtUtils::mouseClick(QWidget *w, Qt::MouseButton button, const QPoint &_pos) {
    mousePress(w, button, _pos);
    mouseRelease(w, button, _pos);
}

void QtUtils::mouseDbClick(QWidget *w, const QPoint &_pos) {
    if (!w) {
        return;
    }
    QPoint pos = _pos;
    if(pos.isNull()) {
        pos = w->rect().center();
    }

    QMouseEvent *me = new QMouseEvent(QEvent::MouseButtonDblClick, pos, Qt::LeftButton, Qt::LeftButton, 0);
    sendEvent(w, me);
}

void QtUtils::expandTopLevelMenu(U2OpStatus &os, const QString &menuName, const QString &parentMenu) {
    QMainWindow *mw = AppContext::getMainWindow()->getQMainWindow();
    QAction *curAction = mw->findChild<QAction*>(menuName);
    CHECK_SET_ERR(curAction != NULL, QString("Can't find action %1").arg(menuName));

    QMenuBar *parMenu = static_cast<QMenuBar*>(findWidgetByName(os, parentMenu));
    CHECK_SET_ERR(parMenu != NULL, QString("Menu %1 not found").arg(parentMenu));
    if(!parMenu->isVisible()) {
        return;
    }

    QPoint pos = parMenu->actionGeometry(curAction).center();

    moveTo(os, parentMenu, pos);
    QtUtils::sleep(500);
    mouseClick(os, parentMenu, Qt::LeftButton, pos);
}

void QtUtils::clickMenu(U2OpStatus &os, const QString &menuName, const QString &parentMenu) {

    QMainWindow *mw = AppContext::getMainWindow()->getQMainWindow();
    QAction *curAction = mw->findChild<QAction*>(menuName);
    CHECK_SET_ERR(curAction != NULL, QString("Can't find action %1").arg(menuName));

    QMenu* parMenu = (QMenu*)findWidgetByName(os, parentMenu);
    CHECK_SET_ERR(parMenu != NULL, QString("Menu %1 not found").arg(parentMenu));
    QPoint pos = parMenu->actionGeometry(curAction).center();

    moveTo(os, parentMenu, pos);
    mouseClick(os, parentMenu, Qt::LeftButton, pos);
}

void QtUtils::clickContextMenu(U2OpStatus &os, const QString &menuName) {

    QMenu* parMenu = getContextMenu();
    CHECK_SET_ERR(parMenu != NULL, "Context menu not found");

    QList<QAction*> actions = parMenu->actions();
    QAction *curAction = NULL;
    foreach(QAction *a, actions) {
        if(a->text() == menuName) {
            curAction = a;
            break;
        }
    }
    CHECK_SET_ERR(curAction != NULL, QString("Can't find action %1").arg(menuName));

    QPoint pos = parMenu->actionGeometry(curAction).center();
    QCursor::setPos(parMenu->mapToGlobal(pos));
    QMouseEvent *me = new QMouseEvent(QEvent::MouseButtonPress, pos, Qt::LeftButton, Qt::LeftButton, 0);
    sendEvent(parMenu, me);
    QMouseEvent *me1 = new QMouseEvent(QEvent::MouseButtonRelease, pos, Qt::LeftButton, Qt::LeftButton, 0);
    sendEvent(parMenu, me1);
}

void QtUtils::contextMenu(U2OpStatus &os, const QString &widgetName, const QPoint &_pos) {
    QWidget * w = findWidgetByName(os, widgetName);
    if (!w) {
        return;
    }

    QPoint pos = _pos;
    if(pos.isNull()) {
        pos = w->rect().center();
    }

    QContextMenuEvent *cme = new QContextMenuEvent(QContextMenuEvent::Mouse, pos);
    //mousePress(widgetName, Qt::RightButton, pos); //Select item in tree
    sendEvent(w, cme);
    while(getContextMenu() == NULL) {
        sleep(1);
    }
}

void QtUtils::clickMenuAction(U2OpStatus &os, const QString &actionName, const QString &menuName) {

    QAction* curAction = getMenuAction(os, actionName, menuName);
    CHECK_SET_ERR(curAction != NULL, QString("Can't find action %1").arg(actionName));

    QMenu* parMenu = (QMenu*)findWidgetByName(os, menuName);
    CHECK_SET_ERR(parMenu != NULL, QString("Menu %1 not found").arg(menuName));
    QPoint pos = parMenu->actionGeometry(curAction).center();

    moveTo(os, menuName, pos);
    mouseClick(os, menuName, Qt::LeftButton, pos);
}

QAction* QtUtils::getMenuAction(U2OpStatus &os, const QString &actionName, const QString &menuName) {

    QtUtils::expandTopLevelMenu(os, menuName, MWMENU);
    QtUtils::sleep(500);

    MainWindow* mw = AppContext::getMainWindow();
    QMenu* menu = mw->getTopLevelMenu(menuName);
    CHECK_SET_ERR_RESULT(menu != NULL, "No such menu: " + menuName, false);

    QAction* neededAction = GUIUtils::findAction(menu->actions(), actionName);
    QtUtils::expandTopLevelMenu(os, menuName, MWMENU);

    return neededAction;
}

QMenu *QtUtils::getContextMenu() {
    return static_cast<QMenu*>(QApplication::activePopupWidget());
}

QDialog* QtUtils::getActiveDialog() {
    return static_cast<QDialog*>(QApplication::activeModalWidget());
}

QWidget* QtUtils::getWidgetInFocus() {
    return QApplication::focusWidget();
}

void QtUtils::sendEvent(QObject *obj, QEvent *e) {
    QSpontaneKeyEvent::setSpontaneous(e);
    qApp->notify(obj, e);
}

void QtUtils::sleep( int msec ){
    QEventLoop l;
    QTimer::singleShot(msec, &l, SLOT(quit()));
    l.exec();
}


void QtUtils::mousePressOnItem(U2OpStatus &os, const QString &widgetName, Qt::MouseButton button, const QPoint &_pos) {
    QTreeView * w = static_cast<QTreeView *>(findWidgetByName(os, widgetName));
    QPoint pos = _pos;
    if(pos.isNull()) {
        pos = w->rect().center();
    }

    QMouseEvent *me = new QMouseEvent(QEvent::MouseButtonPress, pos, button, button, 0); 
    sendEvent(w->viewport(), me);
}

void QtUtils::mouseReleaseOnItem(U2OpStatus &os, const QString &widgetName, Qt::MouseButton button, const QPoint &_pos) {
    QTreeView * w = static_cast<QTreeView *>(findWidgetByName(os, widgetName));
    QPoint pos = _pos;
    if(pos.isNull()) {
        pos = w->rect().center();
    }

    QMouseEvent *me = new QMouseEvent(QEvent::MouseButtonRelease, pos, button, button, 0); 
    sendEvent(w->viewport(), me);
}

void QtUtils::mouseClickOnItem(U2OpStatus &os, const QString &widgetName, Qt::MouseButton button, const QPoint &_pos) {
    mousePressOnItem(os, widgetName, button, _pos);
    mouseReleaseOnItem(os, widgetName, button, _pos);
}

void QtUtils::mouseDbClickOnItem(U2OpStatus &os, const QString &widgetName, const QPoint &_pos) {
    QTreeView * w = static_cast<QTreeView *>(findWidgetByName(os, widgetName));
    QPoint pos = _pos;
    if(pos.isNull()) {
        pos = w->rect().center();
    }

    QMouseEvent *me = new QMouseEvent(QEvent::MouseButtonDblClick, pos, Qt::LeftButton, Qt::LeftButton, 0);
    sendEvent(w->viewport(), me);
}

void QtUtils::contextMenuOnItem(U2OpStatus &os, const QString &widgetName, const QPoint &_pos) {
    QTreeView * w = static_cast<QTreeView *>(findWidgetByName(os, widgetName));
    QPoint pos = _pos;
    if(pos.isNull()) {
        pos = w->rect().center();
    }

    QContextMenuEvent *cme = new QContextMenuEvent(QContextMenuEvent::Mouse, pos);
    //mousePressOnItem(widgetName, Qt::LeftButton, pos); //Select item in tree
    sendEvent(w->viewport(), cme);
    while(getContextMenu() == NULL) {
        sleep(1);
    }
}

QPoint QtUtils::getItemPosition(U2OpStatus &os, const QString &itemName, const QString &treeName) {
    QTreeWidget *tree = static_cast<QTreeWidget*>(findWidgetByName(os, treeName));

    QList<QTreeWidgetItem*> listItems = tree->findItems(itemName, Qt::MatchExactly | Qt::MatchRecursive);
    CHECK_SET_ERR_RESULT(listItems.isEmpty() == false, QString("Item %1 not found").arg(itemName), QPoint());

    QTreeWidgetItem *item = tree->findItems(itemName, Qt::MatchExactly | Qt::MatchRecursive).first();
    if(!item) {
        return QPoint();
    }

    return tree->visualItemRect(item).center();
}

bool QtUtils::isItemExists(U2OpStatus &os, const QString &itemName, const QString &treeName) {
    QTreeWidget *tree = static_cast<QTreeWidget*>(findWidgetByName(os, treeName));
    if(tree->findItems(itemName, Qt::MatchExactly | Qt::MatchRecursive).isEmpty()) {
        return false;
    }
    return true;
}

void QtUtils::expandTreeItem(U2OpStatus &os, const QString &itemName, const QString &treeName){
    QTreeWidget *tree = static_cast<QTreeWidget*>(findWidgetByName(os, treeName));

    QList<QTreeWidgetItem*> listItems = tree->findItems(itemName, Qt::MatchExactly | Qt::MatchRecursive);
    CHECK_SET_ERR(listItems.isEmpty() == false, QString("Item %1 not found").arg(itemName));

    QTreeWidgetItem *item = tree->findItems(itemName, Qt::MatchExactly | Qt::MatchRecursive).first();
    QPoint pos = tree->visualItemRect(item).topLeft();
    moveTo(os, treeName, pos);
    mouseClickOnItem(os, treeName, Qt::LeftButton, pos);
    GTKeyboardDriver::keyClick(os, GTKeyboardDriver::key["right"]);
}

bool QtUtils::waitForWidget(U2OpStatus &os, const QString& widgetName, bool show ){
    int count = 0;
    bool visible = false;
    do {
        if(isWidgetExists(widgetName)) {
            visible = findWidgetByName(os, widgetName)->isVisible();
        } else {
            visible = false;
        }
        sleep(1);
        count++;
    } while (visible != show && count < WAIT_TIMEOUT);
    return count < WAIT_TIMEOUT;
}

bool QtUtils::waitForTreeItem(U2OpStatus &os, const QString& itemName, const QString& treeName, bool show ){
    int count = 0;
    do {
        sleep(1);
        count++;
    } while(isItemExists(os, itemName, treeName) != show && count < WAIT_TIMEOUT);
    return count < WAIT_TIMEOUT;
}

bool QtUtils::waitForMenuWithAction(const QString &actionName) {
    int count = 0;
    bool fl = false;
    while(!fl && count < WAIT_TIMEOUT){
        QMenu* menu = getContextMenu();
        if(menu) {
            QList<QAction*> actions = menu->actions();
            foreach(QAction *a, actions) {
                 coreLog.info(a->text());
                if(a->text() == actionName) {
                    fl = true;
                    break;
                }
            }
        }
        sleep(1);
        count++;
    } 
    return count < WAIT_TIMEOUT;
}

bool QtUtils::waitForTask( Task *t ){
    int count = 0;
    while(!t->isRunning() && count < WAIT_TIMEOUT) {
        //count++;
        sleep(1);
    }
    return count < WAIT_TIMEOUT;
}

void QtUtils::checkThread() {
    QThread* appThread = QApplication::instance()->thread();
    QThread* thisThread = QThread::currentThread();
    assert (appThread == thisThread);
}

} // U2
