/**
 * UGENE - Integrated Bioinformatics Tools.
 * Copyright (C) 2008-2013 UniPro <ugene@unipro.ru>
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

#include "SequenceReadingModeSelectorDialogFiller.h"
#include "api/GTRadioButton.h"
#include "api/GTSpinBox.h"
#include "api/GTWidget.h"

#include <QtGui/QApplication>
#include <QtGui/QDialogButtonBox>
#include <QtGui/QPushButton>

namespace U2 {

#define GT_CLASS_NAME "GTUtilsDialog::SequenceReadingModeSelectorDialogFiller"
#define GT_METHOD_NAME "run"
void SequenceReadingModeSelectorDialogFiller::run()
{
    QWidget *dialog = QApplication::activeModalWidget();
    GT_CHECK(dialog != NULL, "dialog not found");
    if (readingMode == Separate) {
        QRadioButton *separateRB = dialog->findChild<QRadioButton*>(QString::fromUtf8("separateRB"));
        GT_CHECK(separateRB != NULL, "radio button not found");
        GTRadioButton::click(os, separateRB);
    }
    if (readingMode == Merge) {
        QRadioButton *mergeRB = dialog->findChild<QRadioButton*>(QString::fromUtf8("mergeRB"));
        GT_CHECK(mergeRB != NULL, "radio button not found");
        GTRadioButton::click(os, mergeRB);

        QSpinBox *mergeSpinBox = dialog->findChild<QSpinBox*>(QString::fromUtf8("mergeSpinBox"));
        GT_CHECK(mergeSpinBox != NULL, "merge spin box not found");
        GTSpinBox::setValue(os, mergeSpinBox, bases);
    }
    if (readingMode == Join) {
        QRadioButton *malignmentRB = dialog->findChild<QRadioButton*>(QString::fromUtf8("malignmentRB"));
        GT_CHECK(malignmentRB != NULL, "radio button not found");
        GTRadioButton::click(os, malignmentRB);
    }
    if (readingMode == Align) {
        QRadioButton *refalignmentRB = dialog->findChild<QRadioButton*>(QString::fromUtf8("refalignmentRB"));
        GT_CHECK(refalignmentRB != NULL, "radio button not found");
        GTRadioButton::click(os, refalignmentRB);
    }
    QDialogButtonBox *buttonBox = dialog->findChild<QDialogButtonBox*>(QString::fromUtf8("buttonBox"));
    GT_CHECK(buttonBox != NULL, "buttonBox not found");

    QPushButton *button = buttonBox->button(QDialogButtonBox::Ok);
    GT_CHECK(button != NULL, "standart button not found");
    GTWidget::click(os, button);
}
#undef GT_METHOD_NAME
#undef GT_CLASS_NAME

}
