/**
 * UGENE - Integrated Bioinformatics Tools.
 * Copyright (C) 2008-2013 UniPro <ugene@unipro.ru>
 * http://ugene.unipro.ru
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston,
 * MA 02110-1301, USA.
 */

#include "DotPlotFilesDialog.h"

#include <U2Core/GObjectTypes.h>
#include <U2Core/DocumentUtils.h>

#include <U2Gui/LastUsedDirHelper.h>
#include <U2Gui/DialogUtils.h>

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

    LastUsedDirHelper lod("DotPlot first file");
    lod.url = QFileDialog::getOpenFileName(NULL, tr("Open first file"), lod.dir, filter);

    Q_ASSERT(firstFileEdit);
    if (!lod.url.isEmpty()) {
        firstFileEdit->setText(lod.url);
        FormatDetectionConfig conf;
        conf.useImporters = true;
        conf.bestMatchesOnly = true;
        QList<FormatDetectionResult> results = DocumentUtils::detectFormat(lod.url, conf);
        if (results.isEmpty()){
            firstFileEdit->setText("");
            lod.url = "";
            return;
        }
        FormatDetectionResult format = results.at(0); //get moslty matched format
        bool multySeq = format.rawDataCheckResult.properties.value(RawDataCheckResult_MultipleSequences).toBool();
        if(multySeq){
            mergeFirstCheckBox->setChecked(true);
            sl_mergeFirst();            
        }
    }
}

// open second file button clicked
void DotPlotFilesDialog::sl_openSecondFile() {

    LastUsedDirHelper lod("DotPlot second file");
    if (lod.dir.isEmpty()) {
        LastUsedDirHelper lodFirst("DotPlot first file");

        lod.dir = lodFirst.dir;
    }
    lod.url = QFileDialog::getOpenFileName(NULL, tr("Open second file"), lod.dir, filter);

    Q_ASSERT(secondFileEdit);
    if (!lod.url.isEmpty()) {
        secondFileEdit->setText(lod.url);
        FormatDetectionConfig conf;
        conf.useImporters = true;
        conf.bestMatchesOnly = true;
        QList<FormatDetectionResult> results = DocumentUtils::detectFormat(lod.url, conf);
        if (results.isEmpty()){
            secondFileEdit->setText("");
            lod.url = "";
            return;
        }
        FormatDetectionResult format = results.at(0); //get moslty matched format
        bool multySeq = format.rawDataCheckResult.properties.value(RawDataCheckResult_MultipleSequences).toBool();
        if(multySeq){
            mergeSecondCheckBox->setChecked(true);
            sl_mergeSecond();            
        }
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

    FormatDetectionConfig conf;
    QList<FormatDetectionResult> results = DocumentUtils::detectFormat(firstFileName, conf);
    if (results.isEmpty()){
        QMessageBox mb(QMessageBox::Critical, tr("Select files"), tr("Unable to detect format of given file %1.\r\nSelect valid file to build dotplot").arg(firstFileEdit->text()));
        firstFileEdit->setText("");
        mb.exec();
        return;
    }

    results = DocumentUtils::detectFormat(secondFileName, conf);
    if (results.isEmpty()){
        QMessageBox mb(QMessageBox::Critical, tr("Select files"), tr("Unable to detect format of given file %1.\r\nSelect valid file to build dotplot").arg(secondFileEdit->text()));
        firstFileEdit->setText("");
        mb.exec();
        return;
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
