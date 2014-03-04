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

#include "SeqBootModelWidget.h"

#include <U2Core/DNAAlphabet.h>
#include <U2Core/AppContext.h>
#include <U2Core/Settings.h>

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

#define SEQ_BOOT_BOTTSTR_PATH "/bootstr"
#define SEQ_BOOT_REPL_PATH "/replicates"
#define SEQ_BOOT_SEED_PATH "/seq_boot_seed"
#define SEQ_BOOT_FRACTION_PATH "/fraction"
#define SEQ_BOOT_CONSENSUSID_PATH "/consensusID"

SeqBootModelWidget::SeqBootModelWidget(QWidget* parent, const MAlignment& ma) : CreatePhyTreeWidget(parent)
{
    setupUi(this);
    BootstrapGroupBox->setChecked(false);

    seedSpinBox->setValue(getRandomSeed());
    
    ConsModeComboBox->addItems( ConsensusModelTypes::getConsensusModelTypes() );
    connect(ConsModeComboBox, SIGNAL(currentIndexChanged(const QString&)), SLOT(sl_onModelChanged(const QString&))); 

    BootstrapGroupBox->setChecked(AppContext::getSettings()->getValue(CreatePhyTreeWidget::settingsPath + SEQ_BOOT_BOTTSTR_PATH, false).toBool());
    repsSpinBox->setValue(AppContext::getSettings()->getValue(CreatePhyTreeWidget::settingsPath + SEQ_BOOT_REPL_PATH, 100).toInt());
    seedSpinBox->setValue(AppContext::getSettings()->getValue(CreatePhyTreeWidget::settingsPath + SEQ_BOOT_SEED_PATH, getRandomSeed()).toInt());
    FractionSpinBox->setValue(AppContext::getSettings()->getValue(CreatePhyTreeWidget::settingsPath + SEQ_BOOT_FRACTION_PATH, 0.5).toDouble());

    QString consModelName = AppContext::getSettings()->getValue(CreatePhyTreeWidget::settingsPath + SEQ_BOOT_CONSENSUSID_PATH, ConsModeComboBox->itemText(0)).toString();

    for(int i = 0; i<ConsModeComboBox->count(); i++){
        if(ConsModeComboBox->itemText(i) == consModelName){
            ConsModeComboBox->setCurrentIndex(i);
            break;
        }
    }
}

void SeqBootModelWidget::fillSettings( CreatePhyTreeSettings& settings )
{
    settings.bootstrap = BootstrapGroupBox->isChecked();
    settings.replicates = repsSpinBox->value();
    settings.seed = seedSpinBox->value();
    settings.fraction = FractionSpinBox->value();
    settings.consensusID = ConsModeComboBox->currentText();
}

void SeqBootModelWidget::storeSettings(){
    AppContext::getSettings()->setValue(CreatePhyTreeWidget::settingsPath + SEQ_BOOT_BOTTSTR_PATH, BootstrapGroupBox->isChecked());
    AppContext::getSettings()->setValue(CreatePhyTreeWidget::settingsPath + SEQ_BOOT_REPL_PATH, repsSpinBox->value());
    AppContext::getSettings()->setValue(CreatePhyTreeWidget::settingsPath + SEQ_BOOT_SEED_PATH, seedSpinBox->value());
    AppContext::getSettings()->setValue(CreatePhyTreeWidget::settingsPath + SEQ_BOOT_FRACTION_PATH, FractionSpinBox->value());
    AppContext::getSettings()->setValue(CreatePhyTreeWidget::settingsPath + SEQ_BOOT_CONSENSUSID_PATH, ConsModeComboBox->currentText());
    
}
void SeqBootModelWidget::restoreDefault(){
    AppContext::getSettings()->remove(CreatePhyTreeWidget::settingsPath + SEQ_BOOT_BOTTSTR_PATH);
    AppContext::getSettings()->remove(CreatePhyTreeWidget::settingsPath + SEQ_BOOT_REPL_PATH);
    AppContext::getSettings()->remove(CreatePhyTreeWidget::settingsPath + SEQ_BOOT_SEED_PATH);
    AppContext::getSettings()->remove(CreatePhyTreeWidget::settingsPath + SEQ_BOOT_FRACTION_PATH);
    AppContext::getSettings()->remove(CreatePhyTreeWidget::settingsPath + SEQ_BOOT_CONSENSUSID_PATH);


    BootstrapGroupBox->setChecked(false);
    repsSpinBox->setValue(100);
    seedSpinBox->setValue(getRandomSeed());
    FractionSpinBox->setValue(0.5);
    ConsModeComboBox->setCurrentIndex(0);
}

#define SEED_MIN 5
#define SEED_MAX 32765
bool SeqBootModelWidget::checkSettings(QString& msg, const CreatePhyTreeSettings& settings){
    if(!((settings.seed >= SEED_MIN) && (settings.seed <= SEED_MAX) && (settings.seed%2 == 1))){
        msg = tr("Seed must be odd");
        return false;
    }

    return true;
}
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
