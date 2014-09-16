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

#include <QtCore/qglobal.h>
#if (QT_VERSION < 0x050000) //Qt 5
#include <QtGui/QMessageBox>
#include <QtGui/QPushButton>
#else
#include <QtWidgets/QMessageBox>
#include <QtWidgets/QPushButton>
#endif

#include <U2Core/AppContext.h>
#include <U2Core/BaseDocumentFormats.h>
#include <U2Core/DocumentUtils.h>
#include <U2Core/GUrlUtils.h>
#include <U2Core/L10n.h>
#include <U2Core/Settings.h>

#include <U2Gui/HelpButton.h>
#include <U2Gui/LastUsedDirHelper.h>
#include <U2Gui/SaveDocumentGroupController.h>
#include <U2Gui/U2FileDialog.h>

#include "ExportChromatogramDialog.h"
#include "ExportUtils.h"

#define SETTINGS_ROOT QString("dna_export/")

namespace U2 {

ExportChromatogramDialog::ExportChromatogramDialog(QWidget* p, const GUrl& fileUrl): QDialog(p) {
    setupUi(this);    
    new HelpButton(this, buttonBox, "4227406");
    buttonBox->button(QDialogButtonBox::Ok)->setText(tr("Export"));
    buttonBox->button(QDialogButtonBox::Cancel)->setText(tr("Cancel"));

    addToProjectFlag = true;

    QString newUrl = GUrlUtils::getNewLocalUrlByExtention(fileUrl, "chromatogram", ".scf", "_copy");
    fileNameEdit->setText(newUrl);
    formatCombo->addItem(BaseDocumentFormats::SCF.toUpper());
    connect(fileButton, SIGNAL(clicked()), SLOT(sl_onBrowseClicked()));
}

void ExportChromatogramDialog::sl_onBrowseClicked() {
    LastUsedDirHelper lod;
    QString filter;

    lod.url = U2FileDialog::getSaveFileName(this, tr("Select a file"), lod.dir, "*.scf");
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
