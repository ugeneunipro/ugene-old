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
#include <QDir>

#include <primitives/GTComboBox.h>
#include <base_dialogs/GTFileDialog.h>
#include <primitives/GTLineEdit.h>
#include <primitives/GTWidget.h>

#include "SpadesGenomeAssemblyDialogFiller.h"

namespace U2 {

#define GT_CLASS_NAME "SpadesGenomeAssemblyDialogFiller"
#define GT_METHOD_NAME "commonScenario"
void SpadesGenomeAssemblyDialogFiller::commonScenario(){
    QWidget* dialog = QApplication::activeModalWidget();
    GT_CHECK(dialog, "activeModalWidget is NULL");

    GTUtilsDialog::waitForDialog(os, new GTFileDialogUtils(os, output, GTGlobals::UseMouse, GTFileDialogUtils::Choose));
    GTWidget::click(os, GTWidget::findWidget(os, "setResultDirNameButton", dialog));

    QComboBox* libraryComboBox = GTWidget::findExactWidget<QComboBox*>(os, "libraryComboBox", dialog);
    GTComboBox::setIndexWithText(os, libraryComboBox, library);

    foreach (QString s, leftReads) {
        GTUtilsDialog::waitForDialog(os, new GTFileDialogUtils(os, s));
        GTWidget::click(os, GTWidget::findWidget(os, "addLeftButton", dialog));
    }

    foreach (QString s, rightReads) {
        GTUtilsDialog::waitForDialog(os, new GTFileDialogUtils(os, s));
        GTWidget::click(os, GTWidget::findWidget(os, "addFightButton", dialog));
    }

    GTUtilsDialog::clickButtonBox(os, dialog, QDialogButtonBox::Ok);
}
#undef GT_METHOD_NAME
#undef GT_CLASS_NAME

}
