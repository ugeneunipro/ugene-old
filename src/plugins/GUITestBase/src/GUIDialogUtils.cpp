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

#include "GUIDialogUtils.h"
#include "QtUtils.h"

#include <U2Core/U2SafePoints.h>

namespace U2 {

void GUIDialogUtils::clickMessageBoxButton(U2OpStatus &os, QMessageBox::StandardButton b) {

    QWidget* activeModal = QApplication::activeModalWidget();
    QMessageBox *messageBox = qobject_cast<QMessageBox*>(activeModal);
    if (!messageBox) {
        return;
    }

    QAbstractButton* button = messageBox->button(b);
    CHECK_SET_ERR(button != NULL, "There is no such button in messagebox");

    button->click();
    QtUtils::sleep(500);
}

void GUIDialogUtils::fillInSaveProjectAsDialog(U2OpStatus &os, const QString &projectName, const QString &projectFolder, const QString &projectFile, bool pressCancel) {

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

    lineEdits[2]->setText(projectName);
    lineEdits[0]->setText(projectFolder);
    lineEdits[1]->setText(projectFile);

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
        cancelButton->click();
    }
    else {
        saveButton->click();
    }

    QtUtils::sleep(500);
}

}
