/**
 * UGENE - Integrated Bioinformatics Tools.
 * Copyright (C) 2008-2012 UniPro <ugene@unipro.ru>
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

#include "AlignShortReadsDialogFiller.h"
#include "api/GTWidget.h"
#include "api/GTSpinBox.h"
#include "api/GTCheckBox.h"
#include "api/GTLineEdit.h"
#include "runnables/qt/MessageBoxFiller.h"
#include <QtGui/QApplication>

namespace U2 {

#define GT_CLASS_NAME "GTUtilsDialog::AlignShortReadsFiller"
#define GT_METHOD_NAME "run"
void AlignShortReadsFiller::run() {

    QWidget* dialog = QApplication::activeModalWidget();
    GT_CHECK(dialog, "activeModalWidget is NULL");

    GTFileDialogUtils *ob = new GTFileDialogUtils(os, path, fileName);
    GTUtilsDialog::waitForDialog(os, ob);
    GTWidget::click(os, GTWidget::findWidget(os, "addRefButton",dialog));

    GTFileDialogUtils *ob1 = new GTFileDialogUtils(os, path1, fileName1);
    GTUtilsDialog::waitForDialog(os, ob1);
    GTWidget::click(os, GTWidget::findWidget(os, "addShortreadsButton", dialog));

    GTCheckBox::setChecked(os,dialog->findChild<QCheckBox*>("samBox"),true);
    GTGlobals::sleep(500);
    GTWidget::click(os, GTWidget::findWidget(os, "assembleyButton", dialog));

}
#undef GT_METHOD_NAME
#undef GT_CLASS_NAME

}
