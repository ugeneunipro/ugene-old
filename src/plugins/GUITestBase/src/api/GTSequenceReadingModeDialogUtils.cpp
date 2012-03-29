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

#include "api/GTSequenceReadingModeDialogUtils.h"
#include "api/GTSequenceReadingModeDialog.h"
#include "api/GTGlobals.h"
#include "api/GTMouseDriver.h"
#include "api/GTKeyboardDriver.h"
#include "api/GTLineEdit.h"
#include "api/GTWidget.h"
#include <QtGui/QApplication>
#include <QtGui/QWidget>
#include <QRadioButton>
#include <QCheckBox>
#include <QLineEdit>
#include <QPushButton>

#define SEPARATE_MODE "separateMode"
#define MERGE_MODE "mergeMode"
#define INTERAL_GAP "internalGap"
#define FILE_GAP "fileGap"
#define NEW_DOC_NAME "newUrl"
#define SAVE_BOX "saveBox"



namespace U2 {
GTSequenceReadingModeDialogUtils::GTSequenceReadingModeDialogUtils(U2OpStatus &o) : os(o), dialog(NULL)
{
}

void GTSequenceReadingModeDialogUtils::run()
{
    QWidget *openDialog = QApplication::activeModalWidget();
    CHECK_SET_ERR (openDialog != NULL,
                   "Error: dialog not found in GTSequenceReadingModeDialogUtils::run()");

    dialog = openDialog;

    selectMode();

    if (GTSequenceReadingModeDialog::mode == GTSequenceReadingModeDialog::Merge) {
        setNumSymbolsParts();
        setNumSymbolsFiles();
        setNewDocumentName();
        selectSaveDocument();
    }

    clickButton();
}

void GTSequenceReadingModeDialogUtils::selectMode()
{
    QString buttonName = SEPARATE_MODE;
    if (GTSequenceReadingModeDialog::mode == GTSequenceReadingModeDialog::Merge) {
        buttonName = MERGE_MODE;
    }

    QRadioButton *radioButton = dialog->findChild<QRadioButton*>(buttonName);
    CHECK_SET_ERR (radioButton != NULL, "Error: radio button not found in GTSequenceReadingModeDialogUtils::selectMode()");

    if (! radioButton->isChecked()) {
        switch (GTSequenceReadingModeDialog::useMethod) {
        case GTGlobals::UseMouse:
            GTMouseDriver::moveTo(os, radioButton->mapToGlobal(radioButton->rect().topLeft()));
            GTMouseDriver::click(os);
            break;

        case GTGlobals::UseKey:
            GTWidget::setFocus(os, radioButton);
            GTKeyboardDriver::keyClick(os, GTKeyboardDriver::key["space"]);
            break;
        }
    }

    while (! radioButton->isChecked()) {
        GTGlobals::sleep(100);
    }
}

void GTSequenceReadingModeDialogUtils::setNumSymbolsParts()
{
    QSpinBox *spinBox = dialog->findChild<QSpinBox*>(INTERAL_GAP);
    CHECK_SET_ERR (spinBox != NULL, "Error: spinBox not found in GTSequenceReadingModeDialogUtils::setNumSymbolParts()");

    changeSpinBoxValue(spinBox, GTSequenceReadingModeDialog::numSymbolParts);
}

void GTSequenceReadingModeDialogUtils::setNumSymbolsFiles()
{
    QSpinBox *spinBox = dialog->findChild<QSpinBox*>(FILE_GAP);
    CHECK_SET_ERR (spinBox != NULL, "Error: spinBox not found in GTSequenceReadingModeDialogUtils::setNumSymbolFiles()");

    changeSpinBoxValue(spinBox, GTSequenceReadingModeDialog::numSymbolFiles);
}

void GTSequenceReadingModeDialogUtils::setNewDocumentName()
{
    if (GTSequenceReadingModeDialog::newDocName == QString()) {
        return;
    }

    QLineEdit *lineEdit = dialog->findChild<QLineEdit*>(NEW_DOC_NAME);
    CHECK_SET_ERR (lineEdit != NULL, "Error: lineEdit not found in GTSequenceReadingModeDialogUtils::setNewDocumentName()");

    GTLineEdit::clear(os, lineEdit);
    GTLineEdit::setText(os, lineEdit, GTSequenceReadingModeDialog::newDocName);
}

void GTSequenceReadingModeDialogUtils::selectSaveDocument()
{
    QCheckBox *saveBox = dialog->findChild<QCheckBox*>(SAVE_BOX);
    CHECK_SET_ERR (saveBox != NULL, "Error: save check box not found in GTSequenceReadingModeDialogUtils::selectSaveDocument()");

    if (GTSequenceReadingModeDialog::saveDocument != saveBox->isChecked()) {
        switch (GTSequenceReadingModeDialog::useMethod) {
        case GTGlobals::UseMouse:
            GTMouseDriver::moveTo(os, saveBox->mapToGlobal(QPoint(saveBox->rect().left() + 10, saveBox->rect().height() / 2)));
            GTMouseDriver::click(os);
            break;

        case GTGlobals::UseKey:
            while (! saveBox->hasFocus()) {
                GTKeyboardDriver::keyClick(os, GTKeyboardDriver::key["tab"]);
                GTGlobals::sleep(100);
            }
            GTKeyboardDriver::keyClick(os, GTKeyboardDriver::key["space"]);
        }
    }
}

void GTSequenceReadingModeDialogUtils::clickButton()
{
    QList<QPushButton*> buttonList = dialog->findChildren<QPushButton*>();
    CHECK_SET_ERR (buttonList.size() != 0, "Error: button not found in GTSequenceReadingModeDialogUtils::clickButton()");

    QMap<GTSequenceReadingModeDialog::Button, QString> buttons;
    buttons[GTSequenceReadingModeDialog::Ok] = QPushButton::tr("&OK");
    buttons[GTSequenceReadingModeDialog::Cancel] = QPushButton::tr("&Cancel");

    QPushButton *btn = NULL;

    foreach(QPushButton *b, buttonList) {
        if (b->text() == buttons[GTSequenceReadingModeDialog::button]) {
            btn = b;
            break;
        }
    }
    CHECK_SET_ERR (btn != NULL, "Error: button not found in GTSequenceReadingModeDialogUtils::clickButton()");

    GTWidget::click(os, btn/*, UseMethod*/);
}

void GTSequenceReadingModeDialogUtils::changeSpinBoxValue(QSpinBox *sb, int val)
{
    QPoint arrowPos;
    QRect spinBoxRect;
    int key;

    if (sb->value() != val) {
        switch(GTSequenceReadingModeDialog::useMethod) {
        case GTGlobals::UseMouse:
            spinBoxRect = sb->rect();
            if (val > sb->value()) {
                arrowPos = QPoint(spinBoxRect.right() - 5, spinBoxRect.height() / 4); // -5 it's needed that area under cursor was clickable
            } else {
                arrowPos = QPoint(spinBoxRect.right() - 5, spinBoxRect.height() * 3 / 4);
            }

            GTMouseDriver::moveTo(os, sb->mapToGlobal(arrowPos));
            while (sb->value() != val) {
                GTMouseDriver::click(os);
                GTGlobals::sleep(100);
            }
            break;

        case GTGlobals::UseKey:
            if (val > sb->value()) {
                key = GTKeyboardDriver::key["up"];
            } else {
                key = GTKeyboardDriver::key["down"];
            }

            GTWidget::setFocus(os, sb);
            while (sb->value() != val) {
                GTKeyboardDriver::keyClick(os, key);
                GTGlobals::sleep(100);
            }
        }
    }
}
} // namespace
