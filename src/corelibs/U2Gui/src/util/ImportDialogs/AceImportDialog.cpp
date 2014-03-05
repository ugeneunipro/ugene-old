#include "AceImportDialog.h"

#include <U2Core/AppContext.h>
#include <U2Core/BaseDocumentFormats.h>
#include <U2Core/DocumentModel.h>
#include <U2Core/GUrlUtils.h>
#include <U2Core/ProjectModel.h>
#include <U2Core/U2SafePoints.h>

#include <U2Formats/AceImporter.h>

#include <U2Gui/DialogUtils.h>
#include <U2Gui/ObjectViewModel.h>

#if (QT_VERSION < 0x050000) //Qt 5
#include <QtGui/QFileDialog>
#include <QtGui/QMessageBox>
#else
#include <QtWidgets/QFileDialog>
#include <QtWidgets/QMessageBox>
#endif
#include <U2Gui/HelpButton.h>

namespace U2 {

const QString AceImportDialog::EXTENSION = ".ugenedb";

AceImportDialog::AceImportDialog(const QVariantMap& _settings) :
    ImportDialog(_settings) {
    setupUi(this);
    new HelpButton(this, buttonBox, "4227131");

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

}   // namespace U2
