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

#include "ExportAnnotationsDialog.h"


#include <U2Core/global.h>
#include <U2Core/DocumentModel.h>
#include <U2Core/AppContext.h>

#include <U2Gui/LastUsedDirHelper.h>

#include <QtCore/QList>
#include <QtGui/QFileDialog>

namespace U2 {

const QString ExportAnnotationsDialog::CSV_FORMAT_ID("csv");

ExportAnnotationsDialog::ExportAnnotationsDialog(const QString & filename, QWidget *parent) 
: QDialog(parent) {
    setupUi(this);
    
    fileNameEdit->setText(filename);
    connect(chooseFileButton, SIGNAL(clicked()), SLOT(sl_onChooseFileButtonClicked()));
    
    DocumentFormatConstraints constr;
    QList<DocumentFormatId> supportedFormats;
    constr.supportedObjectTypes.insert(GObjectTypes::ANNOTATION_TABLE);
    constr.addFlagToSupport(DocumentFormatFlag_SupportWriting);
    supportedFormats = AppContext::getDocumentFormatRegistry()->selectFormats(constr);
    supportedFormats.append(CSV_FORMAT_ID);
    supportedFormatsExts.append(CSV_FORMAT_ID);
    
    foreach(const DocumentFormatId & fid, supportedFormats) {
        formatsBox->addItem(fid);
        if(fid != CSV_FORMAT_ID) {
            DocumentFormat * df = AppContext::getDocumentFormatRegistry()->getFormatById(fid);
            assert(df != NULL);
            supportedFormatsExts.append(df->getSupportedDocumentFileExtensions().first());
        }
    }
    formatsBox->setCurrentIndex(formatsBox->findText(supportedFormats.first()));
    connect(formatsBox, SIGNAL(currentIndexChanged(const QString &)), SLOT(sl_onFormatChanged(const QString &)));
    sl_onFormatChanged(formatsBox->currentText());
}

void ExportAnnotationsDialog::sl_onFormatChanged(const QString & newFormat) {
    exportSequenceCheck->setEnabled(newFormat == CSV_FORMAT_ID);
    QString ext(".");
    if(newFormat == CSV_FORMAT_ID) {
        ext.append(CSV_FORMAT_ID);
    } else {
        DocumentFormat * df = AppContext::getDocumentFormatRegistry()->getFormatById(newFormat);
        assert(df != NULL);
        ext.append(df->getSupportedDocumentFileExtensions().first());
    }
    assert(!ext.isEmpty());
    QFileInfo fi(fileNameEdit->text());
    fileNameEdit->setText(QDir::cleanPath(fi.absoluteDir().absolutePath() + "/" + fi.completeBaseName() + ext));
}

QString ExportAnnotationsDialog::filePath() const {
    return fileNameEdit->text();
}

bool ExportAnnotationsDialog::exportSequence()const {
    return exportSequenceCheck->isChecked();
}

void ExportAnnotationsDialog::changeEvent(QEvent *e) {
    QDialog::changeEvent(e);
    switch (e->type()) {
        case QEvent::LanguageChange:
            retranslateUi(this);
            break;
        default:
            break;
    }
}

void ExportAnnotationsDialog::sl_onChooseFileButtonClicked() {
    QString curFormatExt;
    {
        QString curFormat = formatsBox->currentText();
        if(curFormat == CSV_FORMAT_ID) {
            curFormatExt = CSV_FORMAT_ID;
        } else {
            DocumentFormat * df = AppContext::getDocumentFormatRegistry()->getFormatById(curFormat);
            assert(df != NULL);
            curFormatExt = df->getSupportedDocumentFileExtensions().first();
        }
    }
    
    QList<QString> formats(supportedFormatsExts);
    formats.removeAll(curFormatExt);
    formats.prepend(curFormatExt);
    QString fileFormats;
    for(int i = 0; i < formats.size(); ++i) {
        QString formatName = formats.at(i);
        fileFormats += formatName.toUpper() + " format (*." + formatName + ");;";
    }
    fileFormats.append("All files (*)");
    
    LastUsedDirHelper helper("ExportAnnotationsDialogHelperDomain");
    helper.url = QFileDialog::getSaveFileName(this, tr("Select file to save annotations"), helper.dir, fileFormats, 
        NULL, QFileDialog::DontConfirmOverwrite);
    if(!helper.url.isEmpty()) {
        fileNameEdit->setText(helper.url);
        sl_onFormatChanged(formatsBox->currentText());
    }
}

void ExportAnnotationsDialog::setExportSequenceVisible( bool value ) {   
    exportSequenceCheck->setVisible(value);
}

QString ExportAnnotationsDialog::fileFormat() const {
    return formatsBox->currentText();
}

} // namespace U2
