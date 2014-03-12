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

#include "FindTandemsDialogFiller.h"
#include "api/GTWidget.h"
#include "api/GTTabWidget.h"

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

FindTandemsDialogFiller::FindTandemsDialogFiller( U2OpStatus &_os, const QString & _resultFilesPath) 
    : Filler(_os, "FindTandemsDialog"), button(Start), resultAnnotationFilesPath(_resultFilesPath){
}

#define GT_CLASS_NAME "GTUtilsDialog::FindTandemsDialogFiller"
#define GT_METHOD_NAME "run"

void FindTandemsDialogFiller::run(){
    QWidget* dialog = QApplication::activeModalWidget();
    GT_CHECK(dialog, "activeModalWidget is NULL");

    if (button == Cancel) {
        QAbstractButton *cancelButton = qobject_cast<QAbstractButton*>(GTWidget::findWidget(os, "cancelButton", dialog));
        GTWidget::click(os, cancelButton);
        return;
    }

    /*
    QTabWidget *tabWidget = qobject_cast<QTabWidget*>(GTWidget::findWidget(os, "tabWidget", dialog));
    GTTabWidget::setCurrentIndex(os, tabWidget, 0);
    */

    QLineEdit *resultLocationEdit = qobject_cast<QLineEdit *>(GTWidget::findWidget(os, "locationEdit", dialog));
    resultLocationEdit->setText(resultAnnotationFilesPath);

    //GTTabWidget::setCurrentIndex(os, tabWidget, 1);

    QDialogButtonBox* box = qobject_cast<QDialogButtonBox*>(GTWidget::findWidget(os, "buttonBox", dialog));
    GT_CHECK(box != NULL, "buttonBox is NULL");
    QPushButton* button = box->button(QDialogButtonBox::Ok);
    GT_CHECK(button !=NULL, "ok button is NULL");
    GTWidget::click(os, button);

    GTGlobals::sleep();
}

#undef GT_METHOD_NAME
#undef GT_CLASS_NAME

}
