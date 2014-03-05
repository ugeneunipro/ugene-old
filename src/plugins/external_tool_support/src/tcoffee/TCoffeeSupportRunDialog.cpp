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

#include "TCoffeeSupportRunDialog.h"
#include <U2Core/DocumentUtils.h>
#include <U2Gui/LastUsedDirHelper.h>
#include <U2Core/GUrlUtils.h>

#include <U2Gui/DialogUtils.h>
#include <U2Gui/HelpButton.h>
#if (QT_VERSION < 0x050000) //Qt 5
#include <QtGui/QPushButton>
#include <QtGui/QFileDialog>
#include <QtGui/QToolButton>
#include <QtGui/QMessageBox>
#else
#include <QtWidgets/QPushButton>
#include <QtWidgets/QFileDialog>
#include <QtWidgets/QToolButton>
#include <QtWidgets/QMessageBox>
#endif

namespace U2 {

////////////////////////////////////////
//TCoffeeSupportRunDialog
TCoffeeSupportRunDialog::TCoffeeSupportRunDialog(TCoffeeSupportTaskSettings& _settings, QWidget* _parent) :
        QDialog(_parent), settings(_settings)
{
    setupUi(this);
    new HelpButton(this, buttonBox, "4228545");
    buttonBox->button(QDialogButtonBox::Ok)->setText(tr("Align"));
    buttonBox->button(QDialogButtonBox::Cancel)->setText(tr("Cancel"));

    inputGroupBox->setVisible(false);
    this->adjustSize();
    QPushButton* cancelButton = buttonBox->button(QDialogButtonBox::Cancel);
    QPushButton* alignButton = buttonBox->button(QDialogButtonBox::Ok);
    connect(cancelButton,SIGNAL(clicked()),this,SLOT(reject()));
    connect(alignButton,SIGNAL(clicked()),this,SLOT(sl_align()));

}

void TCoffeeSupportRunDialog::sl_align(){
    if(gapOpenCheckBox->isChecked()) {
        settings.gapOpenPenalty = gapOpenSpinBox->value();
    }
    if(gapExtCheckBox->isChecked()) {
        settings.gapExtenstionPenalty = gapExtSpinBox->value();
    }
    if(maxNumberIterRefinementCheckBox->isChecked()){
        settings.numIterations = maxNumberIterRefinementSpinBox->value();
    }
    accept();
}
////////////////////////////////////////
//TCoffeeWithExtFileSpecifySupportRunDialog
TCoffeeWithExtFileSpecifySupportRunDialog::TCoffeeWithExtFileSpecifySupportRunDialog(TCoffeeSupportTaskSettings& _settings, QWidget* _parent) :
        QDialog(_parent), settings(_settings)
{
    setupUi(this);
    new HelpButton(this, buttonBox, "4228545");
    buttonBox->button(QDialogButtonBox::Ok)->setText(tr("Align"));
    buttonBox->button(QDialogButtonBox::Cancel)->setText(tr("Cancel"));

    //this->adjustSize();
    connect(inputFilePathButton, SIGNAL(clicked()), SLOT(sl_inputPathButtonClicked()));
    connect(outputFilePathButton, SIGNAL(clicked()), SLOT(sl_outputPathButtonClicked()));
    QPushButton* cancelButton = buttonBox->button(QDialogButtonBox::Cancel);
    QPushButton* alignButton = buttonBox->button(QDialogButtonBox::Ok);

    connect(cancelButton, SIGNAL(clicked()), this, SLOT(reject()));
    connect(alignButton, SIGNAL(clicked()), this, SLOT(sl_align()));
}

void TCoffeeWithExtFileSpecifySupportRunDialog::sl_inputPathButtonClicked() {
    LastUsedDirHelper lod;
    lod.url = QFileDialog::getOpenFileName(this, tr("Open an alignment file"), lod.dir, 
        DialogUtils::prepareDocumentsFileFilterByObjType(GObjectTypes::MULTIPLE_ALIGNMENT, true));
    if (lod.url.isEmpty()) {
        return;
    }
    inputFileLineEdit->setText(lod.url);
}

void TCoffeeWithExtFileSpecifySupportRunDialog::sl_outputPathButtonClicked() {
    LastUsedDirHelper lod;
    lod.url = QFileDialog::getSaveFileName(this, tr("Save an multiple alignment file"), lod.dir);
    if (lod.url.isEmpty()) {
        return;
    }
    outputFileLineEdit->setText(lod.url);
    buildMultipleAlignmentUrl(lod.url);
}

void TCoffeeWithExtFileSpecifySupportRunDialog::buildMultipleAlignmentUrl(const GUrl &alnUrl) {
    GUrl url = GUrlUtils::rollFileName(alnUrl.dirPath() + "/" + alnUrl.baseFileName()+ ".aln", DocumentUtils::getNewDocFileNameExcludesHint());
    outputFileLineEdit->setText(url.getURLString());
}

void TCoffeeWithExtFileSpecifySupportRunDialog::sl_align(){
    if(gapOpenCheckBox->isChecked()) {
        settings.gapOpenPenalty = gapOpenSpinBox->value();
    }
    if(gapExtCheckBox->isChecked()) {
        settings.gapExtenstionPenalty = gapExtSpinBox->value();
    }
    if(maxNumberIterRefinementCheckBox->isChecked()){
        settings.numIterations = maxNumberIterRefinementSpinBox->value();
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
