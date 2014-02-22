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
#include <QtGui/QDialogButtonBox>

#include <QtGui/QApplication>

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

    QDialogButtonBox* box = qobject_cast<QDialogButtonBox*>(GTWidget::findWidget(os, "buttonBox", dialog));
    QPushButton* button = box->button(QDialogButtonBox::Ok);
    GT_CHECK(button !=NULL, "ok button is NULL");
    GTWidget::click(os, button);
}
#undef GT_METHOD_NAME
#undef GT_CLASS_NAME

}
