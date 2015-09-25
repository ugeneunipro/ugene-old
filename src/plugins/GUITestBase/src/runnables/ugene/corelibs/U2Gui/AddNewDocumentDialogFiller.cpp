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

#include "AddNewDocumentDialogFiller.h"
#include "api/GTComboBox.h"
#include "api/GTLineEdit.h"
#include "api/GTWidget.h"

namespace U2 {

#define GT_CLASS_NAME "GTUtilsDialog::AlignShortReadsFiller"

AddNewDocumentDialogFiller::AddNewDocumentDialogFiller(U2OpStatus &os, const QString &format, const QString &path) :
    Filler(os, "AddNewDocumentDialog", scenario),
    format(format),
    path(path)
{

}

AddNewDocumentDialogFiller::AddNewDocumentDialogFiller(U2OpStatus &os, CustomScenario *scenario) :
    Filler(os, "AddNewDocumentDialog", scenario)
{

}

#define GT_METHOD_NAME "commonScenario"
void AddNewDocumentDialogFiller::commonScenario() {
    QWidget *dialog = QApplication::activeModalWidget();
    GT_CHECK(NULL != dialog, "Active modal widget is NULL");

    GTComboBox::setIndexWithText(os, GTWidget::findExactWidget<QComboBox *>(os, "documentTypeCombo", dialog), format);
    GTLineEdit::setText(os, GTWidget::findExactWidget<QLineEdit *>(os, "documentURLEdit", dialog), path);

    GTUtilsDialog::clickButtonBox(os, dialog, QDialogButtonBox::Ok);
}
#undef GT_METHOD_NAME

#undef GT_CLASS_NAME

}   // namespace U2
