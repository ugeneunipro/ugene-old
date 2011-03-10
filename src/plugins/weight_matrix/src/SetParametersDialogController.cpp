#include "SetParametersDialogController.h"

#include <U2Core/AppContext.h>
#include <U2Algorithm/PWMConversionAlgorithmRegistry.h>

namespace U2 {

SetParametersDialogController::SetParametersDialogController(QWidget *w) 
    : QDialog(w) {

    setupUi(this);
    
    QStringList algo = AppContext::getPWMConversionAlgorithmRegistry()->getAlgorithmIds();
    algorithmComboBox->addItems(algo);

    connect(okButton, SIGNAL(clicked()), SLOT(sl_onOkButton()));
    connect(scoreSlider, SIGNAL(valueChanged(int)), SLOT(sl_onSliderMoved(int)));
}

void SetParametersDialogController::sl_onSliderMoved(int value) {
    scoreValueLabel->setText(QString("%1%").arg(value));
}

void SetParametersDialogController::sl_onOkButton() {
    QDialog::accept();
}

}