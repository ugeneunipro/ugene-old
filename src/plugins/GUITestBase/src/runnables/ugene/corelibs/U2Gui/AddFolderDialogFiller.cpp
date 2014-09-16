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
#else
#include <QtWidgets/QApplication>
#endif

#include "api/GTKeyboardDriver.h"
#include "api/GTLineEdit.h"
#include "api/GTWidget.h"

#include "AddFolderDialogFiller.h"

namespace U2 {

AddFolderDialogFiller::AddFolderDialogFiller(U2OpStatus &os, const QString &folderName, GTGlobals::UseMethod acceptMethod)
    : Filler(os, "FolderNameDialog"), folderName(folderName), acceptMethod(acceptMethod)
{

}

#define GT_CLASS_NAME "U2::AddFolderDialogFiller"
#define GT_METHOD_NAME "run"

void AddFolderDialogFiller::run() {
    QWidget *dialog = QApplication::activeModalWidget();
    GT_CHECK(dialog, "active modal widget is invalid");

    QLineEdit *nameEdit = qobject_cast<QLineEdit*>(GTWidget::findWidget(os, "nameEdit", dialog));
    GT_CHECK(nameEdit, "Folder name line edit is invalid");
    GTLineEdit::setText(os, nameEdit, folderName);

    switch (acceptMethod) {
    case GTGlobals::UseMouse :
        GTWidget::click(os, GTWidget::findButtonByText(os, "OK", dialog));
        break;
    case GTGlobals::UseKey :
    case GTGlobals::UseKeyBoard :
        GTKeyboardDriver::keyClick(os, GTKeyboardDriver::key["enter"]);
        break;
    }
}

#undef GT_METHOD_NAME
#undef GT_CLASS_NAME

} // namespace U2
