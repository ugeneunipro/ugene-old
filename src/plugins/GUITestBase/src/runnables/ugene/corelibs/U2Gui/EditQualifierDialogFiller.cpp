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

#include "EditQualifierDialogFiller.h"
#include "api/GTWidget.h"
#include "api/GTLineEdit.h"
#include "api/GTTextEdit.h"
#include "api/GTRadioButton.h"
#include "api/GTComboBox.h"
#include "api/GTCheckBox.h"
#include "api/GTKeyboardDriver.h"
#include "api/GTKeyboardUtils.h"

#include <QtCore/QDir>
#include <QtGui/QApplication>
#include <QtGui/QDialogButtonBox>
#include <QtGui/QPushButton>
#include <QtGui/QToolButton>


namespace U2 {

#define GT_CLASS_NAME "GTUtilsDialog::EditQualifierFiller"
#define GT_METHOD_NAME "run"
void EditQualifierFiller::run()
{
    QWidget *dialog = QApplication::activeModalWidget();
    GT_CHECK(dialog != NULL, "dialog not found");

    QLineEdit *nameEdit = dialog->findChild<QLineEdit*>("nameEdit");
    GT_CHECK(nameEdit != NULL, "line edit not found");
    GTLineEdit::setText(os, nameEdit, qualifierName);

    QTextEdit *valueEdit = dialog->findChild<QTextEdit*>("valueEdit");
    GT_CHECK(valueEdit != NULL, "value line edit not found");
    GTTextEdit::setText(os, valueEdit, valueName);

    QPushButton* okButton = dialog->findChild<QPushButton*>("okButton");
    GT_CHECK(okButton != NULL, "okButton is NULL");
    GTWidget::click(os, okButton);
}
#undef GT_METHOD_NAME
#undef GT_CLASS_NAME

#define GT_CLASS_NAME "GTUtilsDialog::RenameQualifierFiller"
#define GT_METHOD_NAME "run"
void RenameQualifierFiller::run()
    {
    QWidget *dialog = QApplication::activeModalWidget();
    GT_CHECK(dialog != NULL, "dialog not found");

    QLineEdit *nameEdit = dialog->findChild<QLineEdit*>();
    GT_CHECK(nameEdit != NULL, "line edit not found");
    GTLineEdit::setText(os, nameEdit, newName);

    GTKeyboardDriver::keyClick(os, GTKeyboardDriver::key["Enter"]);
    GTGlobals::sleep();
    }
#undef GT_METHOD_NAME
#undef GT_CLASS_NAME

}
