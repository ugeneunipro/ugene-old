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

#include <QToolButton>

#include <QtCore/qglobal.h>
#if (QT_VERSION < 0x050000) //Qt 5
#include <QtGui/QApplication>
#include <QtGui/QGroupBox>
#include <QtGui/QComboBox>
#else
#include <QtWidgets/QApplication>
#include <QtWidgets/QGroupBox>
#include <QtWidgets/QComboBox>
#endif

#include "api/GTWidget.h"
#include "api/GTSpinBox.h"
#include "api/GTDoubleSpinBox.h"
#include "api/GTCheckBox.h"
#include "api/GTLineEdit.h"
#include "api/GTComboBox.h"
#include "api/GTRadioButton.h"

#include "FormatDBDialogFiller.h"

namespace U2 {

FormatDBSupportRunDialogFiller::FormatDBSupportRunDialogFiller(U2OpStatus &os, const Parameters &parameters)
: Filler(os, "FormatDBSupportRunDialog"), parameters(parameters)
{

}

#define GT_CLASS_NAME "GTUtilsDialog::FormatDBSupportRunDialogFiller"
#define GT_METHOD_NAME "run"

void FormatDBSupportRunDialogFiller::run() {
    QWidget *dialog = QApplication::activeModalWidget();
    GT_CHECK(dialog, "activeModalWidget is NULL");

    QRadioButton *inputFilesRadioButton = qobject_cast<QRadioButton*>(GTWidget::findWidget(os, "inputFilesRadioButton", dialog));
    GT_CHECK(inputFilesRadioButton, "inputFilesRadioButton not found");
    QLineEdit *inputFilesLineEdit = qobject_cast<QLineEdit*>(GTWidget::findWidget(os, "inputFilesLineEdit", dialog));
    GT_CHECK(inputFilesLineEdit, "inputFilesLineEdit is NULL");
    QToolButton *inputFilesToolButton = qobject_cast<QToolButton*>(GTWidget::findWidget(os, "inputFilesToolButton", dialog));
    GT_CHECK(inputFilesToolButton, "inputFilesToolButton is NULL");

    QRadioButton *inputDirRadioButton = qobject_cast<QRadioButton*>(GTWidget::findWidget(os, "inputDirRadioButton", dialog));
    GT_CHECK(inputDirRadioButton, "inputDirRadioButton not found");
    QLineEdit *inputDirLineEdit = qobject_cast<QLineEdit*>(GTWidget::findWidget(os, "inputDirLineEdit", dialog));
    GT_CHECK(inputDirLineEdit, "inputDirLineEdit is NULL");
    QToolButton *inputDirToolButton = qobject_cast<QToolButton*>(GTWidget::findWidget(os, "inputDirToolButton", dialog));
    GT_CHECK(inputDirLineEdit, "inputDirToolButton is NULL");

    if (parameters.customFiller_3551) {
        GTWidget::click(os, inputFilesRadioButton);
        CHECK_SET_ERR(inputFilesRadioButton->isChecked(), "Files radio button is unchecked");
        CHECK_SET_ERR(inputFilesLineEdit->isEnabled(), "Files lineedit is disabled");
        CHECK_SET_ERR(inputFilesToolButton->isEnabled(), "Files button is disabled");

        GTGlobals::sleep(500);

        GTWidget::click(os, inputDirRadioButton);
        CHECK_SET_ERR(inputDirRadioButton->isChecked(), "Dir radio button is unchecked");
        CHECK_SET_ERR(inputDirLineEdit->isEnabled(), "Dir lineedit is disabled");
        CHECK_SET_ERR(inputDirToolButton->isEnabled(), "Dir button is disabled");
        GTUtilsDialog::clickButtonBox(os, dialog, QDialogButtonBox::Cancel);
    }

    if (parameters.justCancel) {
        GTUtilsDialog::clickButtonBox(os, dialog, QDialogButtonBox::Cancel);
    }
}

#undef GT_METHOD_NAME
#undef GT_CLASS_NAME

} // U2
