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

#include <QMessageBox>

#include <U2Core/AppContext.h>
#include <U2Core/BaseDocumentFormats.h>
#include <U2Core/DocumentModel.h>
#include <U2Core/GUrlUtils.h>
#include <U2Core/ProjectModel.h>
#include <U2Core/U2SafePoints.h>

#include <U2Formats/AceImporter.h>

#include <U2Gui/DialogUtils.h>
#include <U2Gui/HelpButton.h>
#include <U2Gui/ObjectViewModel.h>
#include <U2Gui/SaveDocumentController.h>
#include <U2Gui/U2FileDialog.h>

#include "AceImportDialog.h"

namespace U2 {

const QString AceImportDialog::EXTENSION = ".ugenedb";

AceImportDialog::AceImportDialog(const QVariantMap& _settings) :
    ImportDialog(_settings),
    saveController(NULL)
{
    setupUi(this);
    new HelpButton(this, buttonBox, "17467699");

    QString src = settings.value(AceImporter::SRC_URL).toString();
    leSource->setText(src);

    initSaveController();
}

bool AceImportDialog::isValid() {
    GUrl destUrl(saveController->getSaveFileName());

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
    settings.insert(AceImporter::DEST_URL, saveController->getSaveFileName());
}

void AceImportDialog::initSaveController() {
    SaveDocumentControllerConfig config;
    if (!leSource->text().isEmpty()) {
        config.defaultFileName = leSource->text() + EXTENSION;
    }
    config.defaultFormatId = BaseDocumentFormats::UGENEDB;
    config.fileDialogButton = tbDest;
    config.fileNameEdit = leDest;
    config.parentWidget = this;
    config.saveTitle = tr("Destination UGENEDB file");

    const QList<DocumentFormatId> formats = QList<DocumentFormatId>() << BaseDocumentFormats::UGENEDB;

    saveController = new SaveDocumentController(config, formats, this);
}

}   // namespace U2
