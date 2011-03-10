#include "ConsensusSelectorDialogController.h"

#include <U2Core/AppContext.h>
#include <U2Core/Counter.h>
#include <U2Algorithm/MSAConsensusAlgorithmRegistry.h>

namespace U2 {

ConsensusSelectorDialogController::ConsensusSelectorDialogController(const QString& _selectedAlgorithmId, ConsensusAlgorithmFlags flags, QWidget* p) 
: QDialog(p) 
{
    setupUi(this);
    
    MSAConsensusAlgorithmRegistry* reg = AppContext::getMSAConsensusAlgorithmRegistry();
    QList<MSAConsensusAlgorithmFactory*> algos = reg->getAlgorithmFactories(flags);

    //TODO: sort algos by name
    foreach(const MSAConsensusAlgorithmFactory* algo, algos) {
        consensusCombo->addItem(QIcon(), algo->getName(), algo->getId());
    }

    selectedAlgorithmId = _selectedAlgorithmId;
    int indexToSelect = qMax(0, consensusCombo->findData(selectedAlgorithmId));
    consensusCombo->setCurrentIndex(indexToSelect);
    
    disableThresholdSelector();

    updateSelectedAlgorithmDesc();

    connect(consensusCombo, SIGNAL(currentIndexChanged(int)), SLOT(sl_algorithmSelectionChanged(int)));
    connect(thresholdSlider, SIGNAL(valueChanged(int)), SLOT(sl_thresholdSliderChanged(int)));
    connect(thresholdSpin, SIGNAL(valueChanged(int)), SLOT(sl_thresholdSpinChanged(int)));
    connect(thresholdDefaultButton, SIGNAL(clicked(bool)), SLOT(sl_resetThresholdClicked(bool)));
    
    GCOUNTER(cvar,tvar, "ConsensusSelectorDialog");
}

void ConsensusSelectorDialogController::sl_algorithmSelectionChanged(int i) {
    selectedAlgorithmId = consensusCombo->itemData(i).toString();
    updateSelectedAlgorithmDesc();
    emit si_algorithmChanged(selectedAlgorithmId);
}

void ConsensusSelectorDialogController::updateSelectedAlgorithmDesc() {
    MSAConsensusAlgorithmRegistry* reg = AppContext::getMSAConsensusAlgorithmRegistry();
    MSAConsensusAlgorithmFactory* algo = reg->getAlgorithmFactory(selectedAlgorithmId);
    QString description = algo->getDescription();
    descriptionEdit->setText(description);
}



void ConsensusSelectorDialogController::enableThresholdSelector(int minVal, int maxVal, int value, const QString& suffix) {
    thresholdLabel->setEnabled(true);
    thresholdSlider->setEnabled(true);
    thresholdSpin->setEnabled(true);
    thresholdDefaultButton->setEnabled(true);
    
    thresholdSlider->setRange(minVal, maxVal);
    thresholdSpin->setRange(minVal, maxVal);
    thresholdSpin->setSuffix(suffix);

    thresholdSpin->setValue(qBound(minVal, value, maxVal)); //triggers slider update too
}

void ConsensusSelectorDialogController::disableThresholdSelector() {
    thresholdLabel->setEnabled(false);
    thresholdSlider->setEnabled(false);
    thresholdSpin->setEnabled(false);
    thresholdDefaultButton->setEnabled(false);
    
    thresholdSlider->setRange(0, 0);
    thresholdSpin->setRange(0, 0);
    thresholdSpin->setSuffix(QString());
    
    thresholdSpin->setValue(0); //triggers slider update too
}

int ConsensusSelectorDialogController::getThresholdValue() const {
    assert(thresholdSpin->isEnabled());
    return thresholdSpin->value();
}

void ConsensusSelectorDialogController::sl_thresholdSliderChanged(int val) {
    thresholdSpin->disconnect(this);
    thresholdSpin->setValue(val);
    connect(thresholdSpin, SIGNAL(valueChanged(int)), SLOT(sl_thresholdSpinChanged(int)));
    emit si_thresholdChanged(val);
}

void ConsensusSelectorDialogController::sl_thresholdSpinChanged(int val) {
    thresholdSlider->disconnect(this);
    thresholdSlider->setValue(val);
    connect(thresholdSlider, SIGNAL(valueChanged(int)), SLOT(sl_thresholdSliderChanged(int)));
    emit si_thresholdChanged(val);
}

void ConsensusSelectorDialogController::sl_resetThresholdClicked(bool) {
    MSAConsensusAlgorithmFactory* factory = AppContext::getMSAConsensusAlgorithmRegistry()->getAlgorithmFactory(selectedAlgorithmId);
    int defaultVal = factory->getDefaultThreshold();
    thresholdSpin->setValue(defaultVal);
}

};
