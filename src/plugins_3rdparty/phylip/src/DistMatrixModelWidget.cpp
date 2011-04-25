/**
 * UGENE - Integrated Bioinformatics Tools.
 * Copyright (C) 2008-2011 UniPro <ugene@unipro.ru>
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

#include "DistMatrixModelWidget.h"
#include "dnadist.h"
#include "protdist.h"

#include <U2Core/DNAAlphabet.h>

namespace U2 {

#define DIST_MATRIX_MODEL_PATH "/model"
#define DIST_MATRIX_GAMMADISTR_PATH "/gamma_distr"
#define DIST_MATRIX_ALPHAFACTOR_PATH "/alfa_factor"
#define DIST_MATRIX_TRRATIO_PATH "/ttratio"

DistMatrixModelWidget::DistMatrixModelWidget(QWidget* parent, const MAlignment& ma) : CreatePhyTreeWidget(parent)
{
    setupUi(this);
    DNAAlphabetType alphabetType = ma.getAlphabet()->getType();
    if ((alphabetType == DNAAlphabet_RAW) || (alphabetType == DNAAlphabet_NUCL)){
        modelBox->addItems( DNADistModelTypes::getDNADistModelTypes() );
    } else {
        modelBox->addItems( ProtDistModelTypes::getProtDistModelTypes() );
    }
    connect(modelBox, SIGNAL(currentIndexChanged(const QString&)), SLOT(sl_onModelChanged(const QString&)));
 
    QString modelName = AppContext::getSettings()->getValue(CreatePhyTreeWidget::settingsPath + DIST_MATRIX_MODEL_PATH, modelBox->itemText(0)).toString();

    for(int i = 0; i<modelBox->count(); i++){
        if(modelBox->itemText(i) == modelName){
            modelBox->setCurrentIndex(i);
            break;
        }
    }
    gammaCheckBox->setChecked(AppContext::getSettings()->getValue(CreatePhyTreeWidget::settingsPath + DIST_MATRIX_GAMMADISTR_PATH, false).toBool());
    alphaSpinBox->setValue(AppContext::getSettings()->getValue(CreatePhyTreeWidget::settingsPath + DIST_MATRIX_ALPHAFACTOR_PATH, 0.5).toDouble());
    transitionRatioSpinBox->setValue(AppContext::getSettings()->getValue(CreatePhyTreeWidget::settingsPath + DIST_MATRIX_TRRATIO_PATH, 2.0).toDouble());
}

void DistMatrixModelWidget::fillSettings( CreatePhyTreeSettings& settings )
{
    settings.matrixId = modelBox->currentText();
    settings.useGammaDistributionRates = gammaCheckBox->isChecked();
    settings.alphaFactor = alphaSpinBox->value();
}
void DistMatrixModelWidget::storeSettings(){
     AppContext::getSettings()->setValue(CreatePhyTreeWidget::settingsPath + DIST_MATRIX_MODEL_PATH, modelBox->currentText());
     AppContext::getSettings()->setValue(CreatePhyTreeWidget::settingsPath + DIST_MATRIX_GAMMADISTR_PATH, gammaCheckBox->isChecked());
     AppContext::getSettings()->setValue(CreatePhyTreeWidget::settingsPath + DIST_MATRIX_ALPHAFACTOR_PATH, alphaSpinBox->value());
     AppContext::getSettings()->setValue(CreatePhyTreeWidget::settingsPath + DIST_MATRIX_TRRATIO_PATH, transitionRatioSpinBox->value());
}
void DistMatrixModelWidget::restoreDefault(){
    AppContext::getSettings()->remove(CreatePhyTreeWidget::settingsPath + DIST_MATRIX_MODEL_PATH);
    AppContext::getSettings()->remove(CreatePhyTreeWidget::settingsPath + DIST_MATRIX_GAMMADISTR_PATH);
    AppContext::getSettings()->remove(CreatePhyTreeWidget::settingsPath + DIST_MATRIX_ALPHAFACTOR_PATH);
    AppContext::getSettings()->remove(CreatePhyTreeWidget::settingsPath + DIST_MATRIX_TRRATIO_PATH);

    modelBox->setCurrentIndex(0);
    gammaCheckBox->setChecked(false);
    alphaSpinBox->setValue(0.5);
    transitionRatioSpinBox->setValue(2.0);
}

void DistMatrixModelWidget::sl_onModelChanged(const QString& modelName) {
    if (modelName == DNADistModelTypes::F84 || modelName == DNADistModelTypes::Kimura) {
        transitionRatioSpinBox->setEnabled(true);
    } else {
        transitionRatioSpinBox->setEnabled(false);
    }
}



} //namespace
