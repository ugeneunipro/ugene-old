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

#include <QtGui/QFileDialog>
#include <QtGui/QMessageBox>
#include <QtGui/QMainWindow>

#include <U2Core/AppContext.h>
#include <U2Core/BaseDocumentFormats.h>
#include <U2Core/DocumentModel.h>
#include <U2Core/FormatUtils.h>
#include <U2Core/GUrlUtils.h>
#include <U2Core/ProjectModel.h>

//#include <U2Gui/ObjectViewModel.h>
#include <U2Gui/MainWindow.h>

#include "ConvertAceToSqliteDialog.h"

namespace U2 {

ConvertAceToSqliteDialog::ConvertAceToSqliteDialog(const GUrl &src) :
    QDialog((AppContext::getMainWindow()->getQMainWindow()->centralWidget())) {
    setupUi(this);
    initialize(src);
    connect(tbDest, SIGNAL(clicked()), SLOT(sl_selectFileClicked()));
}

void ConvertAceToSqliteDialog::accept() {
    destUrl = GUrl(leDest->text());
    addToProject = cbAddToProject->isChecked();
    if (isDestValid()) {
        QDialog::accept();
    }
}

void ConvertAceToSqliteDialog::sl_selectFileClicked() {
    GUrl sourceFile(leSource->text());
    QString fileFilter = FormatUtils::prepareDocumentsFileFilter(BaseDocumentFormats::UGENEDB, true);
    QString newDest = QFileDialog::getSaveFileName(this, tr("Destination UGENEDB file"), sourceFile.dirPath(), fileFilter, NULL, QFileDialog::DontConfirmOverwrite);
    if (!newDest.isEmpty()) {
        destUrl = GUrl(newDest);
    }
    leDest->setText(destUrl.getURLString());
}

void ConvertAceToSqliteDialog::initialize(const GUrl &src) {
    leSource->setText(src.getURLString());
    leDest->setText(src.getURLString() + ".ugenedb");
    destUrl = GUrl(leDest->text());
}

bool ConvertAceToSqliteDialog::isDestValid(){
    if (destUrl.isEmpty()) {
        leDest->setFocus(Qt::OtherFocusReason);
        QMessageBox::critical(this, windowTitle(), tr("Destination URL is not specified"));
        return false;
    } else if (!destUrl.isLocalFile()) {
        leDest->setFocus(Qt::OtherFocusReason);
        QMessageBox::critical(this, windowTitle(), tr("Destination URL must point to a local file"));
        return false;
    } else if (!GUrlUtils::canWriteFile(destUrl.getURLString())) {
        QString defaultDataDestinationUrl;

        defaultDataDestinationUrl = GUrlUtils::getDefaultDataPath();
        defaultDataDestinationUrl += "/" + destUrl.fileName();

        QMessageBox::critical(this, windowTitle(), tr("Cannot write to \'%1\'. You may change "\
                                                      "permissions or a file path. Destination "\
                                                      "is changed to default UGENE data path: %2")
                              .arg(destUrl.getURLString()).arg(defaultDataDestinationUrl));

        if (!defaultDataDestinationUrl.isEmpty()) {
            leDest->setText(defaultDataDestinationUrl);
        }
        return false;
    } else {
        // Disabled because of circular library dependency
        // An issue is created to fix it: UGENE-2292
//        Project * prj = AppContext::getProject();
//        if (prj) {
//            Document * destDoc = prj->findDocumentByURL(destUrl);
//            if (destDoc && destDoc->isLoaded() && !GObjectViewUtils::findViewsWithAnyOfObjects(destDoc->getObjects()).isEmpty()) {
//                QMessageBox::critical(this, windowTitle(), tr("There is opened view with destination file.\n"
//                                                              "Close it or choose different file"));
//                leDest->setFocus(Qt::OtherFocusReason);
//                return false;
//            }
//        }

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
            switch(result) {
                case 0:
                    {
                        bool ok = QFile::remove(destUrl.getURLString());
                        if (!ok) {
                            QMessageBox::critical(this, windowTitle(), tr("Destination file '%1' cannot be removed")
                                                  .arg(destUrl.getURLString()));
                            return false;
                        }
                    }
                case 1:
                    QDialog::accept();
                    break;
            }
        } else {
            return true;
        }
    }
    return true;
}

}   // namespace
