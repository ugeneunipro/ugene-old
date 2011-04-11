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

#include "SeqBootModelWidget.h"

#include <U2Core/DNAAlphabet.h>

#include <QtCore/QTime>

namespace U2 {


QString ConsensusModelTypes::M1("M1");
QString ConsensusModelTypes::Strict("Strict");
QString ConsensusModelTypes::MajorityRuleExt("Majority Rule (extended)");
QString ConsensusModelTypes::MajorityRule("Majority Rule");


QList<QString> ConsensusModelTypes::getConsensusModelTypes()
{
    static QList<QString> list;
    if (list.isEmpty()) {
        list.append(ConsensusModelTypes::MajorityRuleExt);
        list.append(ConsensusModelTypes::Strict);
        list.append(ConsensusModelTypes::MajorityRule);
        list.append(ConsensusModelTypes::M1);
    }

    return list;
}

SeqBootModelWidget::SeqBootModelWidget(QWidget* parent, const MAlignment& ma) : CreatePhyTreeWidget(parent)
{
    setupUi(this);
    BootstrapGroupBox->setChecked(false);

    seedSpinBox->setValue(getRandomSeed());
    
    ConsModeComboBox->addItems( ConsensusModelTypes::getConsensusModelTypes() );
    connect(ConsModeComboBox, SIGNAL(currentIndexChanged(const QString&)), SLOT(sl_onModelChanged(const QString&))); 
}

void SeqBootModelWidget::fillSettings( CreatePhyTreeSettings& settings )
{
    settings.bootstrap = BootstrapGroupBox->isChecked();
    settings.replicates = repsSpinBox->value();
    settings.seed = seedSpinBox->value();
    settings.fraction = FractionSpinBox->value();
    settings.consensusID = ConsModeComboBox->currentText();
}

#define SEED_MIN 5
#define SEED_MAX 32765
int SeqBootModelWidget::getRandomSeed(){
    int seed = 0;
    qsrand(QTime(0,0,0).secsTo(QTime::currentTime()));
    seed = qAbs(qrand()); 

    while(!checkSeed(seed)){
        seed++;
        if(seed >=SEED_MAX){
            seed = SEED_MIN;
        }
    }
    
    return seed;
}
bool SeqBootModelWidget::checkSeed(int seed){
    return (seed >= SEED_MIN) && (seed <=SEED_MAX) && ((seed-1)%4 == 0);
}


void SeqBootModelWidget::sl_onModelChanged(const QString& modelName) {
    if (modelName == ConsensusModelTypes::M1) {
        label5->setEnabled(true);
        FractionSpinBox->setEnabled(true); 
    } else {
        label5->setEnabled(false);
        FractionSpinBox->setEnabled(false);
    }
    if(modelName == ConsensusModelTypes::Strict){
        FractionSpinBox->setValue(1.0);
    }
    if(modelName == ConsensusModelTypes::MajorityRule){
        FractionSpinBox->setValue(0.5);
    }
    if(modelName == ConsensusModelTypes::MajorityRuleExt){
        FractionSpinBox->setValue(0.5);
    }
}




} //namespace
