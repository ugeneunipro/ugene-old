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
#include <QtGui/QMainWindow>
#include <QtGui/QTextBrowser>
#else
#include <QtWidgets/QApplication>
#include <QtWidgets/QMainWindow>
#include <QtWidgets/QTextBrowser>
#endif

#include <U2Core/AppContext.h>
#include <U2Gui/MainWindow.h>
#include <U2Gui/Notification.h>

#include "api/GTWidget.h"

#include "GTUtilsNotifications.h"

namespace U2{
#define GT_CLASS_NAME "NotificationChecker"

NotificationChecker::NotificationChecker(U2OpStatus &_os):os(_os){
    t = new QTimer(this);
    t->connect(t, SIGNAL(timeout()), this, SLOT(sl_checkNotification()));
    t->start(100);
}

#define GT_METHOD_NAME "sl_checkNotification"
void NotificationChecker::sl_checkNotification(){

QList<QWidget*> list = QApplication::allWidgets();
    foreach(QWidget* wid, list){
        Notification* notif = dynamic_cast<Notification*>(wid);
        if(notif!=NULL && notif->isVisible()){
            uiLog.trace("found");
            GTWidget::click(os, notif);
            t->stop();
        }
    }
}
#undef GT_METHOD_NAME
#undef GT_CLASS_NAME

#define GT_CLASS_NAME "NotificationDialogFiller"
#define GT_METHOD_NAME "run"
void NotificationDialogFiller::run(){
    QWidget *dialog = QApplication::activeModalWidget();
    GT_CHECK(dialog, "active modal widget is invalid");

    if(!message.isEmpty()){
        QTextBrowser* tb = dialog->findChild<QTextBrowser*>();
        GT_CHECK(tb != NULL, "text browser not found");
        QString actualMessage = tb->toPlainText();
        GT_CHECK(actualMessage.contains(message), "unexpected message: " + actualMessage);
    }
    QWidget* ok = GTWidget::findButtonByText(os, "Ok", dialog);
    GTWidget::click(os, ok);
}
#undef GT_METHOD_NAME
#undef GT_CLASS_NAME

#define GT_CLASS_NAME "NotificationChecker"
#define GT_METHOD_NAME "waitForNotification"
void GTUtilsNotifications::waitForNotification(U2OpStatus &os, bool dialogExpected){
    if(dialogExpected){
        GTUtilsDialog::waitForDialog(os, new NotificationDialogFiller(os));
    }
    new NotificationChecker(os);
}
#undef GT_METHOD_NAME
#undef GT_CLASS_NAME

}
