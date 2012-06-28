/**
 * UGENE - Integrated Bioinformatics Tools.
 * Copyright (C) 2008-2012 UniPro <ugene@unipro.ru>
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

#include "ClustalOSupportRunDialog.h"
#include <U2Core/AppContext.h>
#include <U2Core/AppSettings.h>
#include <U2Core/AppResources.h>
#include <U2Core/DNAAlphabet.h>
#include <U2Gui/DialogUtils.h>

#include <QtGui/QFileDialog>
#include <QtGui/QToolButton>

namespace U2 {
////////////////////////////////////////
//ClustalOSupportRunDialog
ClustalOSupportRunDialog::ClustalOSupportRunDialog(const MAlignment& _ma, ClustalOSupportTaskSettings& _settings, QWidget* _parent) :
        QDialog(_parent), ma(_ma), settings(_settings)
{
    setupUi(this);
    connect(this->cancelButton,SIGNAL(clicked()),this,SLOT(reject()));
    connect(this->alignButton,SIGNAL(clicked()),this,SLOT(sl_align()));
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
    QWidget * widget = new QWidget(_parent);
    inputFileLineEdit= new FileLineEdit(DialogUtils::prepareDocumentsFileFilterByObjType(GObjectTypes::MULTIPLE_ALIGNMENT, true),
        "", false, widget);
    inputFileLineEdit->setText("");
    QToolButton * selectToolPathButton = new QToolButton(widget);
    selectToolPathButton->setVisible(true);
    selectToolPathButton->setText("...");
    connect(selectToolPathButton, SIGNAL(clicked()), inputFileLineEdit, SLOT(sl_onBrowse()));
    connect(inputFileLineEdit,SIGNAL(textChanged(QString)),this, SLOT(sl_inputFileLineEditChanged(QString)));

    QHBoxLayout* layout = new QHBoxLayout(widget);
    layout->addWidget(inputFileLineEdit);
    layout->addWidget(selectToolPathButton);

    QGroupBox* inputFileGroupBox=new QGroupBox(tr("Select input file"),widget);
    inputFileGroupBox->setLayout(layout);
    QBoxLayout* parentLayout = qobject_cast<QBoxLayout*>(this->layout());
    assert(parentLayout);
    parentLayout->insertWidget(0, inputFileGroupBox);
    alignButton->setEnabled(false);

    connect(this->cancelButton,SIGNAL(clicked()),this,SLOT(reject()));
    connect(this->alignButton,SIGNAL(clicked()),this,SLOT(sl_align()));

    numberOfCPUSpinBox->setMaximum(AppContext::getAppSettings()->getAppResourcePool()->getIdealThreadCount());
    numberOfCPUSpinBox->setValue(AppContext::getAppSettings()->getAppResourcePool()->getIdealThreadCount());
}
void ClustalOWithExtFileSpecifySupportRunDialog::sl_inputFileLineEditChanged(const QString& str){
    alignButton->setEnabled(!str.isEmpty());
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

    if(!inputFileLineEdit->text().isEmpty()){
        settings.inputFilePath=inputFileLineEdit->text();
    }else{
        assert(NULL);
        reject();
    }
    accept();
}

}//namespace
