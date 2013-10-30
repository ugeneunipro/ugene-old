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

#include "CAP3SupportDialog.h"
#include "CAP3SupportTask.h"

#include <QtGui/QMessageBox>
#include <QtGui/QFileDialog>

#include <U2Gui/LastUsedDirHelper.h>


namespace U2 {
////////////////////////////////////////
//CAP3SupportDialog

CAP3SupportDialog::CAP3SupportDialog(CAP3SupportTaskSettings& s, QWidget* parent) 
: QDialog(parent), settings(s)
{
    setupUi(this);

    tabWidget->setCurrentIndex(0);
    QString outputUrl;
    outputPathLineEdit->setText(outputUrl);

    connect(addButton, SIGNAL(clicked()), SLOT(sl_onAddButtonClicked()));
    connect(removeButton, SIGNAL(clicked()), SLOT(sl_onRemoveButtonClicked()));
    connect(removeAllButton, SIGNAL(clicked()), SLOT(sl_onRemoveAllButtonClicked()));
    connect(specifyOutputPathButton, SIGNAL(clicked()), SLOT(sl_onSpecifyOutputPathButtonClicked()));

    initSettings();
}

void CAP3SupportDialog::initSettings() {
    bandExpansionBox->setValue(settings.bandExpansionSize);
    baseQualityDiffCutoffBox->setValue(settings.baseQualityDiffCutoff);
    baseQualityClipCutoffBox->setValue(settings.baseQualityClipCutoff);
    maxQScoreDiffBox->setValue(settings.maxQScoreSum);
    maxGapLengthBox->setValue(settings.maxGapLength);
    gapPenaltyFactorBox->setValue(settings.gapPenaltyFactor);
    matchScoreFactorBox->setValue(settings.matchScoreFactor);
    mismatchScoreFactorBox->setValue(settings.mismatchScoreFactor);
    overlapLengthCutoffBox->setValue(settings.overlapLengthCutoff);
    overlapPercentIdentityBox->setValue(settings.overlapPercentIdentityCutoff);
    if (settings.reverseReads) {
        reverseReadsBox->setChecked(true);
    }
    else {
        reverseReadsBox->setChecked(false);
    }
    overlapSimilarityScoreCutoffBox->setValue(settings.overlapSimilarityScoreCutoff);
    maxNumWordMatchesBox->setValue(settings.maxNumberOfWordMatches);
    clippingRangeBox->setValue(settings.clippingRange);
}

void CAP3SupportDialog::accept()
{
    if (seqList->count() == 0) {
        QMessageBox::information(this, windowTitle(),
            tr("List of input files is empty!") );
        return;
    }
    
    int itemCount = seqList->count();
    for (int i = 0; i < itemCount; ++i) {
        settings.inputFiles.append( seqList->item(i)->text() );
    }

    QString outputPath = outputPathLineEdit->text();
    if (outputPath.isEmpty() ) {
        QMessageBox::information(this, windowTitle(),
            tr("Result contig file name is not set!") );
        return;
    }

    if(QFileInfo(outputPath).exists()) {
        int result = QMessageBox::question(this, windowTitle(),
                                           tr("Destination file already exists.\n"
                                              "To overwrite the file, press 'Replace'.\n"
                                              "To save under other name press 'Cancel' and change name in 'Result contig' field."),
                                           tr("Replace"),
                                           tr("Cancel"));
        if(result == 1) {
            return;
        }
    }
    
    settings.outputFilePath = outputPath;
    settings.bandExpansionSize = bandExpansionBox->value();
    settings.baseQualityClipCutoff = baseQualityClipCutoffBox->value();
    settings.baseQualityDiffCutoff = baseQualityDiffCutoffBox->value();
    settings.maxQScoreSum = maxQScoreDiffBox->value();
    settings.gapPenaltyFactor = gapPenaltyFactorBox->value();
    settings.maxGapLength = maxGapLengthBox->value();
    settings.matchScoreFactor = matchScoreFactorBox->value();
    settings.mismatchScoreFactor = mismatchScoreFactorBox->value();
    settings.overlapPercentIdentityCutoff = overlapPercentIdentityBox->value();
    settings.overlapLengthCutoff = overlapLengthCutoffBox->value();
    settings.overlapSimilarityScoreCutoff = overlapSimilarityScoreCutoffBox->value();
    settings.maxNumberOfWordMatches = maxNumWordMatchesBox->value();
    settings.reverseReads = reverseReadsBox->isChecked();
    settings.clippingRange = clippingRangeBox->value();

    QDialog::accept();
}

void CAP3SupportDialog::sl_onAddButtonClicked()
{
    LastUsedDirHelper lod;
    QStringList fileNames = QFileDialog::getOpenFileNames(this, tr("Add Sequences to Assembly"), lod.dir);
    if (fileNames.isEmpty()) {
        return;
    }
    lod.url = fileNames.at(fileNames.count() - 1);
    foreach(const QString& f, fileNames) {
        seqList->addItem(f);    
    }


    GUrl url( seqList->item(0)->text());
    outputPathLineEdit->setText(url.dirPath() + "/" + url.baseFileName() + ".cap.ace" );

}

void CAP3SupportDialog::sl_onRemoveButtonClicked()
{
    int currentRow = seqList->currentRow();
    seqList->takeItem(currentRow);

}

void CAP3SupportDialog::sl_onRemoveAllButtonClicked()
{
    seqList->clear();
}

void CAP3SupportDialog::sl_onSpecifyOutputPathButtonClicked()
{
    
    LastUsedDirHelper lod;
    lod.url = QFileDialog::getSaveFileName(this, tr("Set Result Contig File Name"), lod.dir, tr("ACE format (*.ace)"));
    if (!lod.url.isEmpty()) {
        GUrl result = lod.url;
        if (result.lastFileSuffix().isEmpty()) {
            result = QString( "%1.ace" ).arg( result.getURLString() );
        }
        outputPathLineEdit->setText(result.getURLString());
    }
    
}


}//namespace
