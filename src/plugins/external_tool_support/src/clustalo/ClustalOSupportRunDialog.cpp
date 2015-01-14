/**
 * UGENE - Integrated Bioinformatics Tools.
 * Copyright (C) 2008-2015 UniPro <ugene@unipro.ru>
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

#include <QtCore/qglobal.h>
#if (QT_VERSION < 0x050000) //Qt 5
#include <QtGui/QMessageBox>
#include <QtGui/QPushButton>
#include <QtGui/QToolButton>
#else
#include <QtWidgets/QMessageBox>
#include <QtWidgets/QPushButton>
#include <QtWidgets/QToolButton>
#endif

#include <U2Core/AppContext.h>
#include <U2Core/AppResources.h>
#include <U2Core/AppSettings.h>
#include <U2Core/DNAAlphabet.h>
#include <U2Core/DocumentUtils.h>
#include <U2Core/GUrlUtils.h>

#include <U2Gui/DialogUtils.h>
#include <U2Gui/HelpButton.h>
#include <U2Gui/LastUsedDirHelper.h>
#include <U2Gui/U2FileDialog.h>

#include "ClustalOSupportRunDialog.h"

namespace U2 {
////////////////////////////////////////
//ClustalOSupportRunDialog
ClustalOSupportRunDialog::ClustalOSupportRunDialog(const MAlignment& _ma, ClustalOSupportTaskSettings& _settings, QWidget* _parent) :
        QDialog(_parent), ma(_ma), settings(_settings)
{
    setupUi(this);
    new HelpButton(this, buttonBox, "6062194");
    buttonBox->button(QDialogButtonBox::Ok)->setText(tr("Align"));
    buttonBox->button(QDialogButtonBox::Cancel)->setText(tr("Cancel"));

    inputGroupBox->setVisible(false);
    this->adjustSize();
    QPushButton* cancelButton = buttonBox->button(QDialogButtonBox::Cancel);
    QPushButton* alignButton = buttonBox->button(QDialogButtonBox::Ok);
    connect(cancelButton,SIGNAL(clicked()),this,SLOT(reject()));
    connect(alignButton,SIGNAL(clicked()),this,SLOT(sl_align()));
    numberOfCPUSpinBox->setMaximum(AppContext::getAppSettings()->getAppResourcePool()->getIdealThreadCount());
    numberOfCPUSpinBox->setValue(AppContext::getAppSettings()->getAppResourcePool()->getIdealThreadCount());

}

void ClustalOSupportRunDialog::sl_align(){
    if(iterationNumberCheckBox->isChecked()){
        settings.numIterations = iterationNumberSpinBox->value();
    }
    if(maxGTIterationsCheckBox->isChecked()){
        settings.maxGuidetreeIterations = maxGTIterationsSpinBox->value();
    }
    if(maxHMMIterationsCheckBox->isChecked()){
        settings.maxHMMIterations = maxHMMIterationsSpinBox->value();
    }
    settings.setAutoOptions = setAutoCheckBox->isChecked();
    settings.numberOfProcessors = numberOfCPUSpinBox->value();
    accept();
}

////////////////////////////////////////
//ClustalOWithExtFileSpecifySupportRunDialog
ClustalOWithExtFileSpecifySupportRunDialog::ClustalOWithExtFileSpecifySupportRunDialog(ClustalOSupportTaskSettings& _settings, QWidget* _parent) :
        QDialog(_parent), settings(_settings)
{
    setupUi(this);
    new HelpButton(this, buttonBox, "4227737");
    buttonBox->button(QDialogButtonBox::Ok)->setText(tr("Align"));
    buttonBox->button(QDialogButtonBox::Cancel)->setText(tr("Cancel"));

    //this->adjustSize();
    connect(inputFilePathButton, SIGNAL(clicked()), SLOT(sl_inputPathButtonClicked()));
    connect(outputFilePathButton, SIGNAL(clicked()), SLOT(sl_outputPathButtonClicked()));

    QPushButton* cancelButton = buttonBox->button(QDialogButtonBox::Cancel);
    QPushButton* alignButton = buttonBox->button(QDialogButtonBox::Ok);
    connect(cancelButton,SIGNAL(clicked()),this,SLOT(reject()));
    connect(alignButton,SIGNAL(clicked()),this,SLOT(sl_align()));

    numberOfCPUSpinBox->setMaximum(AppContext::getAppSettings()->getAppResourcePool()->getIdealThreadCount());
    numberOfCPUSpinBox->setValue(AppContext::getAppSettings()->getAppResourcePool()->getIdealThreadCount());
}
void ClustalOWithExtFileSpecifySupportRunDialog::sl_inputPathButtonClicked() {
    LastUsedDirHelper lod;
    lod.url = U2FileDialog::getOpenFileName(this, tr("Open an alignment file"), lod.dir,
        DialogUtils::prepareDocumentsFileFilterByObjType(GObjectTypes::MULTIPLE_ALIGNMENT, true));
    if (lod.url.isEmpty()) {
        return;
    }
    inputFileLineEdit->setText(lod.url);
}

void ClustalOWithExtFileSpecifySupportRunDialog::sl_outputPathButtonClicked() {
    LastUsedDirHelper lod;
    lod.url = U2FileDialog::getSaveFileName(this, tr("Save an multiple alignment file"), lod.dir);
    if (lod.url.isEmpty()) {
        return;
    }
    outputFileLineEdit->setText(lod.url);
    buildMultipleAlignmentUrl(lod.url);
}

void ClustalOWithExtFileSpecifySupportRunDialog::buildMultipleAlignmentUrl(const GUrl &alnUrl) {
    GUrl url = GUrlUtils::rollFileName(alnUrl.dirPath() + "/" + alnUrl.baseFileName()+ ".aln", DocumentUtils::getNewDocFileNameExcludesHint());
    outputFileLineEdit->setText(url.getURLString());
}
void ClustalOWithExtFileSpecifySupportRunDialog::sl_align(){
    if(iterationNumberCheckBox->isChecked()){
        settings.numIterations = iterationNumberSpinBox->value();
    }
    if(maxGTIterationsCheckBox->isChecked()){
        settings.maxGuidetreeIterations = maxGTIterationsSpinBox->value();
    }
    if(maxHMMIterationsCheckBox->isChecked()){
        settings.maxHMMIterations = maxHMMIterationsSpinBox->value();
    }
    settings.setAutoOptions = setAutoCheckBox->isChecked();
    settings.numberOfProcessors = numberOfCPUSpinBox->value();
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
