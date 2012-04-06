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
#include "api/GTComboBox.h"
#include "api/GTWidget.h"
#include "api/GTLineEdit.h"
#include "api/GTMenu.h"
#include "api/GTMouseDriver.h"
#include "api/GTKeyboardDriver.h"
#include <U2Gui/MainWindow.h>
#include <QtGui/QApplication>
#include <QtGui/QPushButton>
#include <QtGui/QComboBox>
#include <QtGui/QMenu>
#include <QtGui/QRadioButton>

namespace U2 {

void GUIDialogWaiter::wait() {
    QWidget *widget = NULL;

    if (type == Modal) {
        widget = QApplication::activeModalWidget();
    } else {
        widget = QApplication::activePopupWidget();
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

void GTUtilsDialog::preWaitForDialog(U2OpStatus &os, Runnable *r, GUIDialogWaiter::DialogType _type)
{
    GUIDialogWaiter *waiter = new GUIDialogWaiter(r, _type);
    QTimer *t = new QTimer;

    t->connect(t, SIGNAL(timeout()), waiter, SLOT(wait()));
    t->start(100);
}

#undef GT_CLASS_NAME

#define GT_CLASS_NAME "GTUtilsDialog::CreateAnnotationDialogChecker"
#define GT_METHOD_NAME "run"
void GTUtilsDialog::CreateAnnotationDialogChecker::run() {

    QWidget* dialog = QApplication::activeModalWidget();
    GT_CHECK(dialog, "activeModalWidget is NULL");

    QAbstractButton *radioButton = qobject_cast<QAbstractButton*>(GTWidget::findWidget(os, "existingObjectRB", dialog));
    GT_CHECK(radioButton != NULL, "Radiobutton is NULL");
    GT_CHECK(!radioButton->isEnabled() && !radioButton->isCheckable(), "radioButton is enabled or checkable");

    QComboBox *comboBox = qobject_cast<QComboBox*>(GTWidget::findWidget(os, "existingObjectCombo", dialog));
    GT_CHECK(comboBox != NULL, "ComboBox is NULL");
    GT_CHECK(comboBox->count() == 0, "comboBox count is not 0");

    QAbstractButton *b = qobject_cast<QAbstractButton*>(GTWidget::findWidget(os, "existingObjectButton", dialog));
    GT_CHECK(b != NULL, "Button is NULL");
    GT_CHECK(b->isEnabled() == false, "Button is enabled");

    QWidget *cancelButton = GTWidget::findWidget(os, "cancel_button", dialog);
    GT_CHECK(cancelButton != NULL, "Cancel button is NULL");
    GTWidget::click(os, cancelButton);
}
#undef GT_METHOD_NAME
#undef GT_CLASS_NAME


#define GT_CLASS_NAME "GTUtilsDialog::ExportProjectDialogChecker"
#define GT_METHOD_NAME "run"
void GTUtilsDialog::ExportProjectDialogChecker::run() {

    QWidget* w = QApplication::activeModalWidget();
    if (!w) {
        return;
    }

    QWidget* activeW = QApplication::focusWidget();
    GT_CHECK(activeW != NULL, "Not export project dialog");
    QObject* activeWP = activeW->parent();
    GT_CHECK(activeWP != NULL, "Not export project dialog");

    QList<QLineEdit*> lineEdits;
    foreach (QObject *obj, w->children()) {
        QLineEdit *l = qobject_cast<QLineEdit*>(obj);
        if (l) {
            lineEdits.push_front(l);
        }
    }

    GT_CHECK(lineEdits.size() > 0, "There is no lineEdit in dialog");
    GT_CHECK(lineEdits[0]->text() == projectName, "Project name is not " + projectName);

    QList<QPushButton*> buttons;
    foreach (QObject *obj, activeWP->children()) {
        QPushButton *b = qobject_cast<QPushButton*>(obj);
        if (b) {
            buttons.push_front(b);
        }
    }

    GT_CHECK(buttons.size() == 2, "There aren't 2 QPushButtons");

    GTWidget::click(os, buttons[0]);
}
#undef GT_METHOD_NAME
#undef GT_CLASS_NAME

#define GT_CLASS_NAME "GTUtilsDialog::ExportProjectDialogFiller"
#define GT_METHOD_NAME "run"
void GTUtilsDialog::ExportProjectDialogFiller::run() {

    QWidget* w = QApplication::activeModalWidget();
    if (!w) {
        return;
    }

    QWidget* activeW = QApplication::focusWidget();
    GT_CHECK(activeW != NULL, "Not export project dialog");
    QObject* activeWP = activeW->parent();
    GT_CHECK(activeWP != NULL, "Not export project dialog");

    QList<QLineEdit*> lineEdits;
    foreach (QObject *obj, w->children()) {
        QLineEdit *l = qobject_cast<QLineEdit*>(obj);
        if (l) {
            lineEdits.push_front(l);
        }
    }

    GT_CHECK(lineEdits.size() > 1, "There are no lineEdits in dialog");
    if (!projectName.isEmpty()) {
        GTLineEdit::setText(os, lineEdits[0], projectName);
    }
    GTLineEdit::setText(os, lineEdits[1], projectFolder);


    QList<QPushButton*> buttons;
    foreach (QObject *obj, activeWP->children()) {
        QPushButton *b = qobject_cast<QPushButton*>(obj);
        if (b) {
            buttons.push_front(b);
        }
    }

    GT_CHECK(buttons.size() == 2, "There aren't 2 QPushButtons");

    GTWidget::click(os, buttons[1]);
}
#undef GT_METHOD_NAME
#undef GT_CLASS_NAME

#define GT_CLASS_NAME "GTUtilsDialog::MessageBoxDialogFiller"
#define GT_METHOD_NAME "run"
void GTUtilsDialog::MessageBoxDialogFiller::run() {

    QWidget* activeModal = QApplication::activeModalWidget();
    QMessageBox *messageBox = qobject_cast<QMessageBox*>(activeModal);
    if (!messageBox) {
        return;
    }

    QAbstractButton* button = messageBox->button(b);
    GT_CHECK(button != NULL, "There is no such button in messagebox");

    GTWidget::click(os, button);
}
#undef GT_METHOD_NAME
#undef GT_CLASS_NAME

#define GT_CLASS_NAME "GTUtilsDialog::SaveProjectAsDialogFiller"
#define GT_METHOD_NAME "run"
void GTUtilsDialog::SaveProjectAsDialogFiller::run() {

    QWidget* w = QApplication::activeModalWidget();
    if (!w) {
        return;
    }

    QWidget* activeW = QApplication::focusWidget();
    QObject* activeWP = activeW->parent();
    QObject* activeWPP = activeWP->parent();

    QList<QLineEdit*> lineEdits;
    foreach (QObject *obj, activeWP->children()) {
        QLineEdit *l = qobject_cast<QLineEdit*>(obj);
        if (l) {
            lineEdits.push_front(l);
        }
    }

    GT_CHECK(lineEdits.size() == 3, "There aren't 3 QLineEdits");

    GTLineEdit::setText(os, lineEdits[1], projectFile);
    GTLineEdit::setText(os, lineEdits[2], projectName);
    GTLineEdit::setText(os, lineEdits[0], projectFolder);

    QList<QPushButton*> buttons;
    foreach (QObject *obj, activeWPP->children()) {
        QPushButton *b = qobject_cast<QPushButton*>(obj);
        if (b) {
            buttons.push_front(b);
        }
    }

    GT_CHECK(buttons.size() == 2, "There aren't 2 QPushButtons");

    QPushButton* cancelButton = buttons[0];
    QPushButton* saveButton = buttons[1];

    GTWidget::click(os, saveButton);

    GTUtilsDialog::MessageBoxDialogFiller filler(os, QMessageBox::Yes);
    GTUtilsDialog::waitForDialog(os, &filler, GUIDialogWaiter::Modal, false); // MessageBox question appears only if there is already a file on a disk
}
#undef GT_METHOD_NAME
#undef GT_CLASS_NAME

void GTUtilsDialog::PopupChooser::run()
{
    GTGlobals::sleep(100);
    QMenu* activePopupMenu = qobject_cast<QMenu*>(QApplication::activePopupWidget());
    GTMenu::clickMenuItem(os, activePopupMenu, namePath, useMethod);
}

#define GT_CLASS_NAME "GTUtilsDialog::RemoteDBDialogFiller"
#define GT_METHOD_NAME "run"
void GTUtilsDialog::RemoteDBDialogFiller::run() {
    QWidget* w = QApplication::activeModalWidget();
    if (!w) {
        return;
    }

    QWidget* activeW = QApplication::focusWidget();
    QObject* activeWP = activeW->parent();
    QObject* activeWPP = activeWP->parent();

    QList<QLineEdit*> lineEdits;
    foreach (QObject *obj, activeWP->children()) {
        QLineEdit *l = qobject_cast<QLineEdit*>(obj);
        if (l) {
            lineEdits.push_front(l);
        }
    }

    GT_CHECK(lineEdits.size() == 2, "There aren't 3 QLineEdits");

    GTLineEdit::setText(os, lineEdits[1], resID);
    if(!saveDirPath.isEmpty()){
        GTLineEdit::setText(os, lineEdits[0], saveDirPath);
    }

    QList<QComboBox*> comboBoxes;
    foreach (QObject *obj, activeWP->children()) {
        QComboBox *c = qobject_cast<QComboBox*>(obj);
        if (c) {
            comboBoxes.push_front(c);
        }
    }

    GT_CHECK(comboBoxes.size() == 1, "There aren't 1 QComboBox");

    GTComboBox::setCurrentIndex(os, comboBoxes.first(), DBItemNum);
    
    QList<QPushButton*> buttons;
    foreach (QObject *obj, activeWP->children()) {
        QPushButton *b = qobject_cast<QPushButton*>(obj);
        if (b) {
            buttons.push_front(b);
        }
    }

    GT_CHECK(buttons.size() == 2, "There aren't 2 QPushButtons");

    QPushButton* okButton = buttons[1];
    QPushButton* cancelButton = buttons[0];

    if (pressCancel) {
        GTWidget::click(os, cancelButton);
    }else {
        GTWidget::click(os, okButton);
    }
}
#undef GT_METHOD_NAME
#undef GT_CLASS_NAME

#define GT_CLASS_NAME "GTUtilsDialog::ExportToSequenceFormatFiller"
#define GT_METHOD_NAME "run"
void GTUtilsDialog::ExportToSequenceFormatFiller::run()
{
    QWidget *dialog = QApplication::activeModalWidget();
    GT_CHECK(dialog != NULL, "dialog not found");

    QLineEdit *lineEdit = dialog->findChild<QLineEdit*>();
    GT_CHECK(lineEdit != NULL, "line edit not found");

    GTLineEdit::setText(os, lineEdit, path + name);

    QRadioButton *button = dialog->findChild<QRadioButton*>(QString::fromUtf8("keepGapsRB"));
    GT_CHECK(button != NULL, "radio button not found");

    switch(useMethod) {
    case GTGlobals::UseMouse:
        GTMouseDriver::moveTo(os, button->mapToGlobal(button->rect().topLeft()));
        GTMouseDriver::click(os);
        break;
    case GTGlobals::UseKey:
        GTWidget::setFocus(os, button);
        GTKeyboardDriver::keyClick(os, GTKeyboardDriver::key["space"]);
        break;
    }

    QPushButton *okButton = dialog->findChild<QPushButton*>(QString::fromUtf8("okButton"));
    GT_CHECK(okButton != NULL, "OK button not found");

    GTGlobals::sleep(100);
    GTWidget::click(os, okButton);

}
#undef GT_METHOD_NAME
#undef GT_CLASS_NAME

}
