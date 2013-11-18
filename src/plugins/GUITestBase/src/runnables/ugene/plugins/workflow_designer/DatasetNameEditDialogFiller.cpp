/**
 * UGENE - Integrated Bioinformatics Tools.
 * Copyright (C) 2008-2013 UniPro <ugene@unipro.ru>
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

#include "DatasetNameEditDialogFiller.h"
#include "api/GTLineEdit.h"
#include "api/GTKeyboardDriver.h"

#include <QtGui/QApplication>
namespace U2{

#define GT_CLASS_NAME "DatasetNameEditDialogFiller"
#define GT_METHOD_NAME "run"
void DatasetNameEditDialogFiller::run(){
    QWidget* dialog = QApplication::activeModalWidget();
    GT_CHECK(dialog, "activeModalWidget is NULL");

    if(!datasetName.isEmpty()){
        QLineEdit* line = dialog->findChild<QLineEdit*>();
        GT_CHECK(line, "lineEdit not found");
        GTLineEdit::setText(os, line, datasetName);
    }
    GTKeyboardDriver::keyClick(os, GTKeyboardDriver::key["enter"]);
}
#undef GT_METHOD_NAME
#undef GT_CLASS_NAME


}
