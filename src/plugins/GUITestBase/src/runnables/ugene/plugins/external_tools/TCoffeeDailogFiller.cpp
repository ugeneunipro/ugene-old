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

#include "TCoffeeDailogFiller.h"

#if (QT_VERSION < 0x050000) //Qt 5
#include <QtGui/QApplication>
#include <QtGui/QDialogButtonBox>
#else
#include <QtWidgets/QApplication>
#include <QtWidgets/QDialogButtonBox>
#endif

#include "api/GTWidget.h"
#include "api/GTCheckBox.h"
#include "api/GTSpinBox.h"

namespace U2{

#define GT_CLASS_NAME "GTUtilsDialog::DotPlotFiller"
#define GT_METHOD_NAME "run"

TCoffeeDailogFiller::TCoffeeDailogFiller(U2OpStatus &os, int gapOpen, int gapExt, int numOfIters) :
    Filler(os, "TCoffeeSupportRunDialog"),
    gapOpen(gapOpen),
    gapExt(gapExt),
    numOfIters(numOfIters)
{

}

void TCoffeeDailogFiller::run(){
    QWidget* dialog = QApplication::activeModalWidget();
    GT_CHECK(dialog, "activeModalWidget is NULL");

    if (gapOpen!=INT_MAX){
        QCheckBox* gapOpenCheckBox = qobject_cast<QCheckBox*>(GTWidget::findWidget(os, "gapOpenCheckBox", dialog));
        GTCheckBox::setChecked(os,gapOpenCheckBox,true);

        QSpinBox* gapOpenSpinBox = qobject_cast<QSpinBox*>(GTWidget::findWidget(os, "gapOpenSpinBox", dialog));
        GTSpinBox::setValue(os, gapOpenSpinBox, gapOpen);
    }

    if (gapExt!=INT_MAX){
        QCheckBox* gapExtCheckBox = qobject_cast<QCheckBox*>(GTWidget::findWidget(os, "gapExtCheckBox", dialog));
        GTCheckBox::setChecked(os,gapExtCheckBox,true);

        QSpinBox* gapExtSpinBox = qobject_cast<QSpinBox*>(GTWidget::findWidget(os, "gapExtSpinBox", dialog));
        GTSpinBox::setValue(os, gapExtSpinBox, gapExt);
    }

    if (numOfIters!=INT_MAX){
        QCheckBox* maxNumberIterRefinementCheckBox = qobject_cast<QCheckBox*>(GTWidget::findWidget(os, "maxNumberIterRefinementCheckBox", dialog));
        GTCheckBox::setChecked(os,maxNumberIterRefinementCheckBox, true);

        QSpinBox* maxNumberIterRefinementSpinBox = qobject_cast<QSpinBox*>(GTWidget::findWidget(os, "maxNumberIterRefinementSpinBox", dialog));
        GTSpinBox::setValue(os, maxNumberIterRefinementSpinBox, numOfIters);
    }

    GTUtilsDialog::clickButtonBox(os, dialog, QDialogButtonBox::Ok);
}

#undef GT_METHOD_NAME
#undef GT_CLASS_NAME
}
