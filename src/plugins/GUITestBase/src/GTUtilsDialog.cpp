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

#include "GTUtilsDialog.h"

#include <QtCore/QTimer>
#include <QtGui/QApplication>

namespace U2 {

GUIDialogWaiter::GUIDialogWaiter(Runnable* _r, DialogType t, const QString& _objectName)
: hadRun(false), waiterId(-1), objectName(_objectName), runnable(_r), type(t), timer(NULL) {

    static int totalWaiterCount = 0;
    waiterId = totalWaiterCount++;

    timer = new QTimer();

    timer->connect(timer, SIGNAL(timeout()), this, SLOT(checkDialog()));
    timer->start(100);
}

GUIDialogWaiter::~GUIDialogWaiter() {

    finishWaiting();
}

void GUIDialogWaiter::finishWaiting() {

    delete timer; timer = NULL;
    delete runnable; runnable = NULL;
}

bool GUIDialogWaiter::isExpectedName(const QString& widgetObjectName, const QString& expectedObjectName) {

    if (expectedObjectName.isEmpty()) {
        return true;
    }
    uiLog.trace("Checking name, widget name <" + widgetObjectName + ">, expected <" + expectedObjectName + ">");
    return widgetObjectName == expectedObjectName;
}

void GUIDialogWaiter::checkDialog() {
    QWidget *widget = NULL;

    switch (type) {
        case Modal:
            widget = QApplication::activeModalWidget();
            break;

        case Popup:
            widget = QApplication::activePopupWidget();
            break;

        default:
            break;
    }
    if (!widget) {
        return;
    }

    QString widgetObjectName = widget->objectName();
    if (runnable && !hadRun && isExpectedName(widgetObjectName, objectName)) {
        timer->stop();
        uiLog.trace("-------------------------");
        uiLog.trace("GUIDialogWaiter::wait Id = " + QString::number(waiterId) + ", going to RUN");
        uiLog.trace("-------------------------");

        hadRun = true;
        runnable->run();
    }
}

#define GT_CLASS_NAME "GTUtilsDialog"

void GTUtilsDialog::waitForDialog(U2OpStatus &os, Runnable *r, GUIDialogWaiter::DialogType _type, const QString& _objectName)
{
    QString objectName = _objectName;
    Filler* f = dynamic_cast<Filler*>(r);
    if (f && !(f->getObjectName().isEmpty())) {
        objectName = f->getObjectName();
    }

    GUIDialogWaiter *waiter = new GUIDialogWaiter(r, _type, objectName);
//    pool.add
}

#undef GT_CLASS_NAME

}//namespace
