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

#include "DotPlotDialogFiller.h"
#include "api/GTWidget.h"
#include "api/GTSpinBox.h"
#include "api/GTCheckBox.h"
#include <QtGui/QPushButton>

#include <QtGui/QApplication>


#include "api/GTWidget.h"
#include "api/GTSpinBox.h"
#include "api/GTDoubleSpinBox.h"
#include "api/GTCheckBox.h"
#include "api/GTLineEdit.h"
#include "api/GTComboBox.h"
#include "api/GTRadioButton.h"
#include <QtGui/QApplication>
#include <QtGui/QGroupBox>
#include <QtGui/QComboBox>

namespace U2 {

#define GT_CLASS_NAME "GTUtilsDialog::DotPlotFiller"
#define GT_METHOD_NAME "run"
void DotPlotFiller::run() {

    QWidget* dialog = QApplication::activeModalWidget();
    GT_CHECK(dialog, "activeModalWidget is NULL");

    QSpinBox *minLenBox = qobject_cast<QSpinBox*>(GTWidget::findWidget(os, "minLenBox", dialog));
    if(but1kpressed){
        GTWidget::click(os, GTWidget::findWidget(os, "minLenHeuristicsButton", dialog));
        GT_CHECK(minLenBox->value()==2,"minLem not 2, 1k button works wrong");
    }
    else
        GTSpinBox::setValue(os, minLenBox, minLen,GTGlobals::UseKey);

    if (identity) {
        QSpinBox *identityBox = qobject_cast<QSpinBox*>(GTWidget::findWidget(os, "identityBox", dialog));
        GTSpinBox::setValue(os, identityBox, identity,GTGlobals::UseKey);
    }

    QCheckBox* invertedCheckBox = qobject_cast<QCheckBox*>(GTWidget::findWidget(os, "invertedCheckBox", dialog));
    GTCheckBox::setChecked(os, invertedCheckBox, invertedRepeats);

    GTWidget::click(os, GTWidget::findWidget(os, "startButton", dialog));
}
#undef GT_METHOD_NAME
#undef GT_CLASS_NAME





#define GT_CLASS_NAME "GTUtilsDialog::AlignShortReadsFiller"
#define GT_METHOD_NAME "run"

void DotPlotFilesDialogFiller::run() {
QWidget* dialog = QApplication::activeModalWidget();
GT_CHECK(dialog, "activeModalWidget is NULL");

QLineEdit* firstFileEdit = qobject_cast<QLineEdit*>(GTWidget::findWidget(os, "firstFileEdit", dialog));
GT_CHECK(firstFileEdit, "firstFileEdit is NULL");
GTLineEdit::setText(os, firstFileEdit, parameters->line_firstFileEdit_text);

if(parameters->button_openFirstButton_clicked){
GTWidget::click(os, GTWidget::findWidget(os,"openFirstButton"));
}

QCheckBox* mergeFirstCheckBox = qobject_cast<QCheckBox*>(GTWidget::findWidget(os, "mergeFirstCheckBox", dialog));
GT_CHECK(mergeFirstCheckBox, "mergeFirstCheckBox is NULL");
GTCheckBox::setChecked(os, mergeFirstCheckBox, parameters->is_mergeFirstCheckBox_checked);

QSpinBox* gapFirst = qobject_cast<QSpinBox*>(GTWidget::findWidget(os, "gapFirst", dialog));
GT_CHECK(gapFirst, "gapFirst is NULL")
;GTSpinBox::setValue(os, gapFirst, parameters->spin_gapFirst_value);QCheckBox* oneSequenceCheckBox = qobject_cast<QCheckBox*>(GTWidget::findWidget(os, "oneSequenceCheckBox", dialog));
GT_CHECK(oneSequenceCheckBox, "oneSequenceCheckBox is NULL");
GTCheckBox::setChecked(os, oneSequenceCheckBox, parameters->is_oneSequenceCheckBox_checked);

QLineEdit* secondFileEdit = qobject_cast<QLineEdit*>(GTWidget::findWidget(os, "secondFileEdit", dialog));
GT_CHECK(secondFileEdit, "secondFileEdit is NULL");
GTLineEdit::setText(os, secondFileEdit, parameters->line_secondFileEdit_text);

if(parameters->button_openSecondButton_clicked){
GTWidget::click(os, GTWidget::findWidget(os,"openSecondButton"));
}

QCheckBox* mergeSecondCheckBox = qobject_cast<QCheckBox*>(GTWidget::findWidget(os, "mergeSecondCheckBox", dialog));
GT_CHECK(mergeSecondCheckBox, "mergeSecondCheckBox is NULL");
GTCheckBox::setChecked(os, mergeSecondCheckBox, parameters->is_mergeSecondCheckBox_checked);

QSpinBox* gapSecond = qobject_cast<QSpinBox*>(GTWidget::findWidget(os, "gapSecond", dialog));
GT_CHECK(gapSecond, "gapSecond is NULL")
;GTSpinBox::setValue(os, gapSecond, parameters->spin_gapSecond_value);if(parameters->button_startButton_clicked){
GTWidget::click(os, GTWidget::findWidget(os,"startButton"));
}

if(parameters->button_cancelButton_clicked){
GTWidget::click(os, GTWidget::findWidget(os,"cancelButton"));
}

}
#undef GT_METHOD_NAME
#undef GT_CLASS_NAME

}
