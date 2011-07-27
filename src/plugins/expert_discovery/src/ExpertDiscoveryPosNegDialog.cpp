#include "ExpertDiscoveryPosNegDialog.h"

#include <U2Core/GObjectTypes.h>
#include <U2Gui/DialogUtils.h>

#include <QtGui/QFileDialog>
#include <QtGui/QMessageBox>

namespace U2 {

ExpertDiscoveryPosNegDialog::ExpertDiscoveryPosNegDialog(QWidget *parent)
: QDialog(parent), generateNeg(false){

    setupUi(this);

    connect(openFirstButton, SIGNAL(clicked()), SLOT(sl_openFirstFile()));
    connect(openSecondButton, SIGNAL(clicked()), SLOT(sl_openSecondFile()));
    connect(oneSequenceCheckBox, SIGNAL(clicked()), SLOT(sl_oneSequence()));

    filter = DialogUtils::prepareDocumentsFileFilterByObjType(GObjectTypes::SEQUENCE, true);
}

void ExpertDiscoveryPosNegDialog::accept(){
    Q_ASSERT(firstFileEdit);
    Q_ASSERT(secondFileEdit);

    firstFileName = firstFileEdit->text();
    secondFileName = secondFileEdit->text();

    if (oneSequenceCheckBox->isChecked()) {
        secondFileName = firstFileName;
    }

    if (!firstFileName.isEmpty() && !secondFileName.isEmpty()) {
        if(secondFileName == firstFileName && !generateNeg){
            QMessageBox mb(QMessageBox::Critical, tr("Select files"), tr("Positive and negative sequences can't be in the one file. Select another file for one of them"));
            mb.exec();
        }else{
            QDialog::accept();
        }
    }else {
        QMessageBox mb(QMessageBox::Critical, tr("Select files"), tr("Select files for ExpertDiscovery"));
        mb.exec();
    }
}

void ExpertDiscoveryPosNegDialog::sl_oneSequence() {

    secondFileEdit->setDisabled(oneSequenceCheckBox->isChecked());
    openSecondButton->setDisabled(oneSequenceCheckBox->isChecked());
    generateNeg = oneSequenceCheckBox->isChecked();

}

void ExpertDiscoveryPosNegDialog::sl_openFirstFile(){
    LastOpenDirHelper lod("ExpertDiscovery positive sequences file");
    lod.url = QFileDialog::getOpenFileName(NULL, tr("Open positive sequences file"), lod.dir, filter);

    Q_ASSERT(firstFileEdit);
    if (!lod.url.isEmpty()) {
        firstFileEdit->setText(lod.url);
    }
}
void ExpertDiscoveryPosNegDialog::sl_openSecondFile() {

    LastOpenDirHelper lod("ExpertDiscovery negative sequences file");
    if (lod.dir.isEmpty()) {
        LastOpenDirHelper lodFirst("Open negative sequences file");

        lod.dir = lodFirst.dir;
    }
    lod.url = QFileDialog::getOpenFileName(NULL, tr("Open second file"), lod.dir, filter);

    Q_ASSERT(secondFileEdit);
    if (!lod.url.isEmpty()) {
        secondFileEdit->setText(lod.url);
    }
}

}//namespace


