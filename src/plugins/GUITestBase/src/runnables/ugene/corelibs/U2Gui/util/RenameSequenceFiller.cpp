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

#include "RenameSequenceFiller.h"
#include "api/GTLineEdit.h"
#include "api/GTWidget.h"
#include "api/GTClipboard.h"
#include "api/GTKeyboardDriver.h"
#include "api/GTMouseDriver.h"

#include <QtGui/QApplication>
#include <QtGui/QPushButton>

namespace U2 {

#define GT_CLASS_NAME "GTUtilsDialog::RenamesequenceFiller"
#define GT_METHOD_NAME "run"

void RenameSequenceFiller::run(){
    QWidget *dialog = QApplication::activeModalWidget();
    GT_CHECK(dialog != NULL, "dialog not found");
    GT_CHECK(dialog->windowTitle() == "Rename", "dialog not found");

    QLineEdit *nameEdit = dialog->findChild<QLineEdit*>();
    GT_CHECK(nameEdit != NULL, "Line edit not found");

    if(!oldName.isEmpty()){
        QString actualText = nameEdit->text();
        GT_CHECK(oldName == actualText, "edited sequence name not match with expected");
    }
    
    if(oldName != newName){ //if filler used only for checking sequence name
        GTLineEdit::setText(os, nameEdit, newName);
    }
 
    GTGlobals::sleep(1000);

#ifndef Q_OS_MAC
    //instead clicking OK we will send 'Enter'
    GTKeyboardDriver::keyClick(os, GTKeyboardDriver::key["enter"]);
#else
    //but on Mac we will definitly click OK
    QList<QPushButton*> list = dialog->findChildren<QPushButton*>();
    foreach(QPushButton* but,list){
        if (but->text().contains("OK"))
                GTWidget::click(os,but);
    }
#endif
}

#undef GT_METHOD_NAME
#undef GT_CLASS_NAME

}
