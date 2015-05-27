/**
 * UGENE - Integrated Bioinformatics Tools.
 * Copyright (C) 2008-2015 UniPro <ugene@unipro.ru>
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
#include <U2Core/AppSettings.h>
#include <U2Core/BaseDocumentFormats.h>
#include <U2Core/DocumentModel.h>
#include <U2Core/DocumentUtils.h>
#include <U2Core/FormatUtils.h>
#include <U2Core/GUrlUtils.h>
#include <U2Core/L10n.h>
#include <U2Core/Log.h>
#include <U2Core/ProjectModel.h>
#include <U2Core/ProjectService.h>
#include <U2Core/U2ObjectDbi.h>
#include <U2Core/U2OpStatusUtils.h>
#include <U2Core/U2SafePoints.h>
#include <U2Core/UserApplicationsSettings.h>

#include <U2Gui/GUIUtils.h>
#include <U2Gui/HelpButton.h>
#include <U2Gui/LastUsedDirHelper.h>
#include <U2Gui/ProjectTreeItemSelectorDialog.h>
#include <U2Gui/ProjectUtils.h>
#include <U2Gui/SharedConnectionsDialog.h>
#include <U2Core/QObjectScopedPointer.h>
#include <U2Gui/U2FileDialog.h>

#include "ExportPrimersDialog.h"
#include "ExportPrimersToDatabaseTask.h"
#include "ExportPrimersToLocalFileTask.h"

namespace U2 {

const QString ExportPrimersDialog::LOCAL_FILE = QObject::tr("Local file");
const QString ExportPrimersDialog::SHARED_DB = QObject::tr("Shared database");

ExportPrimersDialog::ExportPrimersDialog(const QList<Primer> &primers) :
    QDialog(NULL),
    primers(primers)
{
    setupUi(this);
    new HelpButton(this, buttonBox, "16122426");

    init();
    connectSignals();
    sl_updateState();
}

void ExportPrimersDialog::sl_updateState() {
    fileContainer->setVisible(isFileMode());
    databaseContainer->setVisible(!isFileMode());
    folderContainer->setEnabled(0 < cbDatabase->count());
}

void ExportPrimersDialog::sl_formatChanged() {
    const QString newFilePath = GUrlUtils::changeFileExt(GUrl(leFilePath->text()), cbFormat->currentText()).getURLString();
    leFilePath->setText(newFilePath);
}

void ExportPrimersDialog::sl_fileBrowse() {
    LastUsedDirHelper dirHelper("ExportPrimersDialog");
    const QString filter = FormatUtils::prepareDocumentsFileFilter(cbFormat->currentText(), true);

    QFileDialog::Options additionalOptions;
    Q_UNUSED(additionalOptions);
#ifdef Q_OS_MAC
    if (qgetenv("UGENE_GUI_TEST").toInt() == 1 && qgetenv("UGENE_USE_NATIVE_DIALOGS").toInt() == 0) {
        additionalOptions = QFileDialog::DontUseNativeDialog;
    }
#endif

    const QString filePath = U2FileDialog::getSaveFileName(this, tr("Export to"), dirHelper.dir + GUrl(leFilePath->text()).fileName(), filter, NULL, additionalOptions);
    CHECK(!filePath.isEmpty(), );

    leFilePath->setText(filePath);
}

void ExportPrimersDialog::sl_connect() {
    const bool projectExists = (NULL != AppContext::getProject());
    QPointer<SharedConnectionsDialog> connectionDialog = new SharedConnectionsDialog(this);
    if (projectExists) {
        connectProjectSignals();
    } else {
        connect(connectionDialog.data(), SIGNAL(si_connectionCompleted()), SLOT(sl_connectionCompleted()));
    }

    connectionDialog->exec();
    CHECK(!connectionDialog.isNull(), );

    if (projectExists) {
        delete connectionDialog;
    }
}

void ExportPrimersDialog::sl_connectionCompleted() {
    QObject *connectionDialog = sender();
    if (Q_LIKELY(NULL != connectionDialog)) {
        connectionDialog->deleteLater();
    }
    initDatabases();
    sl_updateState();
}

void ExportPrimersDialog::sl_documentAdded(Document *document) {
    CHECK(document->isDatabaseConnection(), );
    cbDatabase->addItem(document->getName(), QVariant::fromValue<U2DbiRef>(document->getDbiRef()));
    if (cbDatabase->currentIndex() == -1) {
        cbDatabase->setCurrentIndex(0);
    }
    sl_updateState();
}

void ExportPrimersDialog::sl_documentRemoved(Document *document) {
    CHECK(document->isDatabaseConnection(), );
    for (int i = 0; i < cbDatabase->count(); i++) {
        if (cbDatabase->itemData(i).value<U2DbiRef>() == document->getDbiRef()) {
            cbDatabase->removeItem(i);
        }
    }
    sl_updateState();
}

void ExportPrimersDialog::sl_folderBrowse() {
    const Folder folder = ProjectTreeItemSelectorDialog::selectFolder(this);
    if (!folder.getFolderPath().isEmpty()) {
        leFolder->setText(folder.getFolderPath());
    }
}

void ExportPrimersDialog::accept() {
    GUIUtils::setWidgetWarning(leFilePath, false);
    GUIUtils::setWidgetWarning(cbDatabase, false);
    GUIUtils::setWidgetWarning(leFolder, false);

    U2OpStatusImpl os;
    GUrlUtils::validateLocalFileUrl(GUrl(leFilePath->text()), os);
    if (LOCAL_FILE == cbExport->currentText() && os.isCoR()) {
        GUIUtils::setWidgetWarning(leFilePath, true);
        return;
    }

    if (SHARED_DB == cbExport->currentText() && cbDatabase->currentText().isEmpty()) {
        GUIUtils::setWidgetWarning(cbDatabase, true);
        return;
    }

    if (SHARED_DB == cbExport->currentText() && !leFolder->text().startsWith(U2ObjectDbi::ROOT_FOLDER)) {
        GUIUtils::setWidgetWarning(leFolder, true);
        return;
    }

    Task *exportTask = NULL;
    if (LOCAL_FILE == cbExport->currentText()) {
        exportTask = new ExportPrimersToLocalFileTask(primers, cbFormat->currentText(), leFilePath->text());
    } else {
        exportTask = new ExportPrimersToDatabaseTask(primers, cbDatabase->itemData(cbDatabase->currentIndex()).value<U2DbiRef>(), leFolder->text());
    }
    AppContext::getTaskScheduler()->registerTopLevelTask(exportTask);

    QDialog::accept();
}

void ExportPrimersDialog::init() {
    cbExport->addItem(LOCAL_FILE);
    cbExport->addItem(SHARED_DB);

    DocumentFormatConstraints constr;
    constr.supportedObjectTypes.insert(GObjectTypes::SEQUENCE);
    constr.addFlagToSupport(DocumentFormatFlag_SupportWriting);
    constr.addFlagToExclude(DocumentFormatFlag_CannotBeCreated);
    constr.addFlagToExclude(DocumentFormatFlag_Hidden);
    if (primers.size() > 1) {
        constr.addFlagToExclude(DocumentFormatFlag_SingleObjectFormat);
    }

    const QList<DocumentFormatId> formatIds = AppContext::getDocumentFormatRegistry()->selectFormats(constr);
    foreach (const DocumentFormatId &formatId, formatIds) {
        DocumentFormat *format = AppContext::getDocumentFormatRegistry()->getFormatById(formatId);
        if (Q_UNLIKELY(NULL == format)) {
            coreLog.error(L10N::nullPointerError("document format '%1'").arg(formatId));
            Q_ASSERT(false);
            continue;
        }

        cbFormat->addItem(formatId);
    }
    cbFormat->setCurrentIndex(cbFormat->findText(BaseDocumentFormats::PLAIN_GENBANK));

    DocumentFormat *currentFormat = AppContext::getDocumentFormatRegistry()->getFormatById(cbFormat->currentText());
    SAFE_POINT(NULL != currentFormat, L10N::nullPointerError("current format"), );
    const QStringList extensions = currentFormat->getSupportedDocumentFileExtensions();
    SAFE_POINT(!extensions.isEmpty(), QString("'%1' format extenations list is empty").arg(cbFormat->currentText()), );
    leFilePath->setText(AppContext::getAppSettings()->getUserAppsSettings()->getDefaultDataDirPath() + QDir::separator() + "primers" + "." + extensions.first());

    initDatabases();
}

void ExportPrimersDialog::initDatabases() {
    while (0 < cbDatabase->count()) {
        cbDatabase->removeItem(0);
    }

    const QList<Document *> connectedDatabases = ProjectUtils::getConnectedSharedDatabases();
    foreach (Document *database, connectedDatabases) {
        cbDatabase->addItem(database->getName(), QVariant::fromValue<U2DbiRef>(database->getDbiRef()));
    }
}

void ExportPrimersDialog::connectSignals() {
    connect(cbExport, SIGNAL(currentIndexChanged(int)), SLOT(sl_updateState()));
    connect(cbFormat, SIGNAL(currentIndexChanged(int)), SLOT(sl_formatChanged()));
    connect(tbFileBrowse, SIGNAL(clicked()), SLOT(sl_fileBrowse()));
    connect(tbConnect, SIGNAL(clicked()), SLOT(sl_connect()));
    connect(tbFolderBrowse, SIGNAL(clicked()), SLOT(sl_folderBrowse()));
    if (NULL != AppContext::getProject()) {
        connectProjectSignals();
    }
}

void ExportPrimersDialog::connectProjectSignals(){
    connect(AppContext::getProject(), SIGNAL(si_documentAdded(Document *)), SLOT(sl_documentAdded(Document *)), Qt::UniqueConnection);
    connect(AppContext::getProject(), SIGNAL(si_documentRemoved(Document*)), SLOT(sl_documentRemoved(Document *)), Qt::UniqueConnection);
}

bool ExportPrimersDialog::isFileMode() const {
    return LOCAL_FILE == cbExport->currentText();
}

}   // namespace U2
