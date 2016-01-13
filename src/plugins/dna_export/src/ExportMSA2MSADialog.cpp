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

#include "ExportMSA2MSADialog.h"

#include <U2Core/AppContext.h>
#include <U2Core/Settings.h>
#include <U2Core/BaseDocumentFormats.h>
#include <U2Gui/DialogUtils.h>
#include <U2Gui/GUIUtils.h>
#include <U2Gui/SaveDocumentGroupController.h>
#include <U2Gui/HelpButton.h>
#include <U2Core/L10n.h>
#if (QT_VERSION < 0x050000) //Qt 5
#include <QtGui/QPushButton>
#include <QtGui/QMessageBox>
#else
#include <QtWidgets/QPushButton>
#include <QtWidgets/QMessageBox>
#endif

#include <U2Core/DNAAlphabet.h>
#include <U2Core/DNATranslation.h>


namespace U2 {

ExportMSA2MSADialog::ExportMSA2MSADialog(const QString& defaultFileName, const DocumentFormatId& f, bool wholeAlignmentOnly, QWidget* p):  QDialog(p) {
    setupUi(this);
    new HelpButton(this, buttonBox, "17465963");
    buttonBox->button(QDialogButtonBox::Ok)->setText(tr("Export"));
    buttonBox->button(QDialogButtonBox::Cancel)->setText(tr("Cancel"));

    addToProjectFlag = true;

    SaveDocumentGroupControllerConfig conf;
    conf.dfc.addFlagToSupport(DocumentFormatFlag_SupportWriting);
    conf.dfc.supportedObjectTypes+=GObjectTypes::MULTIPLE_ALIGNMENT;
    conf.fileDialogButton = fileButton;
    conf.formatCombo = formatCombo;
    conf.fileNameEdit = fileNameEdit;
    conf.parentWidget = this;
    conf.defaultFileName = defaultFileName;
    conf.defaultFormatId = f;
    conf.saveTitle = tr("Export alignment");
    saveContoller = new SaveDocumentGroupController(conf, this);

    const DNAAlphabet* al = AppContext::getDNAAlphabetRegistry()->findById(BaseDNAAlphabetIds::NUCL_DNA_DEFAULT());
    DNATranslationRegistry* tr = AppContext::getDNATranslationRegistry();
    QList<DNATranslation*> aminoTs = tr->lookupTranslation(al, DNATranslationType_NUCL_2_AMINO);
    assert(!aminoTs.empty());
    foreach(DNATranslation* t, aminoTs) {
        translationCombo->addItem(t->getTranslationName());
        tableID.append(t->getTranslationId());
    }
    translationTable = tableID[translationCombo->currentIndex()];

    QPushButton* exportButton = buttonBox->button(QDialogButtonBox::Ok);
    connect(exportButton, SIGNAL(clicked()), SLOT(sl_exportClicked()));

    rangeGroupBox->setDisabled(wholeAlignmentOnly);

    int height = layout()->minimumSize().height();
    setMaximumHeight(height);

}

void ExportMSA2MSADialog::updateModel(){
    formatId = saveContoller->getFormatIdToSave();
    file = saveContoller->getSaveFileName();
    translationTable = tableID[translationCombo->currentIndex()];
    addToProjectFlag = addDocumentButton->isChecked();
    exportWholeAlignment = wholeRangeButton->isChecked();
}


void ExportMSA2MSADialog::sl_exportClicked() {
    if (fileNameEdit->text().isEmpty()) {
        QMessageBox::warning(this, L10N::warningTitle(), tr("File is empty"));
        fileNameEdit->setFocus();
        return;
    }
    updateModel();
    accept();
}

}//namespace
