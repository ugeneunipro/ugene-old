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

#include "BuildTreeDialogFiller.h"
#include "api/GTComboBox.h"
#include "api/GTWidget.h"
#include "api/GTSpinBox.h"
#include "api/GTLineEdit.h"
#include "api/GTCheckBox.h"
#include "api/GTDoubleSpinBox.h"
#include "api/GTRadioButton.h"


#include <QComboBox>
#include <QSpinBox>
#include <QApplication>
#include <QGroupBox>
#include <QCheckBox>
#include <QDoubleSpinBox>
#include <QRadioButton>

namespace U2 {

#define GT_CLASS_NAME "GTUtilsDialog::BuildTreeDialogFiller"
#define GT_METHOD_NAME "run"
void BuildTreeDialogFiller::run() {

    QWidget* dialog = QApplication::activeModalWidget();
    GT_CHECK(dialog, "activeModalWidget is NULL");

    if(saveTree!="default"){
        QLineEdit* saveLineEdit = qobject_cast<QLineEdit*>(GTWidget::findWidget(os,"fileNameEdit"));
        GTLineEdit::setText(os,saveLineEdit, saveTree);
    }

    QRadioButton* displayInNewWindow = qobject_cast<QRadioButton*>(GTWidget::findWidget(os,"createNewView"));
    if(!displayWithMsa) {
        GTRadioButton::click(os, displayInNewWindow);
    }

    if(model){
        QComboBox* modelBox = qobject_cast<QComboBox*>(GTWidget::findWidget(os,"modelBox"));
        GTComboBox::setCurrentIndex(os,modelBox,model);
    }

    QCheckBox* gammaBox = qobject_cast<QCheckBox*>(GTWidget::findWidget(os,"gammaCheckBox"));
    if(alpha){
        GTCheckBox::setChecked(os,gammaBox,true);

        QDoubleSpinBox* alphaSpinBox = qobject_cast<QDoubleSpinBox*>(GTWidget::findWidget(os,"alphaSpinBox"));
        GTDoubleSpinbox::setValue(os, alphaSpinBox,alpha,GTGlobals::UseKeyBoard);
    }
    else{
        GTCheckBox::setChecked(os,gammaBox,false);
    }

    if (replicates){
        QGroupBox *BootstrapGroupBox = dialog->findChild<QGroupBox*>("BootstrapGroupBox");
        BootstrapGroupBox->setChecked(true);

        QSpinBox* repsSpinBox = dialog->findChild<QSpinBox*>("repsSpinBox");
        GTSpinBox::setValue(os,repsSpinBox,replicates, GTGlobals::UseKeyBoard);

        QSpinBox* seedSpinBox = dialog->findChild<QSpinBox*>("seedSpinBox");
        GTSpinBox::setValue(os,seedSpinBox,seed, GTGlobals::UseKeyBoard);

        QComboBox* ConsModeComboBox = dialog->findChild<QComboBox*>("ConsModeComboBox");
        GTComboBox::setCurrentIndex(os,ConsModeComboBox,type);

        if(type == M1){
            QDoubleSpinBox* FractionSpinBox = dialog->findChild<QDoubleSpinBox*>("FractionSpinBox");
            GTDoubleSpinbox::setValue(os,FractionSpinBox,fraction,GTGlobals::UseKeyBoard);
        }
    }

    GTWidget::click(os, GTWidget::findWidget(os,"okButton"));


}
}
#undef GT_METHOD_NAME
#undef GT_CLASS_NAME
