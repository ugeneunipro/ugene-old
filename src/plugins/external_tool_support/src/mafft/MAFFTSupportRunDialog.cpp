#include "MAFFTSupportRunDialog.h"

#include <U2Misc/DialogUtils.h>
#include <QtGui/QFileDialog>
#include <QtGui/QToolButton>

namespace U2 {

////////////////////////////////////////
//MAFFTSupportRunDialog
MAFFTSupportRunDialog::MAFFTSupportRunDialog(MAFFTSupportTaskSettings& _settings, QWidget* _parent) :
        QDialog(_parent), settings(_settings)
{
    setupUi(this);
    connect(this->cancelButton,SIGNAL(clicked()),this,SLOT(reject()));
    connect(this->alignButton,SIGNAL(clicked()),this,SLOT(sl_align()));
}

void MAFFTSupportRunDialog::sl_align(){
    if(gapOpenCheckBox->isChecked()) {
        settings.gapOpenPenalty = gapOpenSpinBox->value();
    }
    if(gapExtCheckBox->isChecked()) {
        settings.gapExtenstionPenalty = gapExtSpinBox->value();
    }
    if(maxNumberIterRefinementCheckBox->isChecked()){
        settings.maxNumberIterRefinement = maxNumberIterRefinementSpinBox->value();
    }
    accept();
}
////////////////////////////////////////
//MAFFTWithExtFileSpecifySupportRunDialog
MAFFTWithExtFileSpecifySupportRunDialog::MAFFTWithExtFileSpecifySupportRunDialog(MAFFTSupportTaskSettings& _settings, QWidget* _parent) :
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
    connect(this->cancelButton, SIGNAL(clicked()), this, SLOT(reject()));
    connect(this->alignButton, SIGNAL(clicked()), this, SLOT(sl_align()));
}

void MAFFTWithExtFileSpecifySupportRunDialog::sl_inputFileLineEditChanged(const QString& str){
    alignButton->setEnabled(!str.isEmpty());
}

void MAFFTWithExtFileSpecifySupportRunDialog::sl_align(){
    if(gapOpenCheckBox->isChecked()) {
        settings.gapOpenPenalty = gapOpenSpinBox->value();
    }
    if(gapExtCheckBox->isChecked()) {
        settings.gapExtenstionPenalty = gapExtSpinBox->value();
    }
    if(maxNumberIterRefinementCheckBox->isChecked()){
        settings.maxNumberIterRefinement = maxNumberIterRefinementSpinBox->value();
    }
    if(!inputFileLineEdit->text().isEmpty()){
        settings.inputFilePath=inputFileLineEdit->text();
    }else{
        assert(NULL);
        reject();
    }
    accept();
}

}//namespace
