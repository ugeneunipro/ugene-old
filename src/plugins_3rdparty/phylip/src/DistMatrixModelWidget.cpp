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
 
}

void DistMatrixModelWidget::fillSettings( CreatePhyTreeSettings& settings )
{
    settings.matrixId = modelBox->currentText();
    settings.useGammaDistributionRates = gammaCheckBox->isChecked();
    settings.alphaFactor = alphaSpinBox->value();
}

void DistMatrixModelWidget::sl_onModelChanged(const QString& modelName) {
    if (modelName == DNADistModelTypes::F84 || modelName == DNADistModelTypes::Kimura) {
        transitionRatioSpinBox->setEnabled(true);
    } else {
        transitionRatioSpinBox->setEnabled(false);
    }
}



} //namespace
