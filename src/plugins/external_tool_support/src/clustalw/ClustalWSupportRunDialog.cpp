/**
 * UGENE - Integrated Bioinformatics Tools.
 * Copyright (C) 2008-2014 UniPro <ugene@unipro.ru>
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

#include "ClustalWSupportRunDialog.h"
#include <U2Core/DNAAlphabet.h>
#include <U2Gui/DialogUtils.h>
#include <U2Core/DocumentUtils.h>
#include <QtGui/QMessageBox>

#include <U2Gui/LastUsedDirHelper.h>
#include <QtGui/QFileDialog>
#include <QtGui/QToolButton>
#include <U2Core/GUrlUtils.h>

namespace U2 {
////////////////////////////////////////
//ClustalWSupportRunDialog
ClustalWSupportRunDialog::ClustalWSupportRunDialog(const MAlignment& _ma, ClustalWSupportTaskSettings& _settings, QWidget* _parent) :
        QDialog(_parent), ma(_ma), settings(_settings)
{
    setupUi(this);
    inputGroupBox->setVisible(false);
    this->adjustSize();
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
    this->adjustSize();
    connect(inputFilePathButton, SIGNAL(clicked()), SLOT(sl_inputPathButtonClicked()));
    connect(outputFilePathButton, SIGNAL(clicked()), SLOT(sl_outputPathButtonClicked()));

    connect(this->cancelButton,SIGNAL(clicked()),this,SLOT(reject()));
    connect(this->alignButton,SIGNAL(clicked()),this,SLOT(sl_align()));

    proteinGapParamGroupBox->setEnabled(true);
    weightMatrixComboBox->insertSeparator(2);
    weightMatrixComboBox->addItem("BLOSUM");
    weightMatrixComboBox->addItem("PAM");
    weightMatrixComboBox->addItem("GONNET");
    weightMatrixComboBox->addItem("ID");
}

void ClustalWWithExtFileSpecifySupportRunDialog::sl_inputPathButtonClicked() {
    LastUsedDirHelper lod;
    lod.url = QFileDialog::getOpenFileName(this, tr("Open an alignment file"), lod.dir, 
        DialogUtils::prepareDocumentsFileFilterByObjType(GObjectTypes::MULTIPLE_ALIGNMENT, true));
    if (lod.url.isEmpty()) {
        return;
    }
    inputFileLineEdit->setText(lod.url);
}

void ClustalWWithExtFileSpecifySupportRunDialog::sl_outputPathButtonClicked() {
    LastUsedDirHelper lod;
    lod.url = QFileDialog::getSaveFileName(this, tr("Save an multiple alignment file"), lod.dir);
    if (lod.url.isEmpty()) {
        return;
    }
    outputFileLineEdit->setText(lod.url);
    buildMultipleAlignmentUrl(lod.url);
}

void ClustalWWithExtFileSpecifySupportRunDialog::buildMultipleAlignmentUrl(const GUrl &alnUrl) {
    GUrl url = GUrlUtils::rollFileName(alnUrl.dirPath() + "/" + alnUrl.baseFileName()+ ".aln", DocumentUtils::getNewDocFileNameExcludesHint());
    outputFileLineEdit->setText(url.getURLString());
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
        }    
    if(inputFileLineEdit->text().isEmpty()){
        QMessageBox::information(this, tr("Kalign with Align"),
            tr("Input file is not set!") );
        }else if(outputFileLineEdit->text().isEmpty()){
            QMessageBox::information(this, tr("Kalign with Align"),
                tr("Output file is not set!") );
        }
        else{
            settings.outputFilePath=outputFileLineEdit->text();
            settings.inputFilePath=inputFileLineEdit->text();
            QDialog::accept();
            }

}

}//namespace
