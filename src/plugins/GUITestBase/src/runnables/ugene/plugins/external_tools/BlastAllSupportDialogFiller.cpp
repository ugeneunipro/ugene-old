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

#include <QtGui/QApplication>
#include <QtGui/QComboBox>

#include "api/GTComboBox.h"
#include "api/GTFileDialog.h"
#include "api/GTWidget.h"

#include "BlastAllSupportDialogFiller.h"

namespace U2 {

#define GT_CLASS_NAME "GTUtilsDialog::BlastAllSupportDialogFiller"

BlastAllSupportDialogFiller::BlastAllSupportDialogFiller(const Parameters &parameters, U2OpStatus &os)
: Filler(os, "BlastAllSupportDialog"), parameters(parameters)
{

}

#define GT_METHOD_NAME "run"
void BlastAllSupportDialogFiller::run() {
    QWidget* dialog = QApplication::activeModalWidget();
    GT_CHECK(dialog, "activeModalWidget is NULL");

    if (!parameters.runBlast) {
        GTUtilsDialog::clickButtonBox(os, dialog, QDialogButtonBox::Cancel);
        return;
    }

    QComboBox *programName = qobject_cast<QComboBox*>(GTWidget::findWidget(os, "programName", dialog));
    GT_CHECK(programName, "programName is NULL");
    GTComboBox::setIndexWithText(os, programName, parameters.programNameText);

    if (!parameters.dbPath.isEmpty()) {
        GTUtilsDialog::waitForDialog(os, new GTFileDialogUtils(os, parameters.dbPath));
        GTWidget::click(os, GTWidget::findWidget(os,"selectDatabasePushButton"));
    }

    GTUtilsDialog::clickButtonBox(os, dialog, QDialogButtonBox::Ok);
}
#undef GT_METHOD_NAME

#undef GT_CLASS_NAME

}   // namespace U2
