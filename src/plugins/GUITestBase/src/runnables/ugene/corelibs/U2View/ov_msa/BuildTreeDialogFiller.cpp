/**
 * UGENE - Integrated Bioinformatics Tools.
 * Copyright (C) 2008-2015 UniPro <ugene@unipro.ru>
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

#include "runnables/ugene/corelibs/U2View/ov_msa/LicenseAgreemntDialogFiller.h"


#if (QT_VERSION < 0x050000) //Qt 5
#include <QtGui/QApplication>
#include <QtGui/QPushButton>
#include <QtGui/QComboBox>
#include <QtGui/QSpinBox>
#include <QtGui/QGroupBox>
#include <QtGui/QCheckBox>
#include <QtGui/QDoubleSpinBox>
#include <QtGui/QRadioButton>
#include <QtGui/QDialogButtonBox>
#else
#include <QtWidgets/QApplication>
#include <QtWidgets/QPushButton>
#include <QtWidgets/QComboBox>
#include <QtWidgets/QSpinBox>
#include <QtWidgets/QGroupBox>
#include <QtWidgets/QCheckBox>
#include <QtWidgets/QDoubleSpinBox>
#include <QtWidgets/QRadioButton>
#include <QtWidgets/QDialogButtonBox>
#endif

namespace U2 {

BuildTreeDialogFiller::BuildTreeDialogFiller(U2OpStatus &os, QString _saveTree, int _model, double _alpha, bool _displayWithMsa)
    : Filler(os, "CreatePhyTree"), saveTree(_saveTree), model(_model), replicates(0), alpha(_alpha), displayWithMsa(_displayWithMsa)
{

}

BuildTreeDialogFiller::BuildTreeDialogFiller(U2OpStatus &os, int _replicates, QString _saveTree, int _seed,
    BuildTreeDialogFiller::ConsensusType _type, double _fraction)
    : Filler(os, "CreatePhyTree"), saveTree(_saveTree), model(0), replicates(_replicates), seed(_seed), alpha(0),
    fraction(_fraction), type(_type)
{

}

BuildTreeDialogFiller::BuildTreeDialogFiller(U2OpStatus &os, CustomScenario *cs)
    : Filler(os, "CreatePhyTree", cs)
{

}

#define GT_CLASS_NAME "GTUtilsDialog::BuildTreeDialogFiller"
#define GT_METHOD_NAME "commonScenario"
void BuildTreeDialogFiller::commonScenario() {

    QWidget* dialog = QApplication::activeModalWidget();
    GT_CHECK(dialog, "activeModalWidget is NULL");

    GTUtilsDialog::waitForDialogWhichMayRunOrNot(os, new LicenseAgreemntDialogFiller(os));
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

    QDialogButtonBox* box = qobject_cast<QDialogButtonBox*>(GTWidget::findWidget(os, "buttonBox", dialog));
    GT_CHECK(box != NULL, "buttonBox is NULL");
    QPushButton* button = box->button(QDialogButtonBox::Ok);
    GT_CHECK(button !=NULL, "Ok button is NULL");
    GTWidget::click(os, button);
}
#undef GT_METHOD_NAME
#undef GT_CLASS_NAME

BuildTreeDialogFillerPhyML::BuildTreeDialogFillerPhyML(U2OpStatus &os, bool _freqOptimRadioPressed)
    : Filler(os, "CreatePhyTree"), freqOptimRadioPressed(_freqOptimRadioPressed)
{

}

#define GT_CLASS_NAME "GTUtilsDialog::BuildTreeDialogFiller"
#define GT_METHOD_NAME "run"
void BuildTreeDialogFillerPhyML::run(){
    QWidget* dialog = QApplication::activeModalWidget();
    GT_CHECK(dialog, "activeModalWidget is NULL");

    GTUtilsDialog::waitForDialogWhichMayRunOrNot(os, new LicenseAgreemntDialogFiller(os));

    QComboBox* algorithmBox = GTWidget::findExactWidget<QComboBox*>(os, "algorithmBox", dialog);
    GTComboBox::setIndexWithText(os, algorithmBox, "PhyML Maximum Likelihood");

    QRadioButton* freqOptimRadio = GTWidget::findExactWidget<QRadioButton*>(os, "freqOptimRadio");
    if(freqOptimRadioPressed){
        GTRadioButton::click(os, freqOptimRadio);
    }

    QDialogButtonBox* box = qobject_cast<QDialogButtonBox*>(GTWidget::findWidget(os, "buttonBox", dialog));
    GT_CHECK(box != NULL, "buttonBox is NULL");
    QPushButton* button = box->button(QDialogButtonBox::Ok);
    GT_CHECK(button !=NULL, "cancel button is NULL");
    GTWidget::click(os, button);
}
#undef GT_METHOD_NAME
#undef GT_CLASS_NAME

}
