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

#include "ExportMSA2MSADialogFiller.h"
#include "api/GTWidget.h"
#include "api/GTLineEdit.h"
#include "api/GTRadioButton.h"
#include "api/GTComboBox.h"

#include <QtCore/QDir>
#if (QT_VERSION < 0x050000) //Qt 5
#include <QtGui/QApplication>
#include <QtGui/QPushButton>
#include <QtGui/QDialogButtonBox>
#include <QtGui/QComboBox>
#else
#include <QtWidgets/QApplication>
#include <QtWidgets/QDialogButtonBox>
#include <QtWidgets/QPushButton>
#include <QtWidgets/QComboBox>
#endif

namespace U2 {

#define GT_CLASS_NAME "GTUtilsDialog::ExportToSequenceFormatFiller"

ExportMSA2MSADialogFiller::ExportMSA2MSADialogFiller(U2OpStatus &_os, int _formatVal, QString _path) : Filler(_os, "U2__ExportMSA2MSADialog"),
    formatVal(_formatVal),
    path(_path){}

#define GT_METHOD_NAME "run"
void ExportMSA2MSADialogFiller::run()
{
    QWidget *dialog = QApplication::activeModalWidget();
    GT_CHECK(dialog != NULL, "dialog not found");

    if(!path.isEmpty()){
        QLineEdit* fileNameEdit = dialog->findChild<QLineEdit*>("fileNameEdit");
        GTLineEdit::setText(os, fileNameEdit,path);
    }
    if(formatVal>=0){
        QComboBox* formatCombo = dialog->findChild<QComboBox*>("formatCombo");
        GTComboBox::setCurrentIndex(os, formatCombo, formatVal);
    }

    QDialogButtonBox* box = qobject_cast<QDialogButtonBox*>(GTWidget::findWidget(os, "buttonBox", dialog));
    GT_CHECK(box != NULL, "buttonBox is NULL");
    QPushButton* button = box->button(QDialogButtonBox::Ok);
    GT_CHECK(button !=NULL, "ok button is NULL");
    GTWidget::click(os, button);
}
#undef GT_METHOD_NAME
#undef GT_CLASS_NAME

}
