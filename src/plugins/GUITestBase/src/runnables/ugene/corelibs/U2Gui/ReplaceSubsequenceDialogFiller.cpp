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

#include "ReplaceSubsequenceDialogFiller.h"
#include "api/GTWidget.h"
#include "api/GTLineEdit.h"
#include "api/GTComboBox.h"
#include "api/GTRadioButton.h"
#include "api/GTPlainTextEdit.h"


#include <QtCore/QDir>
#if (QT_VERSION < 0x050000) //Qt 5
#include <QtGui/QApplication>
#include <QtGui/QPushButton>
#include <QtGui/QRadioButton>
#include <QtGui/QDialogButtonBox>
#include <QtGui/QGroupBox>
#include "api/GTKeyboardDriver.h"

#else
#include <QtWidgets/QApplication>
#include <QtWidgets/QPushButton>
#include <QtWidgets/QRadioButton>
#include <QtWidgets/QDialogButtonBox>
#include <QtWidgets/QGroupBox>
#endif

namespace U2 {

#define GT_CLASS_NAME "GTUtilsDialog::replaceSubsequenceDialogFiller"
    ReplaceSubsequenceDialogFiller::ReplaceSubsequenceDialogFiller(U2OpStatus &_os, const QString &_pasteDataHere):
    Filler(_os, "EditSequenceDialog"), pasteDataHere(_pasteDataHere)
    {
    }

#define GT_METHOD_NAME "run"
    void ReplaceSubsequenceDialogFiller::run()
    {
        QWidget *dialog = QApplication::activeModalWidget();
        GT_CHECK(dialog != NULL, "dialog not found");

        QPlainTextEdit *plainText = dialog->findChild<QPlainTextEdit*>("sequenceEdit");
        GT_CHECK(plainText != NULL, "plain text not found");
        //GTKeyboardDriver::keyClick(os, GTKeyboardDriver::key["a"], GTKeyboardDriver::key["ctrl"]);
        //GTGlobals::sleep();
        GTKeyboardDriver::keyClick(os, GTKeyboardDriver::key["delete"]);
        GTGlobals::sleep();
        GTPlainTextEdit::setPlainText(os, plainText, pasteDataHere);

        QDialogButtonBox* box = qobject_cast<QDialogButtonBox*>(GTWidget::findWidget(os, "buttonBox", dialog));
        GT_CHECK(box != NULL, "buttonBox is NULL");
        QPushButton* button = box->button(QDialogButtonBox::Ok);
        GT_CHECK(button !=NULL, "cancel button is NULL");
        GTWidget::click(os, button);

    }
#undef GT_METHOD_NAME
#undef GT_CLASS_NAME

}
