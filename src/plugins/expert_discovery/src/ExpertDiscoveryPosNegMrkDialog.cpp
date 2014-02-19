#include "ExpertDiscoveryPosNegMrkDialog.h"

#include <U2Core/GObjectTypes.h>
#include <U2Gui/LastUsedDirHelper.h>
#include <U2Gui/DialogUtils.h>

#include <QtGui/QFileDialog>
#include <QtGui/QMessageBox>
#include <U2Gui/HelpButton.h>


namespace U2 {

ExpertDiscoveryPosNegMrkDialog::ExpertDiscoveryPosNegMrkDialog(QWidget *parent)
: QDialog(parent), generateDescr(true){

    setupUi(this);
    new HelpButton(this, buttonBox, "4227131");

    connect(openFirstButton, SIGNAL(clicked()), SLOT(sl_openFirstFile()));
    connect(openSecondButton, SIGNAL(clicked()), SLOT(sl_openSecondFile()));
    connect(openThirdButton, SIGNAL(clicked()), SLOT(sl_openThirdFile()));
    connect(oneSequenceCheckBox, SIGNAL(clicked()), SLOT(sl_oneSequence()));
    connect(lettersCheck, SIGNAL(clicked()), SLOT(sl_lettersMarkup()));

    oneSequenceCheckBox->click();

    //hiding description file for now (in order to useless)
    oneSequenceCheckBox->hide();
    label_4->hide();
    thirdFileEdit->hide();
    openThirdButton->hide();

    filter = DialogUtils::prepareFileFilter("Markup files", QStringList() << "xml" << "gb");

}

void ExpertDiscoveryPosNegMrkDialog::accept(){
    Q_ASSERT(firstFileEdit);
    Q_ASSERT(secondFileEdit);
    Q_ASSERT(thirdFileEdit);

    firstFileName = firstFileEdit->text();
    secondFileName = secondFileEdit->text();
    thirdFileName = thirdFileEdit->text();

 
    if (!firstFileName.isEmpty() && !secondFileName.isEmpty()) {
        if(!generateDescr && thirdFileName.isEmpty()){
            QMessageBox mb(QMessageBox::Critical, tr("Select files"), tr("Select description file"));
            mb.exec();
        }else{
            QDialog::accept();
        }
    }else if(lettersCheck->isChecked() || !firstFileName.isEmpty()){
        QDialog::accept();
    }else{
        QMessageBox mb(QMessageBox::Critical, tr("Select files"), tr("Select positive markup file at least or chose 'Nucleotides markup'"));
        mb.exec();
    }
}

void ExpertDiscoveryPosNegMrkDialog::sl_oneSequence() {

    thirdFileEdit->setDisabled(oneSequenceCheckBox->isChecked());
    openThirdButton->setDisabled(oneSequenceCheckBox->isChecked());
    generateDescr = oneSequenceCheckBox->isChecked();

}

void ExpertDiscoveryPosNegMrkDialog::sl_lettersMarkup(){
    openFirstButton->setDisabled(lettersCheck->isChecked());
    firstFileEdit->setDisabled(lettersCheck->isChecked());
    openSecondButton->setDisabled(lettersCheck->isChecked());
    secondFileEdit->setDisabled(lettersCheck->isChecked());
    addToCurrentCheck->setDisabled(lettersCheck->isChecked());
}

void ExpertDiscoveryPosNegMrkDialog::sl_openFirstFile(){
    LastUsedDirHelper lod("ExpertDiscovery positive sequences markup file");
    lod.url = QFileDialog::getOpenFileName(NULL, tr("Open positive sequences markup file"), lod.dir, filter);

    Q_ASSERT(firstFileEdit);
    if (!lod.url.isEmpty()) {
        firstFileEdit->setText(lod.url);
    }
}
void ExpertDiscoveryPosNegMrkDialog::sl_openSecondFile() {

    LastUsedDirHelper lod("ExpertDiscovery negative sequences markup file");
    if (lod.dir.isEmpty()) {
        LastUsedDirHelper lodFirst("Open negative sequences file");

        lod.dir = lodFirst.dir;
    }
    lod.url = QFileDialog::getOpenFileName(NULL, tr("Open second file"), lod.dir, filter);

    Q_ASSERT(secondFileEdit);
    if (!lod.url.isEmpty()) {
        secondFileEdit->setText(lod.url);
    }
}

void ExpertDiscoveryPosNegMrkDialog::sl_openThirdFile() {

    LastUsedDirHelper lod("ExpertDiscovery description file");
    if (lod.dir.isEmpty()) {
        LastUsedDirHelper lodFirst("ExpertDiscovery description file");

        lod.dir = lodFirst.dir;
    }
    lod.url = QFileDialog::getOpenFileName(NULL, tr("Open description file"), lod.dir, filter);

    Q_ASSERT(thirdFileEdit);
    if (!lod.url.isEmpty()) {
        thirdFileEdit->setText(lod.url);
    }
}

}//namespace


