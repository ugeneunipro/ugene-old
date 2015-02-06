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

#include <QtCore/QDir>

#if (QT_VERSION < 0x050000) //Qt 5
#include <QtGui/QAbstractButton>
#include <QtGui/QApplication>
#include <QtGui/QDialogButtonBox>
#include <QtGui/QPushButton>
#include <QtGui/QRadioButton>
#else
#include <QtWidgets/QAbstractButton>
#include <QtWidgets/QApplication>
#include <QtWidgets/QDialogButtonBox>
#include <QtWidgets/QPushButton>
#include <QtWidgets/QRadioButton>
#endif

#include "CreateAnnotationWidgetFiller.h"
#include "api/GTComboBox.h"
#include "api/GTKeyboardDriver.h"
#include "api/GTLineEdit.h"
#include "api/GTRadioButton.h"
#include "api/GTWidget.h"

namespace U2 {

#define GT_CLASS_NAME "GTUtilsDialog::CreateAnnotationDialogChecker"
#define GT_METHOD_NAME "run"
void CreateAnnotationWidgetChecker::run() {

    QWidget* dialog = QApplication::activeModalWidget();
    GT_CHECK(dialog, "activeModalWidget is NULL");

    //QAbstractButton *newTableRButton = qobject_cast<QAbstractButton*>(GTWidget::findWidget(os, "rbCreateNewTable", dialog));
    //GT_CHECK(newTableRButton != NULL, "Radiobutton is NULL");
    //GT_CHECK((newTableRButton->isEnabled()==newTableRB) && (newTableRButton->isChecked()==newTableRB), "newFileRB is not enabled and checked");

    //QAbstractButton *radioButton = qobject_cast<QAbstractButton*>(GTWidget::findWidget(os, "rbExistingTable", dialog));
    //GT_CHECK(radioButton != NULL, "Radiobutton is NULL");
    //GT_CHECK((radioButton->isEnabled() != newTableRB) && (radioButton->isChecked() != newTableRB), "existingObjectRB is enabled or checked");

    QAbstractButton *radioButton = qobject_cast<QAbstractButton*>(GTWidget::findWidget(os, "rbExistingTable", dialog));
    GT_CHECK(radioButton != NULL, "Radiobutton is NULL");
    GT_CHECK(!radioButton->isEnabled() && !radioButton->isCheckable(), "radioButton is enabled or checkable");

    QComboBox *comboBox = qobject_cast<QComboBox*>(GTWidget::findWidget(os, "cbExistingTable", dialog));
    GT_CHECK(comboBox != NULL, "ComboBox is NULL");
    GT_CHECK(comboBox->count() == 0, "comboBox count is not 0");

    QAbstractButton *b = qobject_cast<QAbstractButton*>(GTWidget::findWidget(os, "tbBrowseExistingTable", dialog));
    GT_CHECK(b != NULL, "Button is NULL");
    GT_CHECK(b->isEnabled() == false, "Button is enabled");

    QAbstractButton *newTableRB = qobject_cast<QAbstractButton*>(GTWidget::findWidget(os, "rbCreateNewTable", dialog));
    GT_CHECK(newTableRB != NULL, "Radiobutton is NULL");
    GT_CHECK(newTableRB->isEnabled() && newTableRB->isChecked(), "radioButton is not enabled and checked");

    if (!groupName.isEmpty()) {
        QLineEdit *lineEdit = dialog->findChild<QLineEdit*>("leGroupName");
        GT_CHECK(lineEdit != NULL, "line edit not found");
        QString text = lineEdit->text();
        GT_CHECK (text == groupName, "This name is not expected name");
        }

    if (!annotationName.isEmpty()) {
        QLineEdit *lineEdit1 = dialog->findChild<QLineEdit*>("leAnnotationName");
        GT_CHECK(lineEdit1 != NULL, "line edit not found");
        QString text1 = lineEdit1->text();
        GT_CHECK (text1 == annotationName, "This name is not expected name");
        }

    if (!location.isEmpty()) {
        QLineEdit *lineEdit2 = dialog->findChild<QLineEdit*>("leLocation");
        GT_CHECK(lineEdit2 != NULL, "line edit not found");
        QString text2 = lineEdit2->text();
        GT_CHECK (text2 == location, "This name is not expected name");
        }

    QDialogButtonBox* box = qobject_cast<QDialogButtonBox*>(GTWidget::findWidget(os, "buttonBox", dialog));
    GT_CHECK(box != NULL, "buttonBox is NULL");
    QPushButton* button = box->button(QDialogButtonBox::Cancel);
    GT_CHECK(button !=NULL, "ok button is NULL");
    GTWidget::click(os, button);
}
#undef GT_METHOD_NAME
#undef GT_CLASS_NAME

#define GT_CLASS_NAME "GTUtilsDialog::CreateAnnotationDialogFiller"
#define GT_METHOD_NAME "run"
void CreateAnnotationWidgetFiller::run() {
#ifdef Q_OS_MAC
    GTKeyboardDriver::keyRelease(os,GTKeyboardDriver::key["cmd"]);
#endif
    QWidget* dialog = QApplication::activeModalWidget();
    GT_CHECK(dialog, "activeModalWidget is NULL");

    if (newTableRB) {
        QRadioButton *newFileRb = qobject_cast<QRadioButton *>(GTWidget::findWidget(os, "rbCreateNewTable", dialog));
        GT_CHECK(newFileRb != NULL, "Radio button is NULL");
        GTRadioButton::click(os, newFileRb);
        QLineEdit *leFilePath = qobject_cast<QLineEdit *>(GTWidget::findWidget(os, "leNewTablePath", dialog));
        GT_CHECK(leFilePath != NULL, "File path line edit is NULL");
        if (!saveTo.isEmpty()) {
            QDir().mkpath(QFileInfo(saveTo).dir().absolutePath());
            GTLineEdit::setText(os, leFilePath, saveTo);
        }
    } else {
        QRadioButton *existingObjectRb = qobject_cast<QRadioButton *>(GTWidget::findWidget(os, "rbExistingTable", dialog));
        GT_CHECK(existingObjectRb != NULL, "Radio button is NULL");
        GTRadioButton::click(os, existingObjectRb);
        QComboBox *cbExistingObject = qobject_cast<QComboBox *>(GTWidget::findWidget(os, "cbExistingTable", dialog));
        GT_CHECK(cbExistingObject != NULL, "Existing object combobox is NULL");
        if (!saveTo.isEmpty()) {
            GTComboBox::setIndexWithText(os, cbExistingObject, saveTo);
        }
    }

    QLineEdit *groupNameLineEdit = qobject_cast<QLineEdit*>(GTWidget::findWidget(os, "leGroupName", dialog));
    GT_CHECK(groupNameLineEdit != NULL, "LineEdit is NULL");
    GTLineEdit::setText(os, groupNameLineEdit, groupName);

    QLineEdit *annotationNameLineEdit = qobject_cast<QLineEdit*>(GTWidget::findWidget(os, "leAnnotationName", dialog));
    GT_CHECK(annotationNameLineEdit != NULL, "LineEdit is NULL");
    GTLineEdit::setText(os, annotationNameLineEdit, annotationName);

    QLineEdit *locationLineEdit = qobject_cast<QLineEdit*>(GTWidget::findWidget(os, "leLocation", dialog));
    GT_CHECK(locationLineEdit != NULL, "LineEdit is NULL");
    GTLineEdit::setText(os, locationLineEdit, location);

    QDialogButtonBox* box = qobject_cast<QDialogButtonBox*>(GTWidget::findWidget(os, "buttonBox", dialog));
    QPushButton* button = box->button(QDialogButtonBox::Ok);
    GT_CHECK(button !=NULL, "ok button is NULL");
    GTWidget::click(os, button);
}
#undef GT_METHOD_NAME
#undef GT_CLASS_NAME

}
