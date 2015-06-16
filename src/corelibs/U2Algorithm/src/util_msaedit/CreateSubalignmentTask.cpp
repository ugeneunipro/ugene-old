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

#include <QtGui/QClipboard>

#if (QT_VERSION < 0x050000) //Qt 5
#include <QtGui/QApplication>
#else
#include <QtWidgets/QApplication>
#endif

#include <U2Core/AppContext.h>
#include <U2Core/AppSettings.h>
#include <U2Core/BaseDocumentFormats.h>
#include <U2Core/DocumentModel.h>
#include <U2Core/GHints.h>
#include <U2Core/GObjectRelationRoles.h>
#include <U2Core/GObjectUtils.h>
#include <U2Core/GUrlUtils.h>
#include <U2Core/IOAdapter.h>
#include <U2Core/IOAdapterUtils.h>
#include <U2Core/LocalFileAdapter.h>
#include <U2Core/Log.h>
#include <U2Core/MAlignmentImporter.h>
#include <U2Core/SaveDocumentTask.h>
#include <U2Core/U2OpStatusUtils.h>
#include <U2Core/U2SafePoints.h>
#include <U2Core/UserApplicationsSettings.h>


#include "CreateSubalignmentTask.h"

namespace U2{

CreateSubalignmentTask::CreateSubalignmentTask(MAlignmentObject *maObj, const CreateSubalignmentSettings &settings)
    : DocumentProviderTask(tr("Create sub-alignment: %1").arg(maObj->getDocument()->getName()), TaskFlags_NR_FOSCOE),
    origMAObj(maObj), cfg(settings)
{
    origDoc = maObj->getDocument();
    createCopy = cfg.url != origDoc->getURL() || cfg.url.isEmpty();
}

void CreateSubalignmentTask::prepare() {
    QString ext = cfg.url.completeFileSuffix();

    DocumentFormatRegistry *dfr = AppContext::getDocumentFormatRegistry();
    DocumentFormat *dfd = dfr->getFormatById(cfg.formatIdToSave);

    IOAdapterFactory* iof = AppContext::getIOAdapterRegistry()->getIOAdapterFactoryById(IOAdapterUtils::url2io(cfg.url));
    if (createCopy) {
        QVariantMap hints = origDoc->getGHintsMap();
        if (hints.value(DocumentReadingMode_SequenceAsAlignmentHint, false).toBool()) {
            hints[DocumentReadingMode_SequenceAsAlignmentHint] = false;
        }
        resultDocument = dfd->createNewLoadedDocument(iof, cfg.url, stateInfo, hints);
        CHECK_OP(stateInfo, );

        MAlignment msa = origMAObj->getMAlignment();
        resultMAObj = MAlignmentImporter::createAlignment(resultDocument->getDbiRef(), msa, stateInfo);
        CHECK_OP(stateInfo, );
        resultMAObj->setGHints(new GHintsDefaultImpl(origMAObj->getGHintsMap()));

        resultDocument->addObject(resultMAObj);
        GObjectUtils::updateRelationsURL(resultMAObj, origDoc->getURL(), cfg.url);
        QList<GObjectRelation> phyTreeRelations = resultMAObj->findRelatedObjectsByRole(ObjectRole_PhylogeneticTree);
        foreach(GObjectRelation phyTreeRel, phyTreeRelations) {
            resultMAObj->removeObjectRelation(phyTreeRel);
        }
    } else {
        CHECK_EXT(origDoc->isStateLocked(), setError(tr("Document is locked: %1").arg(origDoc->getURLString())), );
        resultDocument = origDoc;
        resultMAObj = origMAObj;
        docOwner = false;
    }

    //TODO: add "remove empty rows and columns" flag to crop function
    QSet<QString> rowNames;
    foreach (const QString& name, cfg.seqNames) {
        rowNames.insert(name);
    }
    resultMAObj->crop(cfg.window, rowNames);

    if (cfg.saveImmediately) {
        addSubTask(new SaveDocumentTask(resultDocument, iof));
    }
}

SubalignmentToClipboardTask::SubalignmentToClipboardTask(MAlignmentObject *_maObj, const QRect& selectionRect, const DocumentFormatId& formatId)
    :Task(tr("Copy formatted alignment to the clipboard"), TaskFlags_NR_FOSE_COSC ), maObj(_maObj), selectionRect(selectionRect), formatId(formatId), createSubalignmentTask(NULL){
}

void SubalignmentToClipboardTask::prepare(){
    CreateSubalignmentSettings settings = defineSettings(maObj, selectionRect, formatId, stateInfo);
    CHECK_OP(stateInfo, )

    createSubalignmentTask = new CreateSubalignmentTask(maObj, settings);
    addSubTask(createSubalignmentTask);
}

#define READ_BUF_SIZE 4096
QList<Task *> SubalignmentToClipboardTask::onSubTaskFinished(Task *subTask){
    QList<Task*> subTasks;
    if (subTask->hasError() || isCanceled()) {
        return subTasks;
    }

    if(subTask == createSubalignmentTask){
        Document* doc = createSubalignmentTask->getDocument();
        SAFE_POINT_EXT(doc != NULL, setError(tr("No temporary document.")), subTasks);
        QScopedPointer<LocalFileAdapterFactory> factory( new LocalFileAdapterFactory());
        QScopedPointer<IOAdapter> io(factory->createIOAdapter());
        if(!io->open(doc->getURL(), IOAdapterMode_Read)){
            setError(tr("Cannot read the temporary file."));
            return subTasks;
        }

        QString res;
        QByteArray buf;
        while(!io->isEof()){
            buf.resize(READ_BUF_SIZE);
            buf.fill(0);
            int read = io->readLine(buf.data(), READ_BUF_SIZE);
            buf.resize(read);
            res.append(buf);
            res.append('\n');
        }
        QApplication::clipboard()->setText(res);
    }

    return subTasks;
}

CreateSubalignmentSettings SubalignmentToClipboardTask::defineSettings(MAlignmentObject *_maObj, const QRect &selectionRect, const DocumentFormatId &formatId, U2OpStatus& os){
    U2Region window(selectionRect.x(), selectionRect.width());

    QStringList names;
    int startSeq = selectionRect.y();
    int endSeq = selectionRect.y() + selectionRect.height();
    for (int i=startSeq; i < endSeq; i++) {
        names.append(_maObj->getMAlignment().getRow(i).getName());
    }

    //Create temporal document for the workflow run task
    const AppSettings* appSettings = AppContext::getAppSettings();
    SAFE_POINT_EXT(NULL != appSettings, os.setError(tr("Invalid applications settings detected")), CreateSubalignmentSettings());

    UserAppsSettings* usersSettings = appSettings->getUserAppsSettings();
    SAFE_POINT_EXT(NULL != usersSettings, os.setError(tr("Invalid users applications settings detected")), CreateSubalignmentSettings());
    const QString tmpDirPath = usersSettings->getCurrentProcessTemporaryDirPath();
    GUrl path = GUrlUtils::prepareTmpFileLocation(tmpDirPath, "clipboard", "tmp", os);

    return CreateSubalignmentSettings(window, names, path, true, false, formatId);
}

}
