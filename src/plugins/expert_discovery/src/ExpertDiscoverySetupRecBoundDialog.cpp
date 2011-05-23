#include "ExpertDiscoverySetupRecBoundDialog.h"

#include <QtGui/QMessageBox>

namespace U2 {

ExpertDiscoverySetupRecBoundDialog::ExpertDiscoverySetupRecBoundDialog(double dRecognizationBound, 
        const std::vector<double>& vPosScore, 
        const std::vector<double>& vNegScore)
: QDialog()
, recognizationBound(dRecognizationBound)
, probNegRec(0)
, probPosRej(0)
, posScore( vPosScore )
, negScore( vNegScore ){

    setupUi(this);
    
    recBoundSpinBox->setValue(recognizationBound);
    posRecLineEdit->setText(QString("%1").arg(probPosRej));
    negRecLineEdit->setText(QString("%1").arg(probNegRec));

    connect(recBoundSpinBox, SIGNAL(valueChanged ( double )), SLOT(sl_recBoundChaged(double)));
    sl_recBoundChaged(recognizationBound);
}

void ExpertDiscoverySetupRecBoundDialog::accept(){

    recognizationBound = recBoundSpinBox->value();
    QDialog::accept();


}
void ExpertDiscoverySetupRecBoundDialog::sl_recBoundChaged(double val){
    recognizationBound = val;
    updateProbs();
    posRecLineEdit->setText(QString("%1").arg(probPosRej));
    negRecLineEdit->setText(QString("%1").arg(probNegRec));
}

void ExpertDiscoverySetupRecBoundDialog::updateProbs(){
    probPosRej = 0;
    for (int i=0; i<(int)posScore.size(); i++)
        if (posScore[i] <= recognizationBound) probPosRej++;
    probPosRej /= posScore.size();

    probNegRec = 0;
    for (int i=0; i<(int)negScore.size(); i++)
        if (negScore[i] > recognizationBound) probNegRec++;
    probNegRec /= negScore.size();    
}

}//namespace


