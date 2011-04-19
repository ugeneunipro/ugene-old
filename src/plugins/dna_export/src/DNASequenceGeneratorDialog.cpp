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

#include "DNASequenceGeneratorDialog.h"
#include "DNASequenceGenerator.h"

#include <U2Core/GObjectTypes.h>
#include <U2Core/BaseDocumentFormats.h>
#include <U2Core/AppContext.h>
#include <U2Core/DNAAlphabet.h>

#include <U2Gui/SaveDocumentGroupController.h>

#include <U2Misc/DialogUtils.h>

#include <QtGui/QFileDialog>
#include <QtGui/QMessageBox>


namespace U2 {

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
    conf.defaultFormatId = BaseDocumentFormats::PLAIN_FASTA;
    //conf.defaultFileName = defaultFileName;
    conf.saveTitle = tr("Save sequences");
    saveGroupContoller = new SaveDocumentGroupController(conf, this);

    connect(inputButton, SIGNAL(clicked()), SLOT(sl_browseReference()));
    connect(configureButton, SIGNAL(clicked()), SLOT(sl_configureContent()));
    connect(generateButton, SIGNAL(clicked()), SLOT(sl_generate()));
    connect(cancelButton, SIGNAL(clicked()), SLOT(reject()));
    connect(referenceButton, SIGNAL(toggled(bool)), SLOT(sl_refButtonToggled(bool)));
}

void DNASequenceGeneratorDialog::sl_browseReference() {
    LastOpenDirHelper lod;
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
    cfg.length = lengthSpin->value();
    cfg.numSeqs = seqNumSpin->value();
    cfg.useRef = referenceButton->isChecked();
    cfg.refUrl = inputEdit->text();
    cfg.outUrl = outputEdit->text();
    cfg.sequenceName = "Sequence ";
    cfg.format = saveGroupContoller->getFormatToSave();
    cfg.content = content;
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
    connect(saveButton, SIGNAL(clicked()), SLOT(sl_save()));
}

void BaseContentDialog::sl_save() {
    float percentA = percentASpin->value();
    float percentC = percentCSpin->value();
    float percentG = percentGSpin->value();
    float percentT = percentTSpin->value();
    float total = percentA + percentC + percentG + percentT;
    if (total != 100) {
        QMessageBox::critical(this, tr("Base content"), tr("Total percentage has to be 100 %"));
        return;
    }
    percentMap['A'] = percentA / 100.0;
    percentMap['C'] = percentC / 100.0;
    percentMap['G'] = percentG / 100.0;
    percentMap['T'] = percentT / 100.0;
    accept();
}

} //namespace
