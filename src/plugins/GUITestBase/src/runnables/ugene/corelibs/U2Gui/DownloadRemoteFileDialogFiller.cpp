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

#include "DownloadRemoteFileDialogFiller.h"
#include "api/GTWidget.h"
#include "api/GTLineEdit.h"
#include "api/GTComboBox.h"

#include <QtGui/QApplication>
#include <QtGui/QAbstractButton>
#include <QtGui/QPushButton>
#include <QtGui/QDialogButtonBox>

namespace U2 {

#define GT_CLASS_NAME "GTUtilsDialog::RemoteDBDialogFiller"
#define GT_METHOD_NAME "run"
void RemoteDBDialogFiller::run() {

    QWidget* dialog = QApplication::activeModalWidget();
    GT_CHECK(dialog, "activeModalWidget is NULL");

    if (!resID.isEmpty()){
        QLineEdit *idLineEdit = qobject_cast<QLineEdit*>(GTWidget::findWidget(os, "idLineEdit", dialog));
        GTLineEdit::setText(os, idLineEdit, resID);
    }

    QLineEdit *saveFilenameLineEdit = qobject_cast<QLineEdit*>(GTWidget::findWidget(os, "saveFilenameLineEdit", dialog));
    if(!saveDirPath.isEmpty()){
        GTLineEdit::setText(os, saveFilenameLineEdit, saveDirPath);
    }

    QComboBox *databasesBox = qobject_cast<QComboBox*>(GTWidget::findWidget(os, "databasesBox", dialog));
    GTComboBox::setCurrentIndex(os, databasesBox, DBItemNum);

    if (outFormatVal!=-1){
        QComboBox* formatBox = qobject_cast<QComboBox*>(GTWidget::findWidget(os, "formatBox"));
        GTComboBox::setCurrentIndex(os, formatBox, outFormatVal);
    }
    if (pressCancel) {
        QDialogButtonBox* box = qobject_cast<QDialogButtonBox*>(GTWidget::findWidget(os, "buttonBox", dialog));
        GT_CHECK(box != NULL, "buttonBox is NULL");
        QPushButton* button = box->button(QDialogButtonBox::Cancel);
        GT_CHECK(button !=NULL, "cancel button is NULL");
        GTWidget::click(os, button);
    }
    else {
        QDialogButtonBox* box = qobject_cast<QDialogButtonBox*>(GTWidget::findWidget(os, "buttonBox", dialog));
        GT_CHECK(box != NULL, "buttonBox is NULL");
        QPushButton* button = box->button(QDialogButtonBox::Ok);
        GT_CHECK(button !=NULL, "cancel button is NULL");
        GTWidget::click(os, button);
    }
}
#undef GT_METHOD_NAME
#undef GT_CLASS_NAME

}
