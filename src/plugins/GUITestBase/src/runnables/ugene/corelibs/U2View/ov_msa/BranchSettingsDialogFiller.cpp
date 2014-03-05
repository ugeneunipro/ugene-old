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
#include "BranchSettingsDialogFiller.h"
#if (QT_VERSION < 0x050000) //Qt 5
#include <QtGui/QApplication>
#include <QtGui/QPushButton>
#include <QtGui/QColorDialog>
#include <QtGui/QSpinBox>
#include <QtGui/QDialogButtonBox>
#else
#include <QtWidgets/QApplication>
#include <QtWidgets/QPushButton>
#include <QtWidgets/QColorDialog>
#include <QtWidgets/QSpinBox>
#include <QtWidgets/QDialogButtonBox>
#endif

#include "api/GTWidget.h"
#include "api/GTSpinBox.h"
#include "runnables/qt/ColorDialogFiller.h"

namespace U2{

#define GT_CLASS_NAME "GTUtilsDialog::BranchSettingsDialogFiller"
#define GT_METHOD_NAME "run"
void BranchSettingsDialogFiller::run(){
    QWidget* dialog = QApplication::activeModalWidget();
    GT_CHECK(dialog!=NULL, "Active modal widget not found");

    GTGlobals::sleep(500);
    GTUtilsDialog::waitForDialog(os, new ColorDialogFiller(os,0,0,255));
    QPushButton* colorButton = dialog->findChild<QPushButton*>("colorButton");
    GTWidget::click(os, colorButton);

    QSpinBox* thicknessSpinBox = dialog->findChild<QSpinBox*>("thicknessSpinBox");
    GTSpinBox::setValue(os,thicknessSpinBox,10);

    QDialogButtonBox* box = qobject_cast<QDialogButtonBox*>(GTWidget::findWidget(os, "buttonBox", dialog));
    GT_CHECK(box != NULL, "buttonBox is NULL");
    QPushButton* button = box->button(QDialogButtonBox::Ok);
    GT_CHECK(button !=NULL, "cancel button is NULL");
    GTWidget::click(os, button);

}
#undef GT_METHOD_NAME
#undef GT_CLASS_NAME
}
