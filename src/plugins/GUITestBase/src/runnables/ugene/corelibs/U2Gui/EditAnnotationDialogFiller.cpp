/**
 * UGENE - Integrated Bioinformatics Tools.
 * Copyright (C) 2008-2016 UniPro <ugene@unipro.ru>
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

#include <QApplication>
#include <QDialogButtonBox>
#include <QDir>
#include <QPushButton>
#include <QToolButton>

#include "EditAnnotationDialogFiller.h"
#include <primitives/GTCheckBox.h>
#include <primitives/GTComboBox.h>
#include <primitives/GTLineEdit.h>
#include <primitives/GTRadioButton.h>
#include <primitives/GTWidget.h>

namespace U2 {

#define GT_CLASS_NAME "GTUtilsDialog::EditAnnotationFiller"
#define GT_METHOD_NAME "run"
void EditAnnotationFiller::commonScenario()
{
    QWidget *dialog = QApplication::activeModalWidget();
    GT_CHECK(dialog != NULL, "dialog not found");

    QLineEdit *lineEdit = dialog->findChild<QLineEdit*>("nameEdit");
    GT_CHECK(lineEdit != NULL, "line edit not found");
    GTLineEdit::setText(os, lineEdit, annotationName);

    QLineEdit *lineEdit1 = dialog->findChild<QLineEdit*>("locationEdit");
    GT_CHECK(lineEdit != NULL, "line edit not found");
    GTLineEdit::setText(os, lineEdit1, location);

    if (complementStrand != false)
        {
        QToolButton* complementStrand = dialog->findChild<QToolButton*>("complementButton");
        GTWidget::click(os, complementStrand);
        }

    GTUtilsDialog::clickButtonBox(os, dialog, QDialogButtonBox::Ok);
}
#undef GT_METHOD_NAME
#undef GT_CLASS_NAME

#define GT_CLASS_NAME "GTUtilsDialog::EditAnnotationChecker"

#define GT_METHOD_NAME "commonScenario"
void EditAnnotationChecker::commonScenario() {
    QWidget *dialog = QApplication::activeModalWidget();
    GT_CHECK(dialog != NULL, "dialog not found");

    if (!annotationName.isEmpty()) {
        QLineEdit *lineEdit = dialog->findChild<QLineEdit*>("nameEdit");
        GT_CHECK(lineEdit != NULL, "line edit not found");
        QString text = lineEdit->text();

        GT_CHECK (text == annotationName, "This name is not expected name");
    }

    if (!location.isEmpty()){
        QLineEdit *lineEdit1 = dialog->findChild<QLineEdit*>("locationEdit");
        GT_CHECK(lineEdit1 != NULL, "line edit not found");

        QString text = lineEdit1->text();

        GT_CHECK (text == location, "This name is not expected name");
    }

    GTUtilsDialog::clickButtonBox(os, dialog, QDialogButtonBox::Cancel);
}
#undef GT_METHOD_NAME

#undef GT_CLASS_NAME

}   // namespace U2
