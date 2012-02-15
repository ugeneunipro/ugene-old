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

#include "ExportChromatogramDialog.h"

#include <U2Core/AppContext.h>
#include <U2Core/Settings.h>
#include <U2Core/BaseDocumentFormats.h>
#include <U2Core/GUrlUtils.h>
#include <U2Core/DocumentUtils.h>
#include <U2Core/L10n.h>

#include <U2Gui/LastUsedDirHelper.h>
#include <U2Gui/SaveDocumentGroupController.h>

#include <QtGui/QMessageBox>
#include <QtGui/QFileDialog>

#define SETTINGS_ROOT QString("dna_export/")

namespace U2 {

ExportChromatogramDialog::ExportChromatogramDialog(QWidget* p, const GUrl& fileUrl): QDialog(p) {
    setupUi(this);    
    addToProjectFlag = true;

    //SaveDocumentGroupControllerConfig conf;
    GUrl newUrl = GUrlUtils::rollFileName(fileUrl.dirPath() + "/" + fileUrl.baseFileName() + "_copy.scf", DocumentUtils::getNewDocFileNameExcludesHint());
    fileNameEdit->setText( newUrl.getURLString() );
    formatCombo->addItem( BaseDocumentFormats::SCF.toUpper() );
    connect(fileButton, SIGNAL(clicked()),SLOT(sl_onBrowseClicked()) );
    
    
}

void ExportChromatogramDialog::sl_onBrowseClicked() {
    LastUsedDirHelper lod;
    QString filter;

    lod.url = QFileDialog::getSaveFileName(this, tr("Select a file"), lod.dir, "*.scf");
    if (lod.url.isEmpty()) {    
        return;
    }
    fileNameEdit->setText( lod.url );

}


void ExportChromatogramDialog::accept() {
    if (fileNameEdit->text().isEmpty()) {
        QMessageBox::critical(this, L10N::errorTitle(), tr("File name is empty!"));
        return;
    }
    
    url = fileNameEdit->text();
    addToProjectFlag = addToProjectBox->isChecked();
    reversed = reverseBox->isChecked();
    complemented = complementBox->isChecked();

    QDialog::accept();
}


}//namespace
