/**
 * UGENE - Integrated Bioinformatics Tools.
 * Copyright (C) 2008-2016 UniPro <ugene@unipro.ru>
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

#include <QMessageBox>
#include <QPushButton>
#include <QToolButton>

#include <U2Core/DocumentUtils.h>
#include <U2Core/GUrlUtils.h>

#include <U2Gui/DialogUtils.h>
#include <U2Gui/HelpButton.h>
#include <U2Gui/LastUsedDirHelper.h>
#include <U2Gui/SaveDocumentController.h>
#include <U2Gui/U2FileDialog.h>

#include "MAFFTSupportRunDialog.h"

namespace U2 {

////////////////////////////////////////
//MAFFTSupportRunDialog
MAFFTSupportRunDialog::MAFFTSupportRunDialog(MAFFTSupportTaskSettings& _settings, QWidget* _parent) :
    QDialog(_parent), settings(_settings)
{
    setupUi(this);
    new HelpButton(this, buttonBox, "17467797");
    buttonBox->button(QDialogButtonBox::Ok)->setText(tr("Align"));
    buttonBox->button(QDialogButtonBox::Cancel)->setText(tr("Cancel"));

    inputGroupBox->setVisible(false);
    this->adjustSize();
}

void MAFFTSupportRunDialog::accept(){
    if(gapOpenCheckBox->isChecked()) {
        settings.gapOpenPenalty = gapOpenSpinBox->value();
    }
    if(gapExtCheckBox->isChecked()) {
        settings.gapExtenstionPenalty = gapExtSpinBox->value();
    }
    if(maxNumberIterRefinementCheckBox->isChecked()){
        settings.maxNumberIterRefinement = maxNumberIterRefinementSpinBox->value();
    }
    QDialog::accept();
}

////////////////////////////////////////
//MAFFTWithExtFileSpecifySupportRunDialog
MAFFTWithExtFileSpecifySupportRunDialog::MAFFTWithExtFileSpecifySupportRunDialog(MAFFTSupportTaskSettings& _settings, QWidget* _parent) :
    QDialog(_parent),
    settings(_settings),
    saveController(NULL)
{
    setupUi(this);
    new HelpButton(this, buttonBox, "17467797");

    buttonBox->button(QDialogButtonBox::Ok)->setText(tr("Align"));
    buttonBox->button(QDialogButtonBox::Cancel)->setText(tr("Cancel"));

    initSaveController();

    connect(inputFilePathButton, SIGNAL(clicked()), SLOT(sl_inputPathButtonClicked()));
}

void MAFFTWithExtFileSpecifySupportRunDialog::sl_inputPathButtonClicked() {
    LastUsedDirHelper lod;
    lod.url = U2FileDialog::getOpenFileName(this, tr("Open an alignment file"), lod.dir,
        DialogUtils::prepareDocumentsFileFilterByObjType(GObjectTypes::MULTIPLE_ALIGNMENT, true));
    if (lod.url.isEmpty()) {
        return;
    }
    inputFileLineEdit->setText(lod.url);
}

void MAFFTWithExtFileSpecifySupportRunDialog::initSaveController() {
    SaveDocumentControllerConfig config;
    config.defaultFormatId = BaseDocumentFormats::CLUSTAL_ALN;
    config.fileDialogButton = outputFilePathButton;
    config.fileNameEdit = outputFileLineEdit;
    config.parentWidget = this;
    config.saveTitle = tr("Save an multiple alignment file");
    config.rollOutProjectUrls = true;

    const QList<DocumentFormatId> formats = QList<DocumentFormatId>() << BaseDocumentFormats::CLUSTAL_ALN;

    saveController = new SaveDocumentController(config, formats, this);
}

void MAFFTWithExtFileSpecifySupportRunDialog::accept(){
    if(gapOpenCheckBox->isChecked()) {
        settings.gapOpenPenalty = gapOpenSpinBox->value();
    }
    if(gapExtCheckBox->isChecked()) {
        settings.gapExtenstionPenalty = gapExtSpinBox->value();
    }
    if(maxNumberIterRefinementCheckBox->isChecked()){
        settings.maxNumberIterRefinement = maxNumberIterRefinementSpinBox->value();
    }
    if(inputFileLineEdit->text().isEmpty()){
        QMessageBox::information(this, tr("Kalign with Align"),
                                 tr("Input file is not set!") );
    }else if(saveController->getSaveFileName().isEmpty()){
        QMessageBox::information(this, tr("Kalign with Align"),
                                 tr("Output file is not set!") );
    }
    else{
        settings.outputFilePath = saveController->getSaveFileName();
        settings.inputFilePath = inputFileLineEdit->text();
        QDialog::accept();
    }
}

}//namespace
