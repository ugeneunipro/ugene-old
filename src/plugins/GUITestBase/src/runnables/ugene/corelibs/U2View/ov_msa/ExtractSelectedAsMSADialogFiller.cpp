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

#include "ExtractSelectedAsMSADialogFiller.h"
#include "api/GTComboBox.h"
#include "api/GTWidget.h"
#include "api/GTSpinBox.h"
#include "api/GTLineEdit.h"
#include "api/GTCheckBox.h"
#include "api/GTMouseDriver.h"
#include "api/GTKeyboardDriver.h"


#include <QComboBox>
#include <QSpinBox>
#include <QApplication>
#include <QCheckBox>
#include <QTableWidget>


namespace U2 {

#define GT_CLASS_NAME "GTUtilsDialog::ExtractSelectedAsMSADialogFiller"
#define GT_METHOD_NAME "run"
void ExtractSelectedAsMSADialogFiller::run() {

    QWidget* dialog = QApplication::activeModalWidget();
    GT_CHECK(dialog, "activeModalWidget is NULL");

    if(from){
        QSpinBox *fromSpin=dialog->findChild<QSpinBox*>("startPosBox");
        GT_CHECK(fromSpin!=NULL, "fromSpin is NULL")
        GTSpinBox::setValue(os,fromSpin,from,GTGlobals::UseKey);
    }

    if(to){
        QSpinBox *toSpin=dialog->findChild<QSpinBox*>("endPosBox");
        GT_CHECK(toSpin!=NULL, "toSpin is NULL")
        GTSpinBox::setValue(os,toSpin,to,GTGlobals::UseKey);
    }

    QLineEdit *filepathEdit=dialog->findChild<QLineEdit*>("filepathEdit");
    GT_CHECK(filepathEdit!=NULL, "filepathEdit is NULL");
    GTLineEdit::setText(os,filepathEdit,filepath);
    GTGlobals::sleep(300);

    QWidget *noneButton = dialog->findChild<QWidget*>("noneButton");
    GT_CHECK(noneButton!=NULL, "noneButton is NULL");
    GTWidget::click(os,noneButton);


    if(invertButtonPress){
        GTGlobals::sleep(300);
        QWidget *invertButton = dialog->findChild<QWidget*>("invertButton");
        GT_CHECK(invertButton!=NULL, "invertButton is NULL");
        GTWidget::click(os,invertButton);
    }

    if(allButtonPress){
        GTGlobals::sleep(300);
        QWidget *allButton = dialog->findChild<QWidget*>("allButton");
        GT_CHECK(allButton!=NULL, "allButton is NULL");
        GTWidget::click(os,allButton);
    }

    if(noneButtonPress){
        GTGlobals::sleep(300);
        QWidget *noneButton = dialog->findChild<QWidget*>("noneButton");
        GT_CHECK(noneButton!=NULL, "noneButton is NULL");
        GTWidget::click(os,noneButton);
    }

    if(addToProj){
        GTGlobals::sleep(300);
        QCheckBox *addToProjCheck = dialog->findChild<QCheckBox*>("addToProjBox");
        GT_CHECK(addToProjCheck!=NULL, "addToProjBox is NULL");
        GTCheckBox::setChecked(os,addToProjCheck,addToProj);
    }

    QTableWidget *table=dialog->findChild<QTableWidget*>("sequencesTableWidget");
    GT_CHECK(table!=NULL, "tableWidget is NULL");
    QPoint p=table->geometry().topRight();
    p.setX(p.x()-2);
    p.setY(p.y()+2);
    p=dialog->mapToGlobal(p);

    GTMouseDriver::moveTo(os,p);
    GTMouseDriver::click(os);
    for(int i=0; i<table->rowCount() ;i++){
        foreach(QString s, list){
            QCheckBox *box = qobject_cast<QCheckBox*>(table->cellWidget(i,0));
            if (s==box->text()){
                GT_CHECK(box->isEnabled(), QString("%1 box is disabled").arg(box->text()));
                box->setChecked(true);
            }
        }
    }

    GTGlobals::sleep();
    GTWidget::click(os, GTWidget::findWidget(os,"okButton"));


}
#undef GT_METHOD_NAME
#undef GT_CLASS_NAME

}
