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
#include "DistanceMatrixDialogFiller.h"

#include "api/GTWidget.h"
#include "api/GTLineEdit.h"
#include "api/GTRadioButton.h"
#include "api/GTComboBox.h"
#include "api/GTCheckBox.h"
#include "api/GTGlobals.h"

#include <QtCore/QDir>
#include <QtGui/QApplication>
#include <QtGui/QDialogButtonBox>
#include <QtGui/QPushButton>
#include <QtGui/QGroupBox>
namespace U2{

#define GT_CLASS_NAME "GTUtilsDialog::DistanceMatrixDialogFiller"
#define GT_METHOD_NAME "run"
void DistanceMatrixDialogFiller::run(){

    QWidget* dialog = QApplication::activeModalWidget();
    GT_CHECK(dialog!=NULL, "dialog not found");


    QComboBox* algoCombo = dialog->findChild<QComboBox*>("algoCombo");
    if(hamming){
        GTComboBox::setCurrentIndex(os,algoCombo,0);
    }
    else{
        GTComboBox::setCurrentIndex(os,algoCombo,1);
    }

    if(counts){
        QRadioButton* countsRB = dialog->findChild<QRadioButton*>("countsRB");
        GTRadioButton::click(os,countsRB);
    }
    else{
        QRadioButton* percentsRB = dialog->findChild<QRadioButton*>("percentsRB");
        GTRadioButton::click(os,percentsRB);
    }

    if(saveToFile){
        QGroupBox* saveBox = dialog->findChild<QGroupBox*>("saveBox");
        saveBox->setChecked(true);

        QLineEdit* fileEdit = dialog->findChild<QLineEdit*>("fileEdit");
        GTLineEdit::setText(os, fileEdit,path);

        if(format == HTML){
            QRadioButton* htmlRB = dialog->findChild<QRadioButton*>("htmlRB");
            GTRadioButton::click(os,htmlRB);
        }
        else{
            QRadioButton* csvRB = dialog->findChild<QRadioButton*>("csvRB");
            GTRadioButton::click(os,csvRB);
        }
    }
    QCheckBox* checkBox = dialog->findChild<QCheckBox*>("checkBox");
    GTCheckBox::setChecked(os,checkBox,excludeGaps);


    GTWidget::click(os, GTWidget::findWidget(os,"okButton"));
}
#undef GT_METHOD_NAME
#undef GT_CLASS_NAME
}
