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


#include "WorkflowMetadialogFiller.h"
#include <primitives/GTWidget.h>
#include <primitives/GTLineEdit.h>

#if (QT_VERSION < 0x050000) //Qt 5
#include <QtGui/QApplication>
#include <QtGui/QDialogButtonBox>
#include <QtGui/QPushButton>
#else
#include <QtWidgets/QApplication>
#include <QtWidgets/QDialogButtonBox>
#include <QtWidgets/QPushButton>
#endif

namespace U2{
#define GT_CLASS_NAME "GTUtilsDialog::StartupDialogFiller"
#define GT_METHOD_NAME "commonScenario"
void WorkflowMetaDialogFiller::commonScenario(){
     QWidget* dialog = QApplication::activeModalWidget();
     GT_CHECK(dialog, "activeModalWidget is NULL");

     QLineEdit* urlEdit = qobject_cast<QLineEdit*>(GTWidget::findWidget(os, "urlEdit", dialog));
     GT_CHECK(urlEdit, "urlEdit not found");
     GTLineEdit::setText(os, urlEdit,url);

     QLineEdit* nameEdit = qobject_cast<QLineEdit*>(GTWidget::findWidget(os, "nameEdit", dialog));
     GT_CHECK(nameEdit, "urlEdit not found");
     GTLineEdit::setText(os, nameEdit,name);

     GTUtilsDialog::clickButtonBox(os, dialog, QDialogButtonBox::Ok);
}
#undef GT_METHOD_NAME
#undef GT_CLASS_NAME
}
