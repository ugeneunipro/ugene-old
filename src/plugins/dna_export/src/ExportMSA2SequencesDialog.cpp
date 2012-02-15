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

#include "ExportMSA2SequencesDialog.h"

#include <U2Core/AppContext.h>
#include <U2Core/Settings.h>
#include <U2Core/BaseDocumentFormats.h>
#include <U2Core/L10n.h>

#include <U2Gui/DialogUtils.h>
#include <U2Gui/SaveDocumentGroupController.h>

#include <QtGui/QMessageBox>
#include <QtGui/QFileDialog>

#define SETTINGS_ROOT QString("dna_export/")

namespace U2 {

ExportMSA2SequencesDialog::ExportMSA2SequencesDialog(QWidget* p): QDialog(p) {
    setupUi(this);    
    trimGapsFlag = false;
    addToProjectFlag = true;
    
    SaveDocumentGroupControllerConfig conf;
    conf.dfc.addFlagToExclude(DocumentFormatFlag_SingleObjectFormat);
    conf.dfc.addFlagToSupport(DocumentFormatFlag_SupportWriting);
    conf.dfc.supportedObjectTypes+=GObjectTypes::SEQUENCE;
    conf.fileDialogButton = fileButton;
    conf.formatCombo = formatCombo;
    conf.fileNameEdit = fileNameEdit;
    conf.parentWidget = this;
    conf.defaultFormatId = BaseDocumentFormats::FASTA;
    saveContoller = new SaveDocumentGroupController(conf, this);
}

void ExportMSA2SequencesDialog::accept() {
    if (fileNameEdit->text().isEmpty()) {
        QMessageBox::critical(this, L10N::errorTitle(), tr("File name is empty!"));
        return;
    }
    
    url = saveContoller->getSaveFileName();
    format = saveContoller->getFormatIdToSave();
    trimGapsFlag = trimGapsRB->isChecked();
    addToProjectFlag = addToProjectBox->isChecked();

    QDialog::accept();
}


}//namespace
