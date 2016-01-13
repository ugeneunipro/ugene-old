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

#include "FindTandemsDialogFiller.h"
#include <primitives/GTWidget.h>
#include <primitives/GTTabWidget.h>
#include <primitives/GTLineEdit.h>

#if (QT_VERSION < 0x050000) //Qt 5
#include <QtGui/QApplication>
#include <QtGui/QAbstractButton>
#include <QtGui/QDialogButtonBox>
#include <QtGui/QPushButton>
#else
#include <QtWidgets/QApplication>
#include <QtWidgets/QAbstractButton>
#include <QtWidgets/QDialogButtonBox>
#include <QtWidgets/QPushButton>
#endif

namespace U2 {

FindTandemsDialogFiller::FindTandemsDialogFiller( HI::GUITestOpStatus &_os, const QString & _resultFilesPath)
    : Filler(_os, "FindTandemsDialog"), button(Start), resultAnnotationFilesPath(_resultFilesPath){
}

#define GT_CLASS_NAME "GTUtilsDialog::FindTandemsDialogFiller"
#define GT_METHOD_NAME "run"

FindTandemsDialogFiller::FindTandemsDialogFiller(HI::GUITestOpStatus &os, CustomScenario *scenario) :
    Filler(os, "FindTandemsDialog", scenario),
    button(Start)
{

}

void FindTandemsDialogFiller::commonScenario(){
    QWidget* dialog = QApplication::activeModalWidget();
    GT_CHECK(dialog, "activeModalWidget is NULL");

    if (button == Cancel) {
        QAbstractButton *cancelButton = qobject_cast<QAbstractButton*>(GTWidget::findWidget(os, "cancelButton", dialog));
        GTWidget::click(os, cancelButton);
        return;
    }

    QLineEdit *resultLocationEdit = qobject_cast<QLineEdit *>(GTWidget::findWidget(os, "leNewTablePath", dialog));
    GT_CHECK(resultLocationEdit, "resultLocation is NULL");
    GTLineEdit::setText(os, resultLocationEdit, resultAnnotationFilesPath);

    GTUtilsDialog::clickButtonBox(os, dialog, QDialogButtonBox::Ok);
}

#undef GT_METHOD_NAME
#undef GT_CLASS_NAME

}
