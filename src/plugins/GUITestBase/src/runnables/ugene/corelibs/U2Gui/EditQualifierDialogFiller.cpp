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

#include "EditQualifierDialogFiller.h"
#include "api/GTWidget.h"
#include "api/GTLineEdit.h"
#include "api/GTTextEdit.h"
#include "api/GTRadioButton.h"
#include "api/GTComboBox.h"
#include "api/GTCheckBox.h"
#include "api/GTKeyboardDriver.h"
#include "api/GTKeyboardUtils.h"
#include "runnables/qt/MessageBoxFiller.h"

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
    GTLineEdit::setText(os, nameEdit, qualifierName, noCheck);

    QTextEdit *valueEdit = dialog->findChild<QTextEdit*>("valueEdit");
    GT_CHECK(valueEdit != NULL, "value line edit not found");
    GTTextEdit::setText(os, valueEdit, valueName);

    QDialogButtonBox* box = qobject_cast<QDialogButtonBox*>(GTWidget::findWidget(os, "buttonBox", dialog));
    GT_CHECK(box != NULL, "buttonBox is NULL");
    QPushButton* button = box->button(QDialogButtonBox::Ok);
    GT_CHECK(button !=NULL, "ok button is NULL");
    
    if(closeErrormessageBox){
        GTUtilsDialog::waitForDialog(os, new MessageBoxDialogFiller(os, QMessageBox::Ok));
        GTGlobals::sleep();
        GTKeyboardDriver::keyClick(os, GTKeyboardDriver::key["Enter"]);
        GTGlobals::sleep();

        GTLineEdit::setText(os, nameEdit, "nice_name", noCheck);
        GTTextEdit::setText(os, valueEdit, "nice_val");
        GTWidget::click(os, button);
    }else{
        GTWidget::click(os, button);
    }
    
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
