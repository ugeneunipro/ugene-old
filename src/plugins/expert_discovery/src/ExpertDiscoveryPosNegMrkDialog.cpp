#include "ExpertDiscoveryPosNegMrkDialog.h"

#include <U2Core/GObjectTypes.h>
#include <U2Gui/DialogUtils.h>

#include <QtGui/QFileDialog>
#include <QtGui/QMessageBox>

namespace U2 {

ExpertDiscoveryPosNegMrkDialog::ExpertDiscoveryPosNegMrkDialog(QWidget *parent)
: QDialog(parent), generateDescr(true){

    setupUi(this);

    connect(openFirstButton, SIGNAL(clicked()), SLOT(sl_openFirstFile()));
    connect(openSecondButton, SIGNAL(clicked()), SLOT(sl_openSecondFile()));
    connect(openThirdButton, SIGNAL(clicked()), SLOT(sl_openThirdFile()));
    connect(oneSequenceCheckBox, SIGNAL(clicked()), SLOT(sl_oneSequence()));

    oneSequenceCheckBox->click();

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
        if(secondFileName == firstFileName){
            QMessageBox mb(QMessageBox::Critical, tr("Select files"), tr("Positive and negative sequences markups can't be in the one file. Select another file for one of them"));
            mb.exec();
        }else if(!generateDescr && thirdFileName.isEmpty()){
            QMessageBox mb(QMessageBox::Critical, tr("Select files"), tr("Select description file"));
            mb.exec();
        }else{
            QDialog::accept();
        }
    }else {
        QMessageBox mb(QMessageBox::Critical, tr("Select files"), tr("Select files for ExpertDiscovery"));
        mb.exec();
    }
}

void ExpertDiscoveryPosNegMrkDialog::sl_oneSequence() {

    thirdFileEdit->setDisabled(oneSequenceCheckBox->isChecked());
    openThirdButton->setDisabled(oneSequenceCheckBox->isChecked());
    generateDescr = oneSequenceCheckBox->isChecked();

}

void ExpertDiscoveryPosNegMrkDialog::sl_openFirstFile(){
    LastOpenDirHelper lod("ExpertDiscovery positive sequences markup file");
    lod.url = QFileDialog::getOpenFileName(NULL, tr("Open positive sequences markup file"), lod.dir, filter);

    Q_ASSERT(firstFileEdit);
    if (!lod.url.isEmpty()) {
        firstFileEdit->setText(lod.url);
    }
}
void ExpertDiscoveryPosNegMrkDialog::sl_openSecondFile() {

    LastOpenDirHelper lod("ExpertDiscovery negative sequences markup file");
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

void ExpertDiscoveryPosNegMrkDialog::sl_openThirdFile() {

    LastOpenDirHelper lod("ExpertDiscovery description file");
    if (lod.dir.isEmpty()) {
        LastOpenDirHelper lodFirst("ExpertDiscovery description file");

        lod.dir = lodFirst.dir;
    }
    lod.url = QFileDialog::getOpenFileName(NULL, tr("Open description file"), lod.dir, filter);

    Q_ASSERT(thirdFileEdit);
    if (!lod.url.isEmpty()) {
        thirdFileEdit->setText(lod.url);
    }
}

}//namespace


