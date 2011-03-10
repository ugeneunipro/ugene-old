#include "ClustalWSupportRunDialog.h"
#include <U2Core/DNAAlphabet.h>
#include <U2Misc/DialogUtils.h>

#include <QtGui/QFileDialog>
#include <QtGui/QToolButton>

namespace U2 {
////////////////////////////////////////
//ClustalWSupportRunDialog
ClustalWSupportRunDialog::ClustalWSupportRunDialog(const MAlignment& _ma, ClustalWSupportTaskSettings& _settings, QWidget* _parent) :
        QDialog(_parent), ma(_ma), settings(_settings)
{
    setupUi(this);
    connect(this->iterationTypeCheckBox,SIGNAL(toggled(bool)),this,SLOT(sl_iterationTypeEnabled(bool)));
    connect(this->cancelButton,SIGNAL(clicked()),this,SLOT(reject()));
    connect(this->alignButton,SIGNAL(clicked()),this,SLOT(sl_align()));
    if(ma.getAlphabet()->isAmino()){
        gapOpenSpinBox->setValue(10.0);
        gapExtSpinBox->setValue(0.2);
        proteinGapParamGroupBox->setEnabled(true);
        weightMatrixComboBox->clear();
        weightMatrixComboBox->addItem("BLOSUM");
        weightMatrixComboBox->addItem("PAM");
        weightMatrixComboBox->addItem("GONNET");
        weightMatrixComboBox->addItem("ID");
    }
}
void ClustalWSupportRunDialog::sl_iterationTypeEnabled(bool checked){
    if(checked){
        iterationTypeComboBox->removeItem(0);
    }else{
        iterationTypeComboBox->insertItem(0,"NONE");
        iterationTypeComboBox->setCurrentIndex(0);
    }
}
void ClustalWSupportRunDialog::sl_align(){
    if(gapOpenCheckBox->isChecked()) {
        settings.gapOpenPenalty = gapOpenSpinBox->value();
    }
    if(gapExtCheckBox->isChecked()) {
        settings.gapExtenstionPenalty = gapExtSpinBox->value();
    }
    if(gapDistancesCheckBox->isChecked()){
        settings.gapDist = gapDistancesSpinBox->value();
    }
    if(residueSpecificGapsOffCheckBox->isChecked()){
        settings.noPGaps=true;

    }
    if(hydrophilicGapsOffCheckBox->isChecked()){
        settings.noHGaps=true;
    }
    if(endGapsCheckBox->isChecked()){
        settings.endGaps=true;
    }
    if(weightMatrixCheckBox->isChecked()){
        settings.matrix=weightMatrixComboBox->currentText();
    }
    if(outOrderCheckBox->isChecked()){
        if(outOrderComboBox->currentText() == "Input"){
            settings.outOrderInput=true;
        }else{
            settings.outOrderInput=false;
        }
    }
    if(iterationTypeCheckBox->isChecked()){
        settings.iterationType=iterationTypeComboBox->currentText();
        if(maxIterationsCheckBox->isChecked()){
            settings.numIterations=maxIterationsSpinBox->value();
        }
    }
    accept();
}

////////////////////////////////////////
//ClustalWWithExtFileSpecifySupportRunDialog
ClustalWWithExtFileSpecifySupportRunDialog::ClustalWWithExtFileSpecifySupportRunDialog(ClustalWSupportTaskSettings& _settings, QWidget* _parent) :
        QDialog(_parent), settings(_settings)
{
    setupUi(this);
    QWidget * widget = new QWidget(_parent);
    inputFileLineEdit= new FileLineEdit(DialogUtils::prepareDocumentsFileFilterByObjType(GObjectTypes::MULTIPLE_ALIGNMENT, true),
        "", false, widget);
    inputFileLineEdit->setText("");
    QToolButton * selectToolPathButton = new QToolButton(widget);
    selectToolPathButton->setVisible(true);
    selectToolPathButton->setText("...");
    connect(selectToolPathButton, SIGNAL(clicked()), inputFileLineEdit, SLOT(sl_onBrowse()));
    connect(inputFileLineEdit,SIGNAL(textChanged(QString)),this, SLOT(sl_inputFileLineEditChanged(QString)));

    QHBoxLayout* layout = new QHBoxLayout(widget);
    layout->addWidget(inputFileLineEdit);
    layout->addWidget(selectToolPathButton);

    QGroupBox* inputFileGroupBox=new QGroupBox(tr("Select input file"),widget);
    inputFileGroupBox->setLayout(layout);
    QBoxLayout* parentLayout = qobject_cast<QBoxLayout*>(this->layout());
    assert(parentLayout);
    parentLayout->insertWidget(0, inputFileGroupBox);
    alignButton->setEnabled(false);

    connect(this->iterationTypeCheckBox,SIGNAL(toggled(bool)),this,SLOT(sl_iterationTypeEnabled(bool)));
    connect(this->cancelButton,SIGNAL(clicked()),this,SLOT(reject()));
    connect(this->alignButton,SIGNAL(clicked()),this,SLOT(sl_align()));

    proteinGapParamGroupBox->setEnabled(true);
    weightMatrixComboBox->insertSeparator(2);
    weightMatrixComboBox->addItem("BLOSUM");
    weightMatrixComboBox->addItem("PAM");
    weightMatrixComboBox->addItem("GONNET");
    weightMatrixComboBox->addItem("ID");
}
void ClustalWWithExtFileSpecifySupportRunDialog::sl_inputFileLineEditChanged(const QString& str){
    alignButton->setEnabled(!str.isEmpty());
}
void ClustalWWithExtFileSpecifySupportRunDialog::sl_iterationTypeEnabled(bool checked){
    if(checked){
        iterationTypeComboBox->removeItem(0);
    }else{
        iterationTypeComboBox->insertItem(0,"NONE");
        iterationTypeComboBox->setCurrentIndex(0);
    }
}
void ClustalWWithExtFileSpecifySupportRunDialog::sl_align(){
    if(gapOpenCheckBox->isChecked()) {
        settings.gapOpenPenalty = gapOpenSpinBox->value();
    }
    if(gapExtCheckBox->isChecked()) {
        settings.gapExtenstionPenalty = gapExtSpinBox->value();
    }
    if(gapDistancesCheckBox->isChecked()){
        settings.gapDist = gapDistancesSpinBox->value();
    }
    if(residueSpecificGapsOffCheckBox->isChecked()){
        settings.noPGaps=true;

    }
    if(hydrophilicGapsOffCheckBox->isChecked()){
        settings.noHGaps=true;
    }
    if(endGapsCheckBox->isChecked()){
        settings.endGaps=true;
    }
    if(weightMatrixCheckBox->isChecked()){
        settings.matrix=weightMatrixComboBox->currentText();
    }
    if(iterationTypeCheckBox->isChecked()){
        settings.iterationType=iterationTypeComboBox->currentText();
        if(maxIterationsCheckBox->isChecked()){
            settings.numIterations=maxIterationsSpinBox->value();
        }
    }    if(!inputFileLineEdit->text().isEmpty()){
        settings.inputFilePath=inputFileLineEdit->text();
    }else{
        assert(NULL);
        reject();
    }
    accept();
}

}//namespace
