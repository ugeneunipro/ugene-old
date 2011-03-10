#include "DotPlotFilesDialog.h"

#include <U2Core/GObjectTypes.h>
#include <U2Misc/DialogUtils.h>

#include <QtGui/QFileDialog>
#include <QtGui/QMessageBox>

namespace U2 {

DotPlotFilesDialog::DotPlotFilesDialog(QWidget *parent)
: QDialog(parent)
{
    setupUi(this);

    connect(openFirstButton, SIGNAL(clicked()), SLOT(sl_openFirstFile()));
    connect(openSecondButton, SIGNAL(clicked()), SLOT(sl_openSecondFile()));
    connect(oneSequenceCheckBox, SIGNAL(clicked()), SLOT(sl_oneSequence()));

    connect(mergeFirstCheckBox, SIGNAL(clicked()), SLOT(sl_mergeFirst()));
    connect(mergeSecondCheckBox, SIGNAL(clicked()), SLOT(sl_mergeSecond()));

    filter = DialogUtils::prepareDocumentsFileFilterByObjType(GObjectTypes::MULTIPLE_ALIGNMENT, true).append("\n").append(
        DialogUtils::prepareDocumentsFileFilterByObjType(GObjectTypes::SEQUENCE, false));
}

void DotPlotFilesDialog::sl_oneSequence() {

    secondFileEdit->setDisabled(oneSequenceCheckBox->isChecked());
    openSecondButton->setDisabled(oneSequenceCheckBox->isChecked());

    mergeSecondCheckBox->setDisabled(oneSequenceCheckBox->isChecked());
    gapSecond->setDisabled(oneSequenceCheckBox->isChecked() || !mergeSecondCheckBox->isChecked());
}

void DotPlotFilesDialog::sl_mergeFirst() {
    gapFirst->setDisabled(!mergeFirstCheckBox->isChecked());
}

void DotPlotFilesDialog::sl_mergeSecond() {
    gapSecond->setDisabled(!mergeSecondCheckBox->isChecked());
}

// open first file button clicked
void DotPlotFilesDialog::sl_openFirstFile() {

    LastOpenDirHelper lod("DotPlot first file");
    lod.url = QFileDialog::getOpenFileName(NULL, tr("Open first file"), lod.dir, filter);

    Q_ASSERT(firstFileEdit);
    if (!lod.url.isEmpty()) {
        firstFileEdit->setText(lod.url);
    }
}

// open second file button clicked
void DotPlotFilesDialog::sl_openSecondFile() {

    LastOpenDirHelper lod("DotPlot second file");
    if (lod.dir.isEmpty()) {
        LastOpenDirHelper lodFirst("DotPlot first file");

        lod.dir = lodFirst.dir;
    }
    lod.url = QFileDialog::getOpenFileName(NULL, tr("Open second file"), lod.dir, filter);

    Q_ASSERT(secondFileEdit);
    if (!lod.url.isEmpty()) {
        secondFileEdit->setText(lod.url);
    }
}

// ok button clicked
void DotPlotFilesDialog::accept() {

    Q_ASSERT(firstFileEdit);
    Q_ASSERT(secondFileEdit);

    firstFileName = firstFileEdit->text();
    secondFileName = secondFileEdit->text();

    if (oneSequenceCheckBox->isChecked()) {
        secondFileName = firstFileName;
    }

    if (!firstFileName.isEmpty() && !secondFileName.isEmpty()) {
        QDialog::accept();
    }
    else {
        QMessageBox mb(QMessageBox::Critical, tr("Select files"), tr("Select files first to build dotplot"));
        mb.exec();
    }
}

int DotPlotFilesDialog::getFirstGap() const {

    if (mergeFirstCheckBox->isChecked()) {
        return gapFirst->value();
    }
    else {
        return -1;
    }
}

int DotPlotFilesDialog::getSecondGap() const {

    if (mergeSecondCheckBox->isChecked()) {
        return gapSecond->value();
    }
    else {
        return -1;
    }
}

}//namespace
