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

#include "ImportBAMFileDialogFiller.h"
#include "api/GTWidget.h"
#include "api/GTSpinBox.h"
#include "api/GTCheckBox.h"
#include "api/GTLineEdit.h"
#include "api/GTFileDialog.h"

#include <QtGui/QApplication>
#include <QtGui/QDialogButtonBox>
#include <QtGui/QPushButton>

namespace U2 {

#define GT_CLASS_NAME "GTUtilsDialog::ImportBAMFileFiller"
#define GT_METHOD_NAME "run"
void ImportBAMFileFiller::run() {

    GTGlobals::sleep(500);
    QWidget* dialog = QApplication::activeModalWidget();
    GT_CHECK(dialog, "activeModalWidget is NULL");

    if (!databasePath.isEmpty())
        {
        GTFileDialogUtils *ob = new GTFileDialogUtils(os, databasePath, databaseFileName);
        GTUtilsDialog::waitForDialog(os, ob);
        GTWidget::click(os, GTWidget::findWidget(os,"refUrlButton",dialog));
        }

    if (!destinationUrl.isEmpty()){
        QLineEdit* destinationUrlEdit = qobject_cast<QLineEdit*>(GTWidget::findWidget(os, "destinationUrlEdit", dialog));
        GT_CHECK(destinationUrlEdit, "destinationUrlEdit not found");
        GTLineEdit::setText(os, destinationUrlEdit, destinationUrl);
    }

    QDialogButtonBox* box = qobject_cast<QDialogButtonBox*>(GTWidget::findWidget(os, "buttonBox", dialog));
    QPushButton* button = box->button(QDialogButtonBox::Ok);
    GT_CHECK(button !=NULL, "ok button is NULL");
    GTWidget::click(os, button);
    }

#undef GT_METHOD_NAME
#undef GT_CLASS_NAME

}
