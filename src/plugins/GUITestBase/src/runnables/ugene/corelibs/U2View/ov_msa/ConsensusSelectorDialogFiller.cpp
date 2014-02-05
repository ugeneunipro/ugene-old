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

#include "ConsensusSelectorDialogFiller.h"
#include "api/GTComboBox.h"
#include "api/GTWidget.h"
#include "api/GTSpinBox.h"

#include <QComboBox>
#include <QSpinBox>
#include <QApplication>
#include <QtGui/QToolButton>

namespace U2 {

#define GT_CLASS_NAME "GTUtilsDialog::ConsensusSelectionDialogFiller"
#define GT_METHOD_NAME "run"
void ConsensusSelectionDialogFiller::run() {

    QWidget* dialog = QApplication::activeModalWidget();
    GT_CHECK(dialog, "activeModalWidget is NULL");

    QComboBox *consensusCombo=dialog->findChild<QComboBox*>("consensusCombo");
    GT_CHECK(consensusCombo!=NULL, "consensusCombo is NULL");
    GTComboBox::setCurrentIndex(os,consensusCombo,comboBoxVal);

    if(spinVal){
        QSpinBox *thresholdSpin=dialog->findChild<QSpinBox*>("thresholdSpin");
        GT_CHECK(thresholdSpin!=NULL, "consensusCombo is NULL");
        GTSpinBox::setValue(os,thresholdSpin,spinVal,GTGlobals::UseKey);
    }

    if (reset){
        QToolButton* resetButton = dialog->findChild<QToolButton*>("thresholdDefaultButton");
        GT_CHECK(resetButton,"reset button not found");
        GTWidget::click(os,resetButton);
    }
    GTWidget::click(os, GTWidget::findWidget(os,"okButton"));


}
#undef GT_METHOD_NAME
#undef GT_CLASS_NAME


#define GT_CLASS_NAME "CheckConsensusValues"
#define GT_METHOD_NAME "run"
void CheckConsensusValues::run(){
    QWidget* dialog = QApplication::activeModalWidget();
    GT_CHECK(dialog, "activeModalWidget is NULL");

    QComboBox *consensusCombo=dialog->findChild<QComboBox*>("consensusCombo");
    GT_CHECK(consensusCombo!=NULL, "consensusCombo is NULL");

    QSpinBox *thresholdSpin=dialog->findChild<QSpinBox*>("thresholdSpin");
    GT_CHECK(thresholdSpin!=NULL, "consensusCombo is NULL");

    if(comboValue!=-1){
        GT_CHECK(comboValue==consensusCombo->currentIndex(),
                 QString("Expected comboValue = %1, found = %2").arg(comboValue).arg(consensusCombo->currentIndex()));
    }

    if(spinValue!=-1){
        GT_CHECK(spinValue==thresholdSpin->value(),
                 QString("Expected spinValue = %1, found = %2").arg(spinValue).arg(thresholdSpin->value()));
    }
    GTWidget::click(os, GTWidget::findWidget(os,"okButton"));
}
#undef GT_METHOD_NAME
#undef GT_CLASS_NAME

}
