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

#include "MessageBoxFiller.h"
#include "api/GTWidget.h"
#include "api/GTKeyboardDriver.h"

#include <QtGui/QApplication>
#include <QtGui/QAbstractButton>

namespace U2 {

#define GT_CLASS_NAME "GTUtilsDialog::MessageBoxDialogFiller"
#define GT_METHOD_NAME "run"
void MessageBoxDialogFiller::run() {

    QWidget* activeModal = QApplication::activeModalWidget();
    QMessageBox *messageBox = qobject_cast<QMessageBox*>(activeModal);
    GT_CHECK(messageBox != NULL, "messageBox is NULL");

    if(message!=""){
        QString actualText = messageBox->text();
        GT_CHECK(messageBox->text().contains(message,Qt::CaseInsensitive),
                 QString("Expected: %1, found: %2").arg(message).arg(actualText));
    }

    if(buttonText!=""){
        QList<QAbstractButton*> list = messageBox->buttons();
        foreach(QAbstractButton* but, list){
            QString s = but->text();
            if(but->text().contains(buttonText,Qt::CaseInsensitive)){
                GTWidget::click(os, but);
                return;
            }
        }
    }

    QAbstractButton* button = messageBox->button(b);
    GT_CHECK(button != NULL, "There is no such button in messagebox");

    GTWidget::click(os, button);
}
#undef GT_METHOD_NAME
#undef GT_CLASS_NAME


#define GT_CLASS_NAME "GTUtilsDialog::MessageBoxNoToAllOrNo"
#define GT_METHOD_NAME "run"
void MessageBoxNoToAllOrNo::run() {

    QWidget* activeModal = QApplication::activeModalWidget();
    QMessageBox *messageBox = qobject_cast<QMessageBox*>(activeModal);
    GT_CHECK(messageBox != NULL, "messageBox is NULL");

    QAbstractButton* button = messageBox->button(QMessageBox::NoToAll);
    if (!button) {
        button = messageBox->button(QMessageBox::No);
    }
    GT_CHECK(button != NULL, "There are no No buttons in messagebox");

    GTWidget::click(os, button);
}
#undef GT_METHOD_NAME
#undef GT_CLASS_NAME

#define GT_CLASS_NAME "MessageBoxOpenAnotherProject"
#define GT_METHOD_NAME "run"
void MessageBoxOpenAnotherProject::run(){
    
    QWidget* activeModal = QApplication::activeModalWidget();
    QMessageBox *messageBox = qobject_cast<QMessageBox*>(activeModal);
    GT_CHECK(messageBox != NULL, "messageBox is NULL");
    
    QAbstractButton* button = messageBox->findChild<QAbstractButton*>("New Window");
    GT_CHECK(button != NULL, "There are no New Window buttons in messagebox");

    button = messageBox->findChild<QAbstractButton*>("This Window");
    GT_CHECK(button != NULL, "There are no This Window buttons in messagebox");    

    button = messageBox->button(QMessageBox::Abort);
    GT_CHECK(button != NULL, "There are no Abort buttons in messagebox");

    GTWidget::click(os, button);
}
#undef GT_METHOD_NAME
#undef GT_CLASS_NAME

}
