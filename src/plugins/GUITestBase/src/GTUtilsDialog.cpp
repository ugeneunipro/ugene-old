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

#include <QtCore/QTimer>

#if (QT_VERSION < 0x050000) //Qt 5
#include <QtGui/QApplication>
#include <QtGui/QPushButton>
#else
#include <QtWidgets/QApplication>
#include <QtWidgets/QPushButton>
#endif

#include "GTUtilsDialog.h"
#include "api/GTMouseDriver.h"
#include "api/GTWidget.h"

namespace U2 {

#define GT_CLASS_NAME "GUIDialogWaiter"

GUIDialogWaiter::GUIDialogWaiter(U2OpStatus &_os, Runnable* _r, const WaitSettings& _settings)
: hadRun(false), waiterId(-1), os(_os), runnable(_r), settings(_settings), timer(NULL), waitingTime(0) {

    static int totalWaiterCount = 0;
    waiterId = totalWaiterCount++;

    timer = new QTimer();

    timer->connect(timer, SIGNAL(timeout()), this, SLOT(checkDialog()));
    timer->start(timerPeriod);
}

GUIDialogWaiter::~GUIDialogWaiter() {

    finishWaiting();
}

void GUIDialogWaiter::finishWaiting() {

    delete timer; timer = NULL;
    delete runnable; runnable = NULL;
}

bool GUIDialogWaiter::isExpectedName(const QString& widgetObjectName, const QString& expectedObjectName) {

    if (expectedObjectName.isNull()) {
        uiLog.trace("GUIDialogWaiter Warning!! Checking name, widget name <" + widgetObjectName + ">, but expected any, saying it's expected");
        return true;
    }

    uiLog.trace("GUIDialogWaiter Checking name, widget name <" + widgetObjectName + ">, expected <" + expectedObjectName + ">");
    return widgetObjectName == expectedObjectName;
}

void GUIDialogWaiter::checkDialogPool(){
    foreach(GUIDialogWaiter* waiter, GTUtilsDialog::pool){
        waiter->checkDialog();
    }
}

#define GT_METHOD_NAME "checkDialog"
void GUIDialogWaiter::checkDialog() {
    QWidget *widget = NULL;
    GT_CHECK(runnable != NULL, "Runnable is NULL");

    switch (settings.dialogType) {
        case Modal:
            widget = QApplication::activeModalWidget();
            break;

        case Popup:
            widget = QApplication::activePopupWidget();
            break;

        default:
            break;
    }

    if (widget && !hadRun && isExpectedName(widget->objectName(), settings.objectName)) {
        timer->stop();
        uiLog.trace("-------------------------");
        uiLog.trace("GUIDialogWaiter::wait Id = " + QString::number(waiterId) + ", going to RUN");
        uiLog.trace("-------------------------");

        hadRun = true;
        try {
            runnable->run();
        } catch(U2OpStatus *opStatus) {
            Q_UNUSED(opStatus);
            QWidget* w = QApplication::activeModalWidget();
            while (w != NULL){
                w->close();
                w = QApplication::activeModalWidget();
            }
            w = QApplication::activePopupWidget();
            while (w != NULL){
                w->close();
                w = QApplication::activePopupWidget();
            }
        }
    }
    else {
        waitingTime += timerPeriod;
        if (waitingTime > settings.timeout) {
            uiLog.trace("-------------------------");
            uiLog.trace("!!! GUIDialogWaiter::TIMEOUT Id = " + QString::number(waiterId) + ", going to finish waiting");
            uiLog.trace("-------------------------");

            finishWaiting();
            GT_CHECK(false, "TIMEOUT, waiterId = " + QString::number(waiterId));
        }
    }
}
#undef GT_METHOD_NAME

#undef GT_CLASS_NAME


#define GT_CLASS_NAME "GTUtilsDialog"

QList<GUIDialogWaiter*> GTUtilsDialog::pool = QList<GUIDialogWaiter*>();

#define GT_METHOD_NAME "clickButtonBox"
void GTUtilsDialog::clickButtonBox(U2OpStatus& os, QWidget* dialog, QDialogButtonBox::StandardButton button) {
    QDialogButtonBox* box = qobject_cast<QDialogButtonBox*>(GTWidget::findWidget(os, "buttonBox", dialog));
    GT_CHECK(box != NULL, "buttonBox is NULL");
    QPushButton* pushButton = box->button(button);
    GT_CHECK(pushButton != NULL, "pushButton is NULL");
    GTWidget::click(os, pushButton);
}
#undef GT_METHOD_NAME

void GTUtilsDialog::waitForDialog(U2OpStatus &os, Runnable *r, const GUIDialogWaiter::WaitSettings& settings)
{
    GUIDialogWaiter *waiter = new GUIDialogWaiter(os, r, settings);
    pool.prepend(waiter);
}

void GTUtilsDialog::waitForDialog(U2OpStatus &os, Runnable *r) {
    GUIDialogWaiter::WaitSettings settings;
    Filler* f = dynamic_cast<Filler*>(r);
    if (f) {
        settings = f->getSettings();
    }

    waitForDialog(os, r, settings);
}

void GTUtilsDialog::waitForDialogWhichMustNotBeRunned(U2OpStatus &os, Runnable *r) {
    GUIDialogWaiter::WaitSettings settings;
    Filler* f = dynamic_cast<Filler*>(r);
    if (f) {
        settings = f->getSettings();
    }

    settings.destiny = GUIDialogWaiter::MustNotBeRun;
    waitForDialog(os, r, settings);
}

void GTUtilsDialog::waitForDialogWhichMayRunOrNot(U2OpStatus &os, Runnable *r) {
    GUIDialogWaiter::WaitSettings settings;
    Filler* f = dynamic_cast<Filler*>(r);
    if (f) {
        settings = f->getSettings();
    }

    settings.destiny = GUIDialogWaiter::NoMatter;
    waitForDialog(os, r, settings);
}

#define GT_METHOD_NAME "checkAllFinished"
void GTUtilsDialog::checkAllFinished(U2OpStatus &os) {
    Q_UNUSED(os);

    foreach(GUIDialogWaiter* w, pool) {
        GT_CHECK(w, "NULL GUIDialogWaiter");
        switch  (w->getSettings().destiny){
        case GUIDialogWaiter::MustBeRun:
            GT_CHECK(w->hadRun, QString("\"%1\" not run but should be").arg((w->getSettings().objectName)));
            break;
        case GUIDialogWaiter::MustNotBeRun:
            GT_CHECK(!w->hadRun, QString("\"%1\" had run but should not").arg((w->getSettings().objectName)));
            break;
        case GUIDialogWaiter::NoMatter:
            break;
        }
    }
}
#undef GT_METHOD_NAME

void GTUtilsDialog::cleanup(U2OpStatus &os, CleanupSettings s) {

    if (s == FailOnUnfinished) {
        checkAllFinished(os);
    }

    qDeleteAll(pool);
    pool.clear();
}

#undef GT_CLASS_NAME

} //namespace
