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

#include <U2Core/AppContext.h>
#include <U2Core/ClipboardController.h>
#include <U2Core/DNASequenceObject.h>
#include <U2Core/DocumentModel.h>
#include <U2Core/DocumentUtils.h>
#include <U2Core/IOAdapter.h>
#include <U2Core/LoadDocumentTask.h>
#include <U2Core/MSAUtils.h>
#include <U2Core/MsaDbiUtils.h>
#include <U2Core/U2AlphabetUtils.h>
#include <U2Core/U2DbiUtils.h>
#include <U2Core/U2MsaDbi.h>

#include "AddSequencesToAlignmentTask.h"

namespace U2 {

const int AddSequenceObjectsToAlignmentTask::maxErrorListSize = 5;

AddSequenceObjectsToAlignmentTask::AddSequenceObjectsToAlignmentTask(MAlignmentObject* obj, const QList<DNASequence>& seqList)
    : Task("Add sequences to alignment task", TaskFlags(TaskFlags_NR_FOSE_COSC)), seqList(seqList), maObj(obj), stateLock(NULL), msaAlphabet(maObj->getAlphabet()) {}

void AddSequenceObjectsToAlignmentTask::prepare() {
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
    processObjectsAndSetResultingAlphabet();
}

void AddSequenceObjectsToAlignmentTask::processObjectsAndSetResultingAlphabet() {
    QList<DNASequence>    newSeqList;
    foreach(const DNASequence& dnaObj, seqList) {
        const DNAAlphabet* newAlphabet = U2AlphabetUtils::deriveCommonAlphabet(dnaObj.alphabet, msaAlphabet);
        if (newAlphabet != NULL) {
            msaAlphabet = newAlphabet;
            newSeqList.append(dnaObj);
        } else {
            errorList << dnaObj.getName();
        }
    }
    seqList = newSeqList;
}

Task::ReportResult AddSequenceObjectsToAlignmentTask::report() {
    releaseLock();
    if (isCanceled() || hasError()) {
        return ReportResult_Finished;
    }
    QList<U2MsaRow> rows;
    qint64 len = createRows(rows);
    CHECK_OP(stateInfo, ReportResult_Finished);

    addRows(rows, len);
    CHECK_OP(stateInfo, ReportResult_Finished);

    if (!errorList.isEmpty()) {
        setupError();
    }
    return ReportResult_Finished;
}

qint64 AddSequenceObjectsToAlignmentTask::createRows(QList<U2MsaRow>& rows) {
    U2EntityRef entityRef = maObj.data()->getEntityRef();
    qint64 maxLen = 0;
    foreach (const DNASequence& seqObj, seqList) {
        U2MsaRow row = MSAUtils::copyRowFromSequence(seqObj, entityRef.dbiRef, stateInfo);
        if (0 < row.gend) {
            rows << row;
            maxLen = qMax(maxLen, (qint64)seqObj.length());
        }
        CHECK_OP(stateInfo, 0);
    }
    return maxLen;
}

void AddSequenceObjectsToAlignmentTask::addRows(QList<U2MsaRow> &rows, qint64 len) {
    U2EntityRef entityRef = maObj.data()->getEntityRef();
    // Create user mod step
    MAlignmentModInfo mi;
    U2UseCommonUserModStep modStep(entityRef, stateInfo);
    CHECK_OP(stateInfo, );
    U2MsaDbi *dbi = modStep.getDbi()->getMsaDbi();

    // Add rows
    dbi->addRows(entityRef.entityId, rows, stateInfo);
    CHECK_OP(stateInfo, );

    if (len > maObj->getLength()) {
        dbi->updateMsaLength(entityRef.entityId, len, stateInfo);
        CHECK_OP(stateInfo, );
    }

    // Update alphabet
    if (maObj->getAlphabet() != msaAlphabet) {
        SAFE_POINT(NULL != msaAlphabet, "NULL result alphabet", );
        dbi->updateMsaAlphabet(entityRef.entityId, msaAlphabet->getId(), stateInfo);
        CHECK_OP(stateInfo, );
        mi.alphabetChanged = true;
    }

    // Update object
    maObj->updateCachedMAlignment(mi);
}

void AddSequenceObjectsToAlignmentTask::setupError() {
    CHECK(!errorList.isEmpty(), );

    QStringList smallList = errorList.mid(0, maxErrorListSize);
    QString error = tr("Some sequences have wrong alphabet: ");
    error += smallList.join(", ");
    if (smallList.size() < errorList.size()) {
        error += tr(" and others");
    }
    setError(error);
}

void AddSequenceObjectsToAlignmentTask::releaseLock(){
    if (stateLock != NULL) {
        if(maObj != NULL) {
            maObj->unlockState(stateLock);
        }
        delete stateLock;
        stateLock = NULL;
    }
}

AddSequencesFromFilesToAlignmentTask::AddSequencesFromFilesToAlignmentTask(MAlignmentObject* obj, const QStringList& urls)
    : AddSequenceObjectsToAlignmentTask(obj, QList<DNASequence>()), urlList(urls), loadTask(NULL) {}

void AddSequencesFromFilesToAlignmentTask::prepare() {
    AddSequenceObjectsToAlignmentTask::prepare();
    foreach(const QString& fileWithSequencesUrl, urlList) {
        QList<FormatDetectionResult> detectedFormats = DocumentUtils::detectFormat(fileWithSequencesUrl);
        if (!detectedFormats.isEmpty()) {
            IOAdapterFactory* factory = AppContext::getIOAdapterRegistry()->getIOAdapterFactoryById(BaseIOAdapters::LOCAL_FILE);
            DocumentFormat* format = detectedFormats.first().format;
            loadTask = new LoadDocumentTask(format->getFormatId(), fileWithSequencesUrl, factory);
            addSubTask(loadTask);
        } else {
            setError("Unknown format");
        }
    }
}

QList<Task*> AddSequencesFromFilesToAlignmentTask::onSubTaskFinished(Task* subTask) {
    QList<Task*> subTasks;

    propagateSubtaskError();
    if (isCanceled() || hasError()) {
        return subTasks;
    }

    LoadDocumentTask* loadTask = qobject_cast<LoadDocumentTask*>(subTask);
    SAFE_POINT(loadTask != NULL, "loadTask is NULL", subTasks);
    Document* doc = loadTask->getDocument();
    foreach(GObject *seqObj, doc->findGObjectByType(GObjectTypes::SEQUENCE)) {
        U2SequenceObject *casted = qobject_cast<U2SequenceObject*>(seqObj);
        SAFE_POINT(casted != NULL, "Cast to U2SequenceObject failed", subTasks);
        DNASequence seq = casted->getWholeSequence(stateInfo);
        CHECK(stateInfo.isCoR(), subTasks);
        seq.alphabet = casted->getAlphabet();
        seqList.append(seq);
    }
    processObjectsAndSetResultingAlphabet();
    return subTasks;
}


////////////////////////////////////////////////////////////////////////////////
//AddSequencesFromDocumentsToAlignmentTask
AddSequencesFromDocumentsToAlignmentTask::AddSequencesFromDocumentsToAlignmentTask(MAlignmentObject* obj, const QList<Document*>& docs)
    : AddSequenceObjectsToAlignmentTask(obj, QList<DNASequence>()), docs(docs) {}

void AddSequencesFromDocumentsToAlignmentTask::prepare() {
    AddSequenceObjectsToAlignmentTask::prepare();
    seqList = PasteUtils::getSequences(docs, stateInfo);
    processObjectsAndSetResultingAlphabet();
}

}




