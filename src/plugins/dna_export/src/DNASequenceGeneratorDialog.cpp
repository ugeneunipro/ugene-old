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

#include "DNASequenceGeneratorDialog.h"
#include "DNASequenceGenerator.h"

#include <U2Core/GObjectTypes.h>
#include <U2Core/BaseDocumentFormats.h>
#include <U2Core/AppContext.h>
#include <U2Core/DNAAlphabet.h>
#include <U2Core/Settings.h>

#include <U2Gui/SaveDocumentGroupController.h>
#include <U2Gui/LastUsedDirHelper.h>

#include <QtGui/QFileDialog>
#include <QtGui/QMessageBox>


namespace U2 {

#define ROOT_SETTING QString("dna_export/")
#define GCSKEW_SETTING QString("gc_skew")

static QMap<char, qreal> initContent() {
    QMap<char, qreal> res;
    res['A'] = 0.25;
    res['C'] = 0.25;
    res['G'] = 0.25;
    res['T'] = 0.25;
    return res;
}

QMap<char, qreal> DNASequenceGeneratorDialog::content = initContent();

DNASequenceGeneratorDialog::DNASequenceGeneratorDialog(QWidget* p) : QDialog(p) {
    setupUi(this);
    seedSpinBox->setEnabled(false);

    referenceButton->setChecked(true);
    sl_refButtonToggled(true);

    SaveDocumentGroupControllerConfig conf;
    conf.dfc.addFlagToExclude(DocumentFormatFlag_SingleObjectFormat);
    conf.dfc.addFlagToSupport(DocumentFormatFlag_SupportWriting);
    conf.dfc.supportedObjectTypes += GObjectTypes::SEQUENCE;
    conf.parentWidget = this;
    conf.fileNameEdit = outputEdit;
    conf.formatCombo = formatCombo;
    conf.fileDialogButton = outputButton;
    conf.defaultFormatId = BaseDocumentFormats::FASTA;
    //conf.defaultFileName = defaultFileName;
    conf.saveTitle = tr("Save sequences");
    saveGroupContoller = new SaveDocumentGroupController(conf, this);

    connect(inputButton, SIGNAL(clicked()), SLOT(sl_browseReference()));
    connect(configureButton, SIGNAL(clicked()), SLOT(sl_configureContent()));
    connect(generateButton, SIGNAL(clicked()), SLOT(sl_generate()));
    connect(cancelButton, SIGNAL(clicked()), SLOT(reject()));
    connect(referenceButton, SIGNAL(toggled(bool)), SLOT(sl_refButtonToggled(bool)));
    connect(seedCheckBox, SIGNAL(stateChanged (int)), SLOT(sl_stateChanged(int)));
}

void DNASequenceGeneratorDialog::sl_stateChanged(int state) {
    if(state == Qt::Checked)  {
        seedSpinBox->setEnabled(true);
    } else {
        seedSpinBox->setEnabled(false);
    }
}

void DNASequenceGeneratorDialog::sl_browseReference() {
    LastUsedDirHelper lod;
    QString filter = DNASequenceGenerator::prepareReferenceFileFilter();
    lod.url = QFileDialog::getOpenFileName(this, tr("Open file"), lod.dir, filter);
    inputEdit->setText(lod.url);
}

void DNASequenceGeneratorDialog::sl_configureContent() {
    BaseContentDialog bcDlg(content, this);
    bcDlg.exec();
}

void DNASequenceGeneratorDialog::sl_generate() {
    DNASequenceGeneratorConfig cfg;
    cfg.addToProj = addToProjCBox->isChecked();
    cfg.length = lengthSpin->value();
    cfg.numSeqs = seqNumSpin->value();
    cfg.useRef = referenceButton->isChecked();
    cfg.refUrl = inputEdit->text();
    cfg.outUrl = outputEdit->text();
    cfg.sequenceName = "Sequence ";
    cfg.format = saveGroupContoller->getFormatToSave();
    cfg.content = content;
    cfg.window = windowSpinBox->value();
    if(seedCheckBox->isChecked()) {
        cfg.seed = seedSpinBox->value();
    } else {
        cfg.seed = -1;
    }
    if(cfg.window > cfg.length) {
        QMessageBox::critical(this, tr("DNA Sequence Generator"), tr("Windows size bigger than sequence length"));
        return;
    }
        
    if (!cfg.useRef) {
        cfg.alphabet = AppContext::getDNAAlphabetRegistry()->findById(BaseDNAAlphabetIds::NUCL_DNA_DEFAULT());
    }

    if (cfg.refUrl.isEmpty() && cfg.useRef) {
        QMessageBox::critical(this, tr("DNA Sequence Generator"), tr("Reference url is not specified."));
        return;
    }

    if (cfg.outUrl.isEmpty()) {
        QMessageBox::critical(this, tr("DNA Sequence Generator"), tr("Output file is no specified."));
        return;
    }

    AppContext::getTaskScheduler()->registerTopLevelTask(new DNASequenceGeneratorTask(cfg));
    accept();
}

void DNASequenceGeneratorDialog::sl_refButtonToggled(bool checked) {
    assert(manualButton->isChecked() == !checked);
    inputEdit->setDisabled(!checked);
    inputButton->setDisabled(!checked);
    configureButton->setDisabled(checked);
}

BaseContentDialog::BaseContentDialog(QMap<char, qreal>& percentMap_, QWidget* p)
: QDialog(p), percentMap(percentMap_) {
    setupUi(this);
    percentASpin->setValue(percentMap.value('A')*100.0);
    percentCSpin->setValue(percentMap.value('C')*100.0);
    percentGSpin->setValue(percentMap.value('G')*100.0);
    percentTSpin->setValue(percentMap.value('T')*100.0);
    gcSkew = ((float)((int)(percentMap.value('G')*100) - (int)(percentMap.value('C')*100)))
        /((int)(percentMap.value('G')*100) + (int)(percentMap.value('C')*100));
    int iGCSkew = (int)(gcSkew * 100);
    gcSkew = (float(iGCSkew))/100.0;
    percentGCSpin->setValue(gcSkew);
    gcSkewPrev = gcSkew;
    
    connect(saveButton, SIGNAL(clicked()), SLOT(sl_save()));
    connect(baseContentRadioButton, SIGNAL(clicked()), SLOT(sl_baseClicked()));
    connect(gcSkewRadioButton, SIGNAL(clicked()), SLOT(sl_gcSkewClicked()));
    //baseContentRadioButton->setChecked(true);

    Settings *s = AppContext::getSettings();
    bool gc = s->getValue(ROOT_SETTING + GCSKEW_SETTING,false).toBool();
    if(gc) {
        percentASpin->setEnabled(false);
        percentCSpin->setEnabled(false);
        percentTSpin->setEnabled(false);
        percentGSpin->setEnabled(false);
        percentGCSpin->setEnabled(true);
    } else {
        percentASpin->setEnabled(true);
        percentCSpin->setEnabled(true);
        percentTSpin->setEnabled(true);
        percentGSpin->setEnabled(true);
        percentGCSpin->setEnabled(false);
    }
    baseContentRadioButton->setChecked(!gc);
    gcSkewRadioButton->setChecked(gc);
}

void BaseContentDialog::sl_baseClicked() {
    percentASpin->setEnabled(true);
    percentCSpin->setEnabled(true);
    percentTSpin->setEnabled(true);
    percentGSpin->setEnabled(true);
    percentGCSpin->setEnabled(false);
}

void BaseContentDialog::sl_gcSkewClicked() {
    percentASpin->setEnabled(false);
    percentCSpin->setEnabled(false);
    percentTSpin->setEnabled(false);
    percentGSpin->setEnabled(false);
    percentGCSpin->setEnabled(true);
}

void BaseContentDialog::sl_save() {
    float percentA;
    float percentC;
    float percentG;
    float percentT;
    if(baseContentRadioButton->isChecked()) {
        percentA = percentASpin->value();
        percentC = percentCSpin->value();
        percentG = percentGSpin->value();
        percentT = percentTSpin->value();
    } else {
        gcSkew = percentGCSpin->value();
        if(gcSkew != gcSkewPrev) {
            int percentAi = qrand();
            int percentCi = qrand();
            int percentTi = qrand();
            int percentGi = qrand();
            int sum = percentAi + percentCi + percentGi + percentTi;
            percentAi = (float)percentAi / sum * 100;
            percentGi = (float)percentGi / sum * 100;
            percentCi = (float)percentCi / sum * 100;
            percentTi = (float)percentTi / sum * 100;
            int CG = percentGi + percentCi;
            
            percentCi = (1 - gcSkew)* CG / 2;
            percentGi = percentCi + gcSkew * CG;
            if(percentCi < 0 || percentCi > 100 || percentGi < 0 || percentGi > 100) {
                QMessageBox::critical(this, tr("Base content"), tr("Incorrect GC Skew value"));
                return;
            }
            sum = percentAi + percentCi + percentGi + percentTi;
            percentAi += 100 - sum;

            percentA = percentAi;
            percentC = percentCi;
            percentG = percentGi;
            percentT = percentTi;
            percentASpin->setValue(percentAi);
            percentCSpin->setValue(percentCi);
            percentGSpin->setValue(percentGi);
            percentTSpin->setValue(percentTi);  
        } else {
            percentA = percentASpin->value();
            percentC = percentCSpin->value();
            percentG = percentGSpin->value();
            percentT = percentTSpin->value();
        }
    }
    float total = percentA + percentC + percentG + percentT;
    if (total != 100) {
        QMessageBox::critical(this, tr("Base content"), tr("Total percentage has to be 100 %"));
        return;
    }
    percentMap['A'] = percentA / 100.0;
    percentMap['C'] = percentC / 100.0;
    percentMap['G'] = percentG / 100.0;
    percentMap['T'] = percentT / 100.0;

    Settings *s = AppContext::getSettings();
    //bool gc = s->getValue("dna_export/gc_skew",false).toBool();
    s->setValue(ROOT_SETTING + GCSKEW_SETTING, gcSkewRadioButton->isChecked());

    accept();
}

} //namespace
