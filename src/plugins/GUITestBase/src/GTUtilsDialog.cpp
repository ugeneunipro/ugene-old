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
#include "api/GTWidget.h"
#include "api/GTLineEdit.h"
#include "api/GTMenu.h"
#include <U2Gui/MainWindow.h>
#include <QtGui/QApplication>
#include <QtGui/QPushButton>
#include <QtGui/QComboBox>
#include <QtGui/QMenu>

namespace U2 {

void GUIDialogWaiter::wait() {
    QWidget* modalWidget = QApplication::activeModalWidget();
    QWidget* popupWidget = QApplication::activePopupWidget();
    if (!modalWidget && !popupWidget) {
        return;
    }

    if (r && !hadRun) {
        hadRun = true;
        r->run();
    }
}

void GTUtilsDialog::waitForDialog(U2OpStatus &os, Runnable *r, bool failOnNoDialog) {

    GUIDialogWaiter waiter(r);
    QTimer t;

    t.connect(&t, SIGNAL(timeout()), &waiter, SLOT(wait()));
    t.start(100);

    GTGlobals::sleep(1000);
    if (failOnNoDialog) {
        CHECK_SET_ERR(waiter.hadRun == true, "GUIDialogUtils::waitForDialog: no dialog");
    }
}

void GTUtilsDialog::preWaitForDialog(U2OpStatus &os, Runnable *r)
{
    static GUIDialogWaiter waiter(r);
    static QTimer t;

    t.connect(&t, SIGNAL(timeout()), &waiter, SLOT(wait()));
    t.start(100);
}

void GTUtilsDialog::CreateAnnotationDialogChecker::run() {

    QWidget* dialog = QApplication::activeModalWidget();
    CHECK_SET_ERR(dialog, "activeModalWidget is NULL");

    QAbstractButton *radioButton = qobject_cast<QAbstractButton*>(GTWidget::findWidget(os, "existingObjectRB", dialog));
    CHECK_SET_ERR(radioButton != NULL, "Radiobutton is NULL");
    CHECK_SET_ERR(!radioButton->isEnabled() && !radioButton->isCheckable(), "radioButton is enabled or checkable");

    QComboBox *comboBox = qobject_cast<QComboBox*>(GTWidget::findWidget(os, "existingObjectCombo", dialog));
    CHECK_SET_ERR(comboBox != NULL, "ComboBox is NULL");
    CHECK_SET_ERR(comboBox->count() == 0, "comboBox count is not 0");

    QAbstractButton *b = qobject_cast<QAbstractButton*>(GTWidget::findWidget(os, "existingObjectButton", dialog));
    CHECK_SET_ERR(b != NULL, "Button is NULL");
    CHECK_SET_ERR(b->isEnabled() == false, "Button is enabled");

    QWidget *cancelButton = GTWidget::findWidget(os, "cancel_button", dialog);
    CHECK_SET_ERR(cancelButton != NULL, "Cancel button is NULL");
    GTWidget::click(os, cancelButton);
}

void GTUtilsDialog::openExportProjectDialog(U2OpStatus &os) {

    QMenu *m = GTMenu::showMainMenu(os, MWMENU_FILE);
    GTMenu::clickMenuItem(os, m, ACTION_PROJECTSUPPORT__EXPORT_PROJECT);
}

void GTUtilsDialog::checkExportProjectDialog(U2OpStatus &os, const QString& projectName) {

    QWidget* w = QApplication::activeModalWidget();
    if (!w) {
        return;
    }

    QWidget* activeW = QApplication::focusWidget();
    CHECK_SET_ERR(activeW != NULL, "Not export project dialog");
    QObject* activeWP = activeW->parent();
    CHECK_SET_ERR(activeWP != NULL, "Not export project dialog");

    QList<QLineEdit*> lineEdits;
    foreach (QObject *obj, w->children()) {
        QLineEdit *l = qobject_cast<QLineEdit*>(obj);
        if (l) {
            lineEdits.push_front(l);
        }
    }

    CHECK_SET_ERR(lineEdits.size() > 0, "There is no lineEdit in dialog");
    CHECK_SET_ERR(lineEdits[0]->text() == projectName, "Project name is not " + projectName);

    QList<QPushButton*> buttons;
    foreach (QObject *obj, activeWP->children()) {
        QPushButton *b = qobject_cast<QPushButton*>(obj);
        if (b) {
            buttons.push_front(b);
        }
    }

    CHECK_SET_ERR(buttons.size() == 2, "There aren't 2 QPushButtons in SaveProjectAs dialog");

    GTWidget::click(os, buttons[0]);
}

void GTUtilsDialog::fillInExportProjectDialog(U2OpStatus &os, const QString &projectFolder, const QString &projectName) {

    QWidget* w = QApplication::activeModalWidget();
    if (!w) {
        return;
    }

    QWidget* activeW = QApplication::focusWidget();
    CHECK_SET_ERR(activeW != NULL, "Not export project dialog");
    QObject* activeWP = activeW->parent();
    CHECK_SET_ERR(activeWP != NULL, "Not export project dialog");

    QList<QLineEdit*> lineEdits;
    foreach (QObject *obj, w->children()) {
        QLineEdit *l = qobject_cast<QLineEdit*>(obj);
        if (l) {
            lineEdits.push_front(l);
        }
    }

    CHECK_SET_ERR(lineEdits.size() > 1, "There are no lineEdits in dialog");
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

    CHECK_SET_ERR(buttons.size() == 2, "There aren't 2 QPushButtons in SaveProjectAs dialog");

    GTWidget::click(os, buttons[1]);
}

void GTUtilsDialog::clickMessageBoxButton(U2OpStatus &os, QMessageBox::StandardButton b) {

    QWidget* activeModal = QApplication::activeModalWidget();
    QMessageBox *messageBox = qobject_cast<QMessageBox*>(activeModal);
    if (!messageBox) {
        return;
    }

    QAbstractButton* button = messageBox->button(b);
    CHECK_SET_ERR(button != NULL, "There is no such button in messagebox");

    GTWidget::click(os, button);
}

void GTUtilsDialog::openSaveProjectAsDialog(U2OpStatus &os) {

    QMenu *m = GTMenu::showMainMenu(os, MWMENU_FILE);
    GTMenu::clickMenuItem(os, m, ACTION_PROJECTSUPPORT__SAVE_AS_PROJECT);
}

void GTUtilsDialog::fillInSaveProjectAsDialog(U2OpStatus &os, const QString &projectName, const QString &projectFolder, const QString &projectFile, bool pressCancel) {

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

    CHECK_SET_ERR(lineEdits.size() == 3, "There aren't 3 QLineEdits in SaveProjectAs dialog");

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

    CHECK_SET_ERR(buttons.size() == 2, "There aren't 2 QPushButtons in SaveProjectAs dialog");

    QPushButton* cancelButton = buttons[0];
    QPushButton* saveButton = buttons[1];

    if (pressCancel) {
        GTWidget::click(os, cancelButton);
    }
    else {
        GTWidget::click(os, saveButton);
    }

   GTUtilsDialog::MessageBoxDialogFiller filler(os, QMessageBox::Yes);
   GTUtilsDialog::waitForDialog(os, &filler, false); // MessageBox question appears only if there is already a file on a disk
}

void GTUtilsDialog::PopupChooser::run()
{
    GTGlobals::sleep(100);
    QMenu* activePopupMenu = qobject_cast<QMenu*>(QApplication::activePopupWidget());
    GTMenu::clickMenuItem(os, activePopupMenu, namePath, useMethod);
}

}
