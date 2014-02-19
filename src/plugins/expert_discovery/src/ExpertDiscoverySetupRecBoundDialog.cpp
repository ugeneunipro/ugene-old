#include "ExpertDiscoverySetupRecBoundDialog.h"

#include <QtGui/QMessageBox>
#include <U2Gui/HelpButton.h>

namespace U2 {

ExpertDiscoverySetupRecBoundDialog::ExpertDiscoverySetupRecBoundDialog(double dRecognizationBound, 
        const std::vector<double>& vPosScore, 
        const std::vector<double>& vNegScore)
: QDialog()
, recognizationBound(dRecognizationBound)
, probNegRec(0)
, probPosRej(0)
, posScore( vPosScore )
, negScore( vNegScore )
,graphWidget(NULL){

    setupUi(this);
    new HelpButton(this, buttonBox, "4227131");
    
    boundLabel->setStyleSheet(QString("color : %1;").arg(ExpertDiscoveryRecognitionErrorGraphWidget::BOUNDCOLOR.name()));
    er1Lable->setStyleSheet(QString("color : %1;").arg(ExpertDiscoveryRecognitionErrorGraphWidget::ER1COLOR.name()));
    er2Label->setStyleSheet(QString("color : %1;").arg(ExpertDiscoveryRecognitionErrorGraphWidget::ER2COLOR.name()));

    recBoundSpinBox->setValue(recognizationBound);
    posRecLineEdit->setText(QString("%1").arg(probPosRej));
    negRecLineEdit->setText(QString("%1").arg(probNegRec));

    warningLabel->setVisible(false);
    
    CalculateErrorTaskInfo settings;
    settings.scoreReg = U2Region(0, 50);
    settings.scoreStep = 0.1;

    settings.posScore.resize(posScore.size());
    for(int i = 0; i < posScore.size(); i++){
        settings.posScore[i] = posScore[i];
    }

    settings.negScore.resize(negScore.size());
    for(int i = 0; i < negScore.size(); i++){
        settings.negScore[i] = negScore[i];
    } 
    graphWidget = new ExpertDiscoveryRecognitionErrorGraphWidget(this, posScore, negScore, settings);
    graphLayout->addWidget(graphWidget);
    graphWidget->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
    graphWidget->draw(recognizationBound);

    connect(graphWidget, SIGNAL(si_showWarning(bool)), SLOT(sl_showWarning(bool)));
    connect(optimizeRecognitionBoundButton, SIGNAL(clicked()), SLOT(sl_optRecBound()));
    connect(recBoundSpinBox, SIGNAL(valueChanged ( double )), SLOT(sl_recBoundChaged(double)));
    connect(calculateButton, SIGNAL(clicked()), SLOT(sl_recalculateValues()));
    connect(minBoundSpin, SIGNAL(valueChanged (int)), SLOT(sl_intervalChanged(int)));
    connect(maxBoundSpin, SIGNAL(valueChanged (int)), SLOT(sl_intervalChanged(int)));

    maxBoundSpin->setValue(50);

    sl_recBoundChaged(recognizationBound);

}

void ExpertDiscoverySetupRecBoundDialog::accept(){

    recognizationBound = recBoundSpinBox->value();
    QDialog::accept();


}
void ExpertDiscoverySetupRecBoundDialog::sl_recBoundChaged(double val){
    recognizationBound = val;
    updateProbs();
    graphWidget->draw(recognizationBound);
    posRecLineEdit->setText(QString("%1").arg(probPosRej));
    negRecLineEdit->setText(QString("%1").arg(probNegRec));
}

void ExpertDiscoverySetupRecBoundDialog::sl_showWarning(bool isShown){
    warningLabel->setVisible(isShown);
}
void ExpertDiscoverySetupRecBoundDialog::sl_optRecBound(){
    double dPosScore = 0;
    for (uint i=0; i<posScore.size(); i++){
        dPosScore += posScore[i];
    }
    dPosScore /= posScore.size();

    double dNegScore = 0;
    for (uint i=0; i<negScore.size(); i++)
    {
        dNegScore += negScore[i];
    }
    dNegScore /= negScore.size();
    sl_recBoundChaged((dPosScore + dNegScore)/2);
    recBoundSpinBox->setValue(recognizationBound);
}

void ExpertDiscoverySetupRecBoundDialog::updateProbs(){
    probPosRej = 0;
    for (int i=0; i<(int)posScore.size(); i++)
        if (posScore[i] < recognizationBound) probPosRej++;
    probPosRej /= posScore.size();

    probNegRec = 0;
    for (int i=0; i<(int)negScore.size(); i++)
        if (negScore[i] >= recognizationBound) probNegRec++;
    probNegRec /= negScore.size();    
}

void ExpertDiscoverySetupRecBoundDialog::sl_intervalChanged(int val){
    int leftValue = minBoundSpin->value();
    int rightValue = maxBoundSpin->value();

    QPalette p = minBoundSpin->palette();
    if(leftValue >= rightValue){
        p.setColor(QPalette::Base, QColor(255,200,200));//pink color
    }else{
        p.setColor(QPalette::Base, QColor(255,255,255));//pink color
    }

    minBoundSpin->setPalette(p);
    maxBoundSpin->setPalette(p);
}
void ExpertDiscoverySetupRecBoundDialog::sl_recalculateValues(){
    int leftValue = minBoundSpin->value();
    int rightValue = maxBoundSpin->value();
    if(leftValue >= rightValue){
         QMessageBox mb(QMessageBox::Critical, tr("Wrong bound interval"), 
             tr("Minimal bound must not be grater then maximal bound"));
         mb.exec();
         return;
    }
    double stepVale = stepSpin->value();

    if(stepVale == 0){
        QMessageBox mb(QMessageBox::Critical, tr("Wrong step value"), 
            tr("Bound step must be non zero"));
        mb.exec();
        return;
    }


    CalculateErrorTaskInfo settings;
    settings.scoreReg = U2Region(leftValue, rightValue);
    settings.scoreStep = stepVale;

    settings.posScore.resize(posScore.size());
    for(int i = 0; i < posScore.size(); i++){
        settings.posScore[i] = posScore[i];
    }

    settings.negScore.resize(negScore.size());
    for(int i = 0; i < negScore.size(); i++){
        settings.negScore[i] = negScore[i];
    } 

    graphWidget->sl_calculateErrors(settings);
}

}//namespace


