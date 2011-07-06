/**
 * UGENE - Integrated Bioinformatics Tools.
 * Copyright (C) 2008-2011 UniPro <ugene@unipro.ru>
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

namespace U2 {
////////////////////////////////////////
//CAP3SupportDialog

CAP3SupportDialog::CAP3SupportDialog(CAP3SupportTaskSettings& s, QWidget* parent) 
: QDialog(parent), settings(s)
{
    setupUi(this);
    
    QString outputUrl;
    outputPathLineEdit->setText(outputUrl);

    connect(addButton, SIGNAL(clicked()), SLOT(sl_onAddButtonClicked()));
    connect(removeButton, SIGNAL(clicked()), SLOT(sl_onRemoveButtonClicked()));
    connect(removeAllButton, SIGNAL(clicked()), SLOT(sl_onRemoveAllButtonClicked()));
    connect(specifyOutputPathButton, SIGNAL(clicked()), SLOT(sl_onSpecifyOutputPathButtonClicked()));

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

    if (outputPathLineEdit->text().isEmpty() ) {
        QMessageBox::information(this, windowTitle(),
            tr("Result contig file name is not set!") );
        return;
    } 
    
    settings.outputFilePath = outputPathLineEdit->text();
    settings.bandExpansionSize = bandExpansionBox->value();
    settings.baseQualityClipCutoff = baseQualityClipCutoffBox->value();
    settings.baseQualityDiffCutoff = baseQualityDiffCutoffBox->value();
    settings.maxQScoreSum = maxQScoreDiffBox->value();
    settings.gapPenaltyFactor = gapPenaltyFactorBox->value();
    settings.maxGapLength = maxGapLengthBox->value();
    settings.matchScoreFactor = matchScoreFactorBox->value();
    settings.mismatchScoreFactor = mismatchScoreFactorBox->value();
    settings.overlapPercentIdentetyCutoff = overlapPercentIdentityBox->value();
    settings.overlapLengthCutoff = overlapLengthCutoffBox->value();
    settings.overlapSimilarityScoreCutoff = overlapSimilarityScoreCutoffBox->value();
    settings.maxNumberOfWordMatches = maxNumWordMatchesBox->value();
    settings.reverseReads = reverseReadsBox->isChecked();

    QDialog::accept();
}

void CAP3SupportDialog::sl_onAddButtonClicked()
{
    LastOpenDirHelper lod;
    QStringList fileNames = QFileDialog::getOpenFileNames(this, tr("Add sequences to assembly"), lod.dir);
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
    
    LastOpenDirHelper lod;
    lod.url = QFileDialog::getSaveFileName(this, tr("Set result contig file name"), lod.dir, tr("ACE format (*.ace)"));
    if (!lod.url.isEmpty()) {
        GUrl result = lod.url;
        if (result.lastFileSuffix().isEmpty()) {
            result = QString( "%1.ace" ).arg( result.getURLString() );
        }
        outputPathLineEdit->setText(result.getURLString());
    }
    
}


}//namespace
