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

#include <QtCore/qglobal.h>
#if (QT_VERSION < 0x050000) //Qt 5
#include <QtGui/QApplication>
#include <QtGui/QDialogButtonBox>
#else
#include <QtWidgets/QApplication>
#include <QtWidgets/QDialogButtonBox>
#endif

#include "CreateObjectRelationDialogFiller.h"
#include "api/GTWidget.h"

namespace U2 {

#define GT_CLASS_NAME "GTUtilsDialog::FindRepeatsDialogFiller"
#define GT_METHOD_NAME "run"

CreateObjectRelationDialogFiller::CreateObjectRelationDialogFiller(U2OpStatus& os) :
    Filler(os, "CreateObjectRelationDialog")
{
}

void CreateObjectRelationDialogFiller::run() {
    QWidget* dialog = QApplication::activeModalWidget();
    GT_CHECK(NULL != dialog, "activeModalWidget is NULL");

    QDialogButtonBox *buttonBox = qobject_cast<QDialogButtonBox*>(GTWidget::findWidget(os, "buttonBox", dialog));
    GT_CHECK(NULL != buttonBox, "buttonBox is NULL");
    QAbstractButton *okButton = buttonBox->button(QDialogButtonBox::Ok);
    GT_CHECK(NULL != okButton, "okButton is NULL");
    GTWidget::click(os, okButton);

    GTGlobals::sleep(200);
}

#undef GT_METHOD_NAME
#undef GT_CLASS_NAME

}   // namespace U2
