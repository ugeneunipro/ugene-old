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
#include <QtGui/QApplication>
#include <QtGui/QPushButton>
#include <QtGui/QRadioButton>
#include <QtGui/QDialogButtonBox>
#include <QtGui/QGroupBox>
#else
#include <QtWidgets/QApplication>
#include <QtWidgets/QPushButton>
#include <QtWidgets/QRadioButton>
#include <QtWidgets/QDialogButtonBox>
#include <QtWidgets/QGroupBox>
#endif

#include "api/GTCheckBox.h"
#include "api/GTComboBox.h"
#include "drivers/GTKeyboardDriver.h"
#include "api/GTLineEdit.h"
#include "api/GTPlainTextEdit.h"
#include "api/GTRadioButton.h"
#include "primitives/GTWidget.h"

#include "base_dialogs/MessageBoxFiller.h"

#include "ReplaceSubsequenceDialogFiller.h"

namespace U2 {
using namespace HI;

#define GT_CLASS_NAME "GTUtilsDialog::replaceSubsequenceDialogFiller"
ReplaceSubsequenceDialogFiller::ReplaceSubsequenceDialogFiller(U2OpStatus &_os, const QString &_pasteDataHere, bool recalculateQuals)
    : Filler(_os, "EditSequenceDialog"), pasteDataHere(_pasteDataHere), recalculateQuals(recalculateQuals)
{

}

ReplaceSubsequenceDialogFiller::ReplaceSubsequenceDialogFiller(U2OpStatus &os, CustomScenario *scenario)
    : Filler(os, "EditSequenceDialog", scenario)
{

}

#define GT_METHOD_NAME "commonScenario"
void ReplaceSubsequenceDialogFiller::commonScenario()
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

    GTCheckBox::setChecked(os, GTWidget::findExactWidget<QCheckBox *>(os, "recalculateQualsCheckBox"), recalculateQuals);

    GTUtilsDialog::waitForDialogWhichMayRunOrNot(os, new MessageBoxDialogFiller(os, QMessageBox::Ok));

    GTUtilsDialog::clickButtonBox(os, dialog, QDialogButtonBox::Ok);
}

#undef GT_METHOD_NAME
#undef GT_CLASS_NAME

}
