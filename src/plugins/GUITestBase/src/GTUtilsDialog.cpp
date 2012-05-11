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

void GUIDialogWaiter::wait() {
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

    if (r && !hadRun) {
        hadRun = true;
        r->run();
    }
}

#define GT_CLASS_NAME "GTUtilsDialog"

#define GT_METHOD_NAME "waitForDialog"
void GTUtilsDialog::waitForDialog(U2OpStatus &os, Runnable *r, GUIDialogWaiter::DialogType type, bool failOnNoDialog) {

    GUIDialogWaiter waiter(r, type);
    QTimer t;

    t.connect(&t, SIGNAL(timeout()), &waiter, SLOT(wait()));
    t.start(100);

    GTGlobals::sleep(1000);
    if (failOnNoDialog) {
        GT_CHECK(waiter.hadRun == true, "no dialog");
    }
}
#undef GT_METHOD_NAME

void GTUtilsDialog::preWaitForDialog(U2OpStatus &os, Runnable *r, GUIDialogWaiter::DialogType _type, int msec)
{
    GUIDialogWaiter *waiter = new GUIDialogWaiter(r, _type);
    QTimer *t = new QTimer;

    t->connect(t, SIGNAL(timeout()), waiter, SLOT(wait()));
    t->start(msec);
}

#undef GT_CLASS_NAME

}//namespace
