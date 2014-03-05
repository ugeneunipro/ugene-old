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

#include "CreateNewProjectWidgetFiller.h"
#include "api/GTWidget.h"
#include "api/GTLineEdit.h"

#if (QT_VERSION < 0x050000) //Qt 5
#include <QtGui/QApplication>
#include <QtGui/QPushButton>
#include <QtGui/QAbstractButton>
#include <QtGui/QDialogButtonBox>
#else
#include <QtWidgets/QApplication>
#include <QtWidgets/QPushButton>
#include <QtWidgets/QAbstractButton>
#include <QtWidgets/QDialogButtonBox>
#endif

namespace U2 {

#define GT_CLASS_NAME "GTUtilsDialog::SaveProjectAsDialogFiller"
#define GT_METHOD_NAME "run"
void SaveProjectAsDialogFiller::run() {

    GTGlobals::sleep();
    QWidget* dialog = QApplication::activeModalWidget();
    GT_CHECK(dialog, "activeModalWidget is NULL");

    QLineEdit *projectNameEdit = qobject_cast<QLineEdit*>(GTWidget::findWidget(os, "projectNameEdit", dialog));
    GTLineEdit::setText(os, projectNameEdit, projectName);

    QLineEdit *projectFolderEdit = qobject_cast<QLineEdit*>(GTWidget::findWidget(os, "projectFolderEdit", dialog));
    GTLineEdit::setText(os, projectFolderEdit, projectFolder);

    QLineEdit *projectFileEdit = qobject_cast<QLineEdit*>(GTWidget::findWidget(os, "projectFileEdit", dialog));
    GTLineEdit::setText(os, projectFileEdit, projectFile);

    QDialogButtonBox* box = qobject_cast<QDialogButtonBox*>(GTWidget::findWidget(os, "buttonBox", dialog));
    GT_CHECK(box != NULL, "buttonBox is NULL");
    QPushButton* button = box->button(QDialogButtonBox::Ok);
    GT_CHECK(button !=NULL, "cancel button is NULL");
    GTWidget::click(os, button);

    GTGlobals::sleep();
}
#undef GT_METHOD_NAME
#undef GT_CLASS_NAME

}
