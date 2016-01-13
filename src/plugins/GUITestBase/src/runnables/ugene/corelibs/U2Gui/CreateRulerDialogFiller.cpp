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

#include <primitives/GTLineEdit.h>
#include <primitives/GTSpinBox.h>
#include <primitives/GTWidget.h>

#include "CreateRulerDialogFiller.h"

namespace U2 {

#define GT_CLASS_NAME "CreateRulerDialogFiller"
#define GT_METHOD_NAME "commonScenario"

void CreateRulerDialogFiller::commonScenario(){
    QWidget* dialog = QApplication::activeModalWidget();
    GT_CHECK(NULL != dialog, "activeModalWidget is NULL");

    QLineEdit* nameEdit = GTWidget::findExactWidget<QLineEdit*>(os, "nameEdit", dialog);
    GTLineEdit::setText(os, nameEdit, rulerName);

    QSpinBox* spinBox = GTWidget::findExactWidget<QSpinBox*>(os, "spinBox", dialog);
    GTSpinBox::setValue(os, spinBox, startPos, GTGlobals::UseKeyBoard);

    GTUtilsDialog::clickButtonBox(os, QDialogButtonBox::Ok);
}

#undef GT_METHOD_NAME
#undef GT_CLASS_NAME

}
