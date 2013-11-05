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

#include "ImportDialogs.h"

#include <U2Core/AppContext.h>
#include <U2Core/BaseDocumentFormats.h>
#include <U2Core/DocumentModel.h>
#include <U2Core/GUrlUtils.h>
#include <U2Core/ProjectModel.h>
#include <U2Core/U2SafePoints.h>

#include <U2Formats/AceImporter.h>

#include <U2Gui/DialogUtils.h>
#include <U2Gui/ObjectViewModel.h>

#include <QtGui/QFileDialog>
#include <QtGui/QMessageBox>

namespace U2 {

void ImportDialogFactories::registerFactories() {
    DocumentFormatRegistry* dfRegistry = AppContext::getDocumentFormatRegistry();
    SAFE_POINT(dfRegistry, "Document format registry is NULL", );
    DocumentImportersRegistry* diRegistry = dfRegistry->getImportSupport();
    SAFE_POINT(diRegistry, "Document import registry is NULL", );

    DocumentImporter* aceImporter = diRegistry->getDocumentImporter(AceImporter::ID);
    SAFE_POINT(aceImporter, "ACE importer is NULL", );
    aceImporter->setDialogFactory(new AceImportDialogFactory());
}

const QString AceImportDialog::EXTENSION = ".ugenedb";

AceImportDialog::AceImportDialog(const QVariantMap& _settings) :
    ImportDialog(_settings) {
    setupUi(this);
    QString src = settings.value(AceImporter::SRC_URL).toString();
    leSource->setText(src);

    if (!src.isEmpty()) {
        leDest->setText(src + EXTENSION);
    }

    connect(tbDest, SIGNAL(clicked()), SLOT(sl_selectFileClicked()));
}

void AceImportDialog::sl_selectFileClicked() {
    GUrl sourceFile(leSource->text());
    QString fileFilter = DialogUtils::prepareDocumentsFileFilter(BaseDocumentFormats::UGENEDB, true);
    QString newDest = QFileDialog::getSaveFileName(this, tr("Destination UGENEDB file"), sourceFile.dirPath(), fileFilter, NULL, QFileDialog::DontConfirmOverwrite);
    if (!newDest.isEmpty()) {
        leDest->setText(newDest);
    }
}

bool AceImportDialog::isValid() {
    GUrl destUrl(leDest->text());

    if (destUrl.isEmpty()) {
        leDest->setFocus(Qt::OtherFocusReason);
        QMessageBox::critical(this, windowTitle(), tr("Destination URL is not specified"));
        return false;
    }

    if (!destUrl.isLocalFile()) {
        leDest->setFocus(Qt::OtherFocusReason);
        QMessageBox::critical(this, windowTitle(), tr("Destination URL must point to a local file"));
        return false;
    }

    Project * prj = AppContext::getProject();
    if (prj) {
        Document * destDoc = prj->findDocumentByURL(destUrl);
        if (destDoc && destDoc->isLoaded() && !GObjectViewUtils::findViewsWithAnyOfObjects(destDoc->getObjects()).isEmpty()) {
            QMessageBox::critical(this, windowTitle(), tr("There is opened view with destination file.\n"
                                                          "Close it or choose different file"));
            leDest->setFocus(Qt::OtherFocusReason);
            return false;
        }
    }

    QFileInfo destinationDir(QFileInfo(destUrl.getURLString()).path());
    if (!destinationDir.isWritable()) {
        leDest->setFocus(Qt::OtherFocusReason);
        QMessageBox::critical(this, windowTitle(), tr("Destination directory '%1' is not writable, "\
                                                      "please choose different destination URL")
                              .arg(destinationDir.absoluteFilePath()));
        return false;
    }

    if (QFile::exists(destUrl.getURLString())) {
        int result = QMessageBox::question(this, windowTitle(),
                                           tr("Destination file already exists.\n"
                                              "To overwrite the file, press 'Replace'.\n"
                                              "To append data to existing file press 'Append'."),
                                           tr("Replace"),
                                           tr("Append"),
                                           tr("Cancel"), 2);
        if (result == 0) {
            bool ok = QFile::remove(destUrl.getURLString());
            if (!ok) {
                QMessageBox::critical(this, windowTitle(), tr("Destination file '%1' cannot be removed")
                                      .arg(destUrl.getURLString()));
                return false;
            }
        } else if (result == 2) {
            return false;
        }
    }

    return true;
}

void AceImportDialog::applySettings() {
    settings.insert(AceImporter::DEST_URL, leDest->text());
}

ImportDialog* AceImportDialogFactory::getDialog(const QVariantMap &settings) const {
    return new AceImportDialog(settings);
}

}   // namespace U2
