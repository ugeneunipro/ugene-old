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

#include "ExportSequences2MSADialog.h"

#include <U2Core/AppContext.h>
#include <U2Core/Settings.h>
#include <U2Core/BaseDocumentFormats.h>
#include <U2Gui/DialogUtils.h>
#include <U2Gui/HelpButton.h>
#include <U2Gui/SaveDocumentGroupController.h>
#include <U2Core/L10n.h>
#include <QtGui/QPushButton>

#include <QtGui/QMessageBox>
#include <QtGui/QFileDialog>

#define SETTINGS_ROOT QString("dna_export/")

namespace U2 {

ExportSequences2MSADialog::ExportSequences2MSADialog(QWidget* p, const QString& defaultUrl): QDialog(p) {
    setupUi(this);    
    new HelpButton(this, buttonBox, "4227244");
    buttonBox->button(QDialogButtonBox::Ok)->setText(tr("Export"));
    buttonBox->button(QDialogButtonBox::Cancel)->setText(tr("Cancel"));
    okButton = buttonBox->button(QDialogButtonBox::Ok);

    addToProjectFlag = true;
    useGenbankHeader = false;

    SaveDocumentGroupControllerConfig conf;
    conf.dfc.addFlagToSupport(DocumentFormatFlag_SupportWriting);
    conf.dfc.supportedObjectTypes+=GObjectTypes::MULTIPLE_ALIGNMENT;
    conf.fileDialogButton = fileButton;
    conf.formatCombo = formatCombo;
    conf.fileNameEdit = fileNameEdit;
    conf.parentWidget = this;
    conf.defaultFileName = defaultUrl;
    conf.defaultFormatId = BaseDocumentFormats::CLUSTAL_ALN;
    saveContoller = new SaveDocumentGroupController(conf, this);

}


void ExportSequences2MSADialog::accept() {
    if (fileNameEdit->text().isEmpty()) {
        QMessageBox::critical(this, L10N::errorTitle(), tr("File name is empty!"));
        return;
    }
    
    url = saveContoller->getSaveFileName();
    format = saveContoller->getFormatIdToSave();
    addToProjectFlag = addToProjectBox->isChecked();
    useGenbankHeader = genbankBox->isChecked();

    QDialog::accept();
}

void ExportSequences2MSADialog::setOkButtonText(const QString& text) const {
    okButton->setText(text);
}

void ExportSequences2MSADialog::setFileLabelText(const QString& text) const {
    fileLabel->setText(text);
}

}//namespace
