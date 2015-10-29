/**
 * UGENE - Integrated Bioinformatics Tools.
 * Copyright (C) 2008-2015 UniPro <ugene@unipro.ru>
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

#include <QApplication>
#include <QMainWindow>
#include <QMdiSubWindow>
#include <QMenu>

#include <U2Core/AppContext.h>

#include <U2Gui/MainWindow.h>

#include "GTUtilsMdi.h"
#include "GTGlobals.h"
#include "primitives/GTMenu.h"
#include "drivers/GTMouseDriver.h"
#include "utils/GTThread.h"
#include "base_dialogs/MessageBoxFiller.h"

namespace U2 {
using namespace HI;

#define GT_CLASS_NAME "GTUtilsMdi"

#define GT_METHOD_NAME "click"
void GTUtilsMdi::click(U2OpStatus &os, GTGlobals::WindowAction action) {

    MainWindow* mw = AppContext::getMainWindow();
    GT_CHECK(mw != NULL, "MainWindow == NULL");

    QMainWindow* mainWindow = mw->getQMainWindow();
    GT_CHECK(mainWindow != NULL, "QMainWindow == NULL");

    // TODO: batch tests run fails because of not maximized window by default from settings
//    if ((action == GTGlobals::Maximize) || (action == GTGlobals::Minimize)) {
//        return;
//    }

#ifndef Q_OS_MAC
    GTMenuBar::clickCornerMenu(os, mainWindow->menuBar(), action);
#else
    MWMDIWindow *mdiWindow = mw->getMDIManager()->getActiveWindow();
    GT_CHECK(mdiWindow != NULL, "MDIWindow == NULL");

    // TODO: make click on button
    switch (action) {
    case GTGlobals::Maximize:
        GTWidget::showMaximized(os, mdiWindow);
        break;
    case GTGlobals::Close: {
        int left = mdiWindow->rect().left();
        int top = mdiWindow->rect().top();
        QPoint p(left + 15,top - 10);
        GTMouseDriver::moveTo(os, mdiWindow->mapToGlobal(p));
        GTMouseDriver::click(os);
        break;
    }
    default:
        assert(false);
        break;
    }
#endif
}
#undef GT_METHOD_NAME

#define GT_METHOD_NAME "findWindow"
QWidget * GTUtilsMdi::findWindow(U2OpStatus &os, const QString &windowName, const GTGlobals::FindOptions &options) {
    Q_UNUSED(os);
    GT_CHECK_RESULT(windowName.isEmpty() == false, "windowname is empty", NULL);

    MainWindow *mw = AppContext::getMainWindow();
    GT_CHECK_RESULT(mw != NULL, "MainWindow == NULL", NULL);

    QList<MWMDIWindow *> mdiWindows = mw->getMDIManager()->getWindows();
    foreach (MWMDIWindow *w, mdiWindows) {
        QString mdiTitle = w->windowTitle();
        switch (options.matchPolicy) {
        case Qt::MatchExactly:
            if (mdiTitle == windowName) {
                return w;
            }
            break;
        case Qt::MatchContains:
            if (mdiTitle.contains(windowName)) {
                return w;
            }
            break;
        default:
            GT_CHECK_RESULT(false, "Not implemented", NULL);
        }
    }

    if (options.failIfNull) {
        GT_CHECK_RESULT(false, "Widget " + windowName + " not found", NULL);
    }

    return NULL;
}
#undef GT_METHOD_NAME

#define GT_METHOD_NAME "closeWindow"
void GTUtilsMdi::closeWindow(U2OpStatus &os, const QString &windowName, const GTGlobals::FindOptions& options) {
    GT_CHECK(windowName.isEmpty() == false, "windowname is empty");

    MainWindow* mw = AppContext::getMainWindow();
    GT_CHECK(mw != NULL, "MainWindow == NULL");

    MWMDIWindow* window = qobject_cast<MWMDIWindow*>(findWindow(os, windowName, options));
    GT_CHECK(window != NULL, "Cannot find MDI window");
    GTWidget::close(os, window->parentWidget());
}
#undef GT_METHOD_NAME

#define GT_METHOD_NAME "closeAllWindows"
void GTUtilsMdi::closeAllWindows(U2OpStatus &os) {
    class Scenario : public CustomScenario {
    public:
        void run(U2OpStatus &os) {
            const QList<QMdiSubWindow *> mdiWindows = AppContext::getMainWindow()->getQMainWindow()->findChildren<QMdiSubWindow *>();
            foreach (QMdiSubWindow *mdiWindow, mdiWindows) {
                GTUtilsDialog::waitForDialogWhichMayRunOrNot(os, new MessageBoxDialogFiller(os, QMessageBox::Discard));
                mdiWindow->close();
                GTGlobals::sleep(100);
            }
        }
    };

    GTThread::runInMainThread(os, new Scenario);
}
#undef GT_METHOD_NAME

#define GT_METHOD_NAME "waitWindowOpened"
void GTUtilsMdi::waitWindowOpened(U2OpStatus &os, const QString &windowNamePart, qint64 timeout) {
    MainWindow *mainWindow = AppContext::getMainWindow();
    GT_CHECK(mainWindow != NULL, "MainWindow == NULL");
    MWMDIManager *mdiManager = mainWindow->getMDIManager();
    GT_CHECK(mdiManager != NULL, "MainWindow == NULL");

    bool found = false;
    int passedTime = 0;
    while (!found && passedTime < timeout / 1000) {
        foreach (MWMDIWindow *window, mdiManager->getWindows()) {
            found |= window->windowTitle().contains(windowNamePart, Qt::CaseInsensitive);
        }
        GTGlobals::sleep(1000);
        passedTime++;
    }

    if (!found) {
        os.setError(QString("Cannot find MDI window with part of name '%1', timeout").arg(windowNamePart));
    }
}
#undef GT_METHOD_NAME

#define GT_METHOD_NAME "activeWindow"
QWidget* GTUtilsMdi::activeWindow(U2OpStatus &os, const GTGlobals::FindOptions& options) {

    MainWindow* mw = AppContext::getMainWindow();
    GT_CHECK_RESULT(mw != NULL, "MainWindow == NULL", NULL);

    QWidget* w = mw->getMDIManager()->getActiveWindow();
    if (options.failIfNull) {
        GT_CHECK_RESULT(w != NULL, "Active window is not found", NULL);
    }
    return w;
}
#undef GT_METHOD_NAME

#define GT_METHOD_NAME "activeWindowTitle"
QString GTUtilsMdi::activeWindowTitle(U2OpStatus &os){
    QWidget* w = activeWindow(os);
    MWMDIWindow* mdi = qobject_cast<MWMDIWindow*>(w);
    GT_CHECK_RESULT(mdi, "unexpected object type", QString());
    return mdi->windowTitle();
}
#undef GT_METHOD_NAME

#define GT_METHOD_NAME "activateWindow"
void GTUtilsMdi::activateWindow(U2OpStatus &os, const QString &windowName){
    MainWindow* mw = AppContext::getMainWindow();
    GT_CHECK(mw != NULL, "MainWindow == NULL");

    CHECK(!activeWindowTitle(os).contains(windowName), );

    GTGlobals::FindOptions options;
    options.matchPolicy = Qt::MatchContains;
    MWMDIWindow* window = qobject_cast<MWMDIWindow*>(findWindow(os, windowName, options));
    GT_CHECK(window != NULL, "window " + windowName + " not found");

    GTMenu::clickMainMenuItem(os, QStringList() << "Window" << windowName, GTGlobals::UseMouse, Qt::MatchContains);
    GTGlobals::sleep(500);
}
#undef GT_METHOD_NAME

#define GT_METHOD_NAME "getMdiItemPosition"
QPoint GTUtilsMdi::getMdiItemPosition(U2OpStatus &os, const QString& windowName){
    QWidget* w = findWindow(os, windowName);
    GT_CHECK_RESULT(w != NULL, "MDI window not found", QPoint());
    const QRect r = w->rect();
    return w->mapToGlobal(r.center());
}
#undef GT_METHOD_NAME

#define GT_METHOD_NAME "selectRandomRegion"
void GTUtilsMdi::selectRandomRegion(U2OpStatus &os, const QString& windowName){
    QWidget* w = findWindow(os, windowName);
    GT_CHECK(w != NULL, "MDI window not found");
    const QRect r = w->rect();
    QPoint p = QPoint((r.topLeft().x() + r.bottomLeft().x())/2 + 5, r.center().y()/2);
    GTMouseDriver::moveTo(os, w->mapToGlobal(p));
    GTMouseDriver::press(os);
    GTMouseDriver::moveTo(os, w->mapToGlobal(r.center()));
    GTMouseDriver::release(os);
}
#undef GT_METHOD_NAME

namespace {

bool isWidgetPartVisible(QWidget *widget) {
    CHECK(NULL != widget, false);

    if (!widget->visibleRegion().isEmpty()) {
        return true;
    }

    foreach (QObject *child, widget->children()) {
        if (child->isWidgetType() && isWidgetPartVisible(qobject_cast<QWidget *>(child))) {
            return true;
        }
    }

    return false;
}

}

#define GT_METHOD_NAME "isAnyPartOfWindowVisible"
bool GTUtilsMdi::isAnyPartOfWindowVisible(U2OpStatus &os, const QString &windowName) {
    GTGlobals::FindOptions options;
    options.failIfNull = false;
    QWidget *window = findWindow(os, windowName, options);
    CHECK(NULL != window, false);
    return isWidgetPartVisible(window);
}
#undef GT_METHOD_NAME

#undef GT_CLASS_NAME

}
