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

#include "BuildDotPlotDialogFiller.h"
#include "api/GTWidget.h"
#include "api/GTSpinBox.h"
#include "api/GTCheckBox.h"
#include "api/GTLineEdit.h"

#if (QT_VERSION < 0x050000) //Qt 5
#include <QtGui/QApplication>
#include <QtGui/QPushButton>
#include <QtGui/QDialogButtonBox>
#else
#include <QtWidgets/QApplication>
#include <QtWidgets/QPushButton>
#include <QtWidgets/QDialogButtonBox>
#endif

namespace U2 {

#define GT_CLASS_NAME "GTUtilsDialog::DotPlotFiller"
#define GT_METHOD_NAME "run"
void BuildDotPlotFiller::run() {

    QWidget* dialog = QApplication::activeModalWidget();
    GT_CHECK(dialog, "activeModalWidget is NULL");

    GTCheckBox::setChecked(os, dialog->findChild<QCheckBox*>("oneSequenceCheckBox"), oneSequenceBoxChecked);
    GTLineEdit::setText(os, dialog->findChild<QLineEdit*>("firstFileEdit"), firstFileEdit);

    GTCheckBox::setChecked(os, dialog->findChild<QCheckBox*>("mergeFirstCheckBox"), mergeFirstBoxChecked);
    if (mergeFirstBoxChecked){
        GTSpinBox::setValue(os, dialog->findChild<QSpinBox*>("gapFirst"), gapFirstVal);
    }

    if (!oneSequenceBoxChecked) {
        GTLineEdit::setText(os, dialog->findChild<QLineEdit*>("secondFileEdit"), secondFileEdit);

        GTCheckBox::setChecked(os, dialog->findChild<QCheckBox*>("mergeSecondCheckBox"), mergeSecondBoxChecked);
        if(mergeSecondBoxChecked){
            GTSpinBox::setValue(os, dialog->findChild<QSpinBox*>("gapSecond"), gapSecondVal);
        }
    }

    QDialogButtonBox* box = qobject_cast<QDialogButtonBox*>(GTWidget::findWidget(os, "buttonBox", dialog));
    QPushButton* button = box->button(QDialogButtonBox::Ok);
    GT_CHECK(button !=NULL, "ok button is NULL");
    GTWidget::click(os, button);
    if (cancel) {
        GTWidget::click(os, GTWidget::findWidget(os, "cancelButton", dialog));
    }
}
#undef GT_METHOD_NAME
#undef GT_CLASS_NAME

}
