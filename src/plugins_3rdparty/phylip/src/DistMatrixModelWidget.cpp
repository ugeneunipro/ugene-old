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
