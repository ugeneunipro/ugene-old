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

#include <QApplication>
#include <QGroupBox>

#include <primitives/GTDoubleSpinBox.h>
#include <primitives/GTSpinBox.h>
#include <primitives/GTWidget.h>

#include <base_dialogs/ColorDialogFiller.h>

#include "GraphSettingsDialogFiller.h"

namespace U2 {

#define GT_CLASS_NAME "GTUtilsDialog::GraphSettingsDialogFiller"
#define GT_METHOD_NAME "run"

void GraphSettingsDialogFiller::commonScenario(){
    QWidget* dialog = QApplication::activeModalWidget();
    GT_CHECK(dialog, "activeModalWidget is NULL");

    if(window != -1){
        QSpinBox* windowEdit = GTWidget::findExactWidget<QSpinBox*>(os, "windowEdit", dialog);
        GTSpinBox::setValue(os, windowEdit, window, GTGlobals::UseKeyBoard);
    }

    if(steps != -1){
        QSpinBox* stepsPerWindowEdit = GTWidget::findExactWidget<QSpinBox*>(os, "stepsPerWindowEdit", dialog);
        GTSpinBox::setValue(os, stepsPerWindowEdit, steps, GTGlobals::UseKeyBoard);
    }

    if(cutoff_max != 0 || cutoff_min != 0){
        QGroupBox* minmaxGroup = GTWidget::findExactWidget<QGroupBox*>(os, "minmaxGroup", dialog);
        minmaxGroup->setChecked(true);
    }

    if(cutoff_min != 0){
        QDoubleSpinBox* minBox = GTWidget::findExactWidget<QDoubleSpinBox*>(os, "minBox", dialog);
        GTDoubleSpinbox::setValue(os, minBox, cutoff_min, GTGlobals::UseKeyBoard);
    }

    if(cutoff_max != 0){
        QDoubleSpinBox* maxBox = GTWidget::findExactWidget<QDoubleSpinBox*>(os, "maxBox", dialog);
        GTDoubleSpinbox::setValue(os, maxBox, cutoff_max, GTGlobals::UseKeyBoard);
    }

    if(r != -1 && g != -1 && b != -1){
        GTUtilsDialog::waitForDialog(os, new ColorDialogFiller(os, r, g, b));
        GTWidget::click(os, GTWidget::findWidget(os, "Default color", dialog));
    }

    GTGlobals::sleep(200);
    GTUtilsDialog::clickButtonBox(os, QDialogButtonBox::Ok);
}

#undef GT_METHOD_NAME
#undef GT_CLASS_NAME

}
