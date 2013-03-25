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

#include "DeleteGapsDialogFiller.h"
#include "api/GTWidget.h"
#include "api/GTRadioButton.h"
#include "api/GTSpinBox.h"

#include <QtGui/QApplication>
#include <QtGui/QAbstractButton>
#include <QtGui/QSpinBox>

namespace U2 {

#define GT_CLASS_NAME "GTUtilsDialog::DeleteGapsDialogFiller"
#define GT_METHOD_NAME "run"
void DeleteGapsDialogFiller::run() {

    QWidget* dialog = QApplication::activeModalWidget();
    GT_CHECK(dialog, "activeModalWidget is NULL");

    if (radioButNum==1){
        QRadioButton* allColumnsOfGaps = dialog->findChild<QRadioButton*>("allRadioButton");
        GTRadioButton::click(os, allColumnsOfGaps);
    }
    else{
        QRadioButton* withNumberOfGaps = dialog->findChild<QRadioButton*>("absoluteRadioButton");
        GTRadioButton::click(os, withNumberOfGaps);
    }
    QAbstractButton *removeButton = dialog->findChild<QAbstractButton*>("deleteButton");
    GTWidget::click(os, removeButton);
}
#undef GT_METHOD_NAME
#undef GT_CLASS_NAME

#define GT_CLASS_NAME "GTUtilsDialog::RemoveGapColsDialogFiller"
#define GT_METHOD_NAME "run"
void RemoveGapColsDialogFiller::run() {

    QWidget* dialog = QApplication::activeModalWidget();
    GT_CHECK(dialog, "activeModalWidget is NULL");

    QRadioButton* radio = dialog->findChild<QRadioButton*>(radioMap[button]);
    GTRadioButton::click(os, radio);

    if (button==Number){
        QSpinBox* box = dialog->findChild<QSpinBox*>("absoluteSpinBox");
        GTSpinBox::setValue(os, box, spinValue);
    }else if (button==Percent){
        QSpinBox* box = dialog->findChild<QSpinBox*>("relativeSpinBox");
        GTSpinBox::setValue(os, box, spinValue);
    }

    QAbstractButton *removeButton = dialog->findChild<QAbstractButton*>("deleteButton");
    GTWidget::click(os, removeButton);
}
#undef GT_METHOD_NAME
#undef GT_CLASS_NAME
}
