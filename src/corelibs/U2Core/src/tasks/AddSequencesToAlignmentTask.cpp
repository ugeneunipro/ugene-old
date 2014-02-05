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

#include "AddSequencesToAlignmentTask.h"

#include <U2Core/DocumentModel.h>
#include <U2Core/AppContext.h>
#include <U2Core/DNASequenceObject.h>
#include <U2Core/LoadDocumentTask.h>
#include <U2Core/MSAUtils.h>
#include <U2Core/DocumentUtils.h>
#include <U2Core/IOAdapter.h>
#include <U2Core/U2AlphabetUtils.h>
#include <U2Core/U2DbiUtils.h>
#include <U2Core/U2MsaDbi.h>

namespace U2 {

const int AddSequencesToAlignmentTask::maxErrorListSize = 5;

AddSequencesToAlignmentTask::AddSequencesToAlignmentTask( MAlignmentObject* obj, const QStringList& fileWithSequencesUrls )
: Task("Add sequences to alignment task", TaskFlag_NoRun), maObj(obj), urls(fileWithSequencesUrls), stateLock(NULL)
{
    assert(!fileWithSequencesUrls.isEmpty());
    msaAlphabet = maObj->getAlphabet();
}

void AddSequencesToAlignmentTask::prepare()
{
 
    if (maObj.isNull()) {
        stateInfo.setError(tr("Object is empty."));
        return;
    }

    if (maObj->isStateLocked()) {
        stateInfo.setError(tr("Object is locked for modifications."));
        return;
    }
    
    stateLock = new StateLock("Adding_files_to_alignment", StateLockFlag_LiveLock);
    maObj->lockState(stateLock);

    foreach( const QString& fileWithSequencesUrl, urls) {
        QList<FormatDetectionResult> detectedFormats = DocumentUtils::detectFormat(fileWithSequencesUrl);    
        if (!detectedFormats.isEmpty()) {
            IOAdapterFactory* factory = AppContext::getIOAdapterRegistry()->getIOAdapterFactoryById(BaseIOAdapters::LOCAL_FILE);
            DocumentFormat* format = detectedFormats.first().format;
            LoadDocumentTask* loadTask = new LoadDocumentTask(format->getFormatId(), fileWithSequencesUrl, factory);
            addSubTask(loadTask);
        } else {
            setError("Unknown format");
        }
    }

}



QList<Task*> AddSequencesToAlignmentTask::onSubTaskFinished( Task* subTask )
{
    QList<Task*> subTasks;

    propagateSubtaskError();
    if ( isCanceled() || hasError() ) {
        return subTasks;
    }

    LoadDocumentTask* loadTask = qobject_cast<LoadDocumentTask*>(subTask);
    assert(loadTask != NULL);
    Document* doc = loadTask->getDocument();
    QList<GObject*> seqObjects = doc->findGObjectByType(GObjectTypes::SEQUENCE);

    foreach(GObject* obj, seqObjects) {
        U2SequenceObject* dnaObj = qobject_cast<U2SequenceObject*>(obj);
        assert(dnaObj != NULL);
        const DNAAlphabet* newAlphabet = U2AlphabetUtils::deriveCommonAlphabet(dnaObj->getAlphabet(), msaAlphabet);
        if (newAlphabet != NULL) {
            seqList << dnaObj;
            msaAlphabet = newAlphabet;
        } else {
            errorList << dnaObj->getGObjectName();
        }
    }

    return subTasks;
}

Task::ReportResult AddSequencesToAlignmentTask::report()
{
    if (stateLock) {
        maObj->unlockState(stateLock);
        delete stateLock;
    }

    QList<U2MsaRow> rows = createRows();
    CHECK_OP(stateInfo, ReportResult_Finished);

    addRows(rows);
    CHECK_OP(stateInfo, ReportResult_Finished);

    if (!errorList.isEmpty()) {
        setupError();
    }
    return ReportResult_Finished;
}

QList<U2MsaRow> AddSequencesToAlignmentTask::createRows() {
    QList<U2MsaRow> rows;
    U2EntityRef entityRef = maObj.data()->getEntityRef();
    foreach (U2SequenceObject *seqObj, seqList) {
        rows << MSAUtils::copyRowFromSequence(seqObj, entityRef.dbiRef, stateInfo);
        CHECK_OP(stateInfo, rows);
    }
    return rows;
}

void AddSequencesToAlignmentTask::addRows(QList<U2MsaRow> &rows) {
    U2EntityRef entityRef = maObj.data()->getEntityRef();
    // Create user mod step
    U2UseCommonUserModStep modStep(entityRef, stateInfo);
    CHECK_OP(stateInfo, );
    U2MsaDbi *dbi = modStep.getDbi()->getMsaDbi();

    // Add rows
    dbi->addRows(entityRef.entityId, rows, stateInfo);
    CHECK_OP(stateInfo, );

    // Update alphabet
    if (maObj->getAlphabet() != msaAlphabet) {
        SAFE_POINT(NULL != msaAlphabet, "NULL result alphabet", );
        dbi->updateMsaAlphabet(entityRef.entityId, msaAlphabet->getId(), stateInfo);
        CHECK_OP(stateInfo, );
    }

    // Update object
    maObj->updateCachedMAlignment();
}

void AddSequencesToAlignmentTask::setupError() {
    CHECK(!errorList.isEmpty(), );

    QStringList smallList = errorList.mid(0, maxErrorListSize);
    QString error = tr("Some sequences have wrong alphabet: ");
    error += smallList.join(", ");
    if (smallList.size() < errorList.size()) {
        error += tr(" and others");
    }
    setError(error);
}

}
