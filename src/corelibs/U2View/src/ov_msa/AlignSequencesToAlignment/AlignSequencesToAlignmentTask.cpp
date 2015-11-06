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
#include <U2Core/DNASequenceObject.h>
#include <U2Core/DocumentModel.h>
#include <U2Core/DocumentSelection.h>
#include <U2Core/DocumentUtils.h>
#include <U2Core/ExternalToolRunTask.h>
#include <U2Core/GObjectSelection.h>
#include <U2Core/IOAdapter.h>
#include <U2Core/IOAdapterUtils.h>
#include <U2Core/LoadDocumentTask.h>
#include <U2Core/MSAUtils.h>
#include <U2Core/MsaDbiUtils.h>
#include <U2Core/U2AlphabetUtils.h>
#include <U2Core/U2DbiUtils.h>
#include <U2Core/U2MsaDbi.h>
#include <U2Core/U2SequenceUtils.h>

#include <U2Gui/U2FileDialog.h>
#include <U2Gui/ProjectView.h>
#include <U2View/MSAEditor.h>

#include <U2Algorithm/AlignmentAlgorithmsRegistry.h>
#include <U2Algorithm/AbstractAlignmentTask.h>
#include <U2Algorithm/BaseAlignmentAlgorithmsIds.h>

#include "AlignSequencesToAlignmentTask.h"

#include <QtCore/QDir>
#include <QMessageBox>

namespace U2 {

const int LoadSequencesTask::maxErrorListSize = 5;

/************************************************************************/
/* SequencesExtractor */
/************************************************************************/
SequenceObjectsExtractor::SequenceObjectsExtractor() : extractFromMsa(false), sequencesMaxLength(0) {}

void SequenceObjectsExtractor::setAlphabet(const DNAAlphabet* newAlphabet) {
    seqsAlphabet = newAlphabet;
}

void SequenceObjectsExtractor::extractSequencesFromDocuments(const QList<Document*>& documentsList) {
    foreach(Document* curDocument, documentsList) {
        extractSequencesFromDocument(curDocument);
    }
}

void SequenceObjectsExtractor::extractSequencesFromDocument(Document* doc) {
    extractSequencesFromObjects(doc->getObjects());
}

void SequenceObjectsExtractor::extractSequencesFromObjects(const QList<GObject*>& objects) {
    foreach(GObject* object, objects) {
        Document* doc = object->getDocument();
        if(doc != NULL) {
            if(!usedDocuments.contains(doc)){
                usedDocuments << doc;
            }
        }

        if(object->getGObjectType() == GObjectTypes::MULTIPLE_ALIGNMENT) {
            extractFromMsa = true;
            MAlignmentObject* curObj = qobject_cast<MAlignmentObject*>(object);
            SAFE_POINT(curObj != NULL, "MAlignmentObject is null",);

            checkAlphabet(curObj->getAlphabet(), curObj->getGObjectName());
            sequencesMaxLength = qMax(sequencesMaxLength, curObj->getLength());

            foreach(const MAlignmentRow& row, curObj->getMAlignment().getRows()) {
                U2EntityRef seqRef(curObj->getEntityRef().dbiRef, row.getRowDBInfo().sequenceId);
                sequenceRefs << seqRef;
                sequenceNames << row.getName();
            }
        } else if(object->getGObjectType() == GObjectTypes::SEQUENCE) {
            U2SequenceObject* dnaObj = qobject_cast<U2SequenceObject*>(object);
            SAFE_POINT(dnaObj != NULL, "U2SequenceObject is null",);
            sequencesMaxLength = qMax(sequencesMaxLength, dnaObj->getSequenceLength());
            sequenceRefs << dnaObj->getEntityRef();
            sequenceNames << dnaObj->getSequenceName();

            checkAlphabet(dnaObj->getAlphabet(), dnaObj->getGObjectName());
        }
    }
}

void SequenceObjectsExtractor::checkAlphabet(const DNAAlphabet* newAlphabet, const QString& objectName) {
    if(seqsAlphabet == NULL) {
        seqsAlphabet = newAlphabet;
    } else {
        const DNAAlphabet* commonAlphabet = U2AlphabetUtils::deriveCommonAlphabet(newAlphabet, seqsAlphabet);
        if(commonAlphabet == NULL) {
            errorList << objectName;
        } else {
            seqsAlphabet = commonAlphabet;
        }
    }
}

const QStringList& SequenceObjectsExtractor::getErrorList() const {
    return errorList;
}

const DNAAlphabet* SequenceObjectsExtractor::getAlphabet() const {
    return seqsAlphabet;
}

const QList<U2EntityRef>& SequenceObjectsExtractor::getSequenceRefs() const {
    return sequenceRefs;
}

const QStringList& SequenceObjectsExtractor::getSequenceNames() const {
    return sequenceNames;
}
qint64 SequenceObjectsExtractor::getMaxSequencesLength() const {
    return sequencesMaxLength;
}

const QList<Document*>& SequenceObjectsExtractor::getUsedDocuments() const {
    return usedDocuments;
}

/************************************************************************/
/* LoadSequencesTask */
/************************************************************************/
LoadSequencesTask::LoadSequencesTask(const DNAAlphabet* msaAlphabet, const QStringList& fileWithSequencesUrls)
: Task(tr("Load sequences task"), TaskFlag_NoRun), msaAlphabet(msaAlphabet), urls(fileWithSequencesUrls), extractor()
{
    assert(!fileWithSequencesUrls.isEmpty());
    extractor.setAlphabet(msaAlphabet);
}

void LoadSequencesTask::prepare()
{
    foreach( const QString& fileWithSequencesUrl, urls) {
        QList<FormatDetectionResult> detectedFormats = DocumentUtils::detectFormat(fileWithSequencesUrl);
        if (!detectedFormats.isEmpty()) {
            LoadDocumentTask* loadTask = LoadDocumentTask::getDefaultLoadDocTask(fileWithSequencesUrl);
            if(loadTask != NULL) {
                addSubTask(loadTask);
            }
        } else {
            setError("Unknown format");
        }
    }
}

QList<Task*> LoadSequencesTask::onSubTaskFinished(Task* subTask) {
    QList<Task*> subTasks;

    propagateSubtaskError();
    if (subTask->isCanceled() || isCanceled() || hasError() ) {
        return subTasks;
    }

    LoadDocumentTask* loadTask = qobject_cast<LoadDocumentTask*>(subTask);
    SAFE_POINT(loadTask != NULL, "LoadDocumentTask is null", subTasks);

    CHECK(loadTask->getDocument() != NULL, subTasks);
    extractor.extractSequencesFromDocument(loadTask->getDocument());
    return subTasks;
}

void LoadSequencesTask::setupError() {
    CHECK(!extractor.getErrorList().isEmpty(), );

    QStringList smallList = extractor.getErrorList().mid(0, maxErrorListSize);
    QString error = tr("Some sequences have wrong alphabet: ");
    error += smallList.join(", ");
    if (smallList.size() < extractor.getErrorList().size()) {
        error += tr(" and others");
    }
    setError(error);
}

Task::ReportResult LoadSequencesTask::report() {
    CHECK_OP(stateInfo, ReportResult_Finished);

    if(!extractor.getErrorList().isEmpty()) {
        setupError();
    }
    if(extractor.getSequenceRefs().isEmpty()) {
        QString filesSeparator(", ");
        setError(tr("There are no sequences to align in the document(s): %1").arg(urls.join(filesSeparator)));
        return ReportResult_Finished;
    }
    if(U2AlphabetUtils::deriveCommonAlphabet(extractor.getAlphabet(), msaAlphabet) == NULL) {
        setError(tr("Sequences have incompatible alphabets"));
    }
    return ReportResult_Finished;
}

const SequenceObjectsExtractor& LoadSequencesTask::getExtractor() const {
    return extractor;
}



/************************************************************************/
/* AlignSequencesToAlignmentTask */
/************************************************************************/
AlignSequencesToAlignmentTask::AlignSequencesToAlignmentTask(MAlignmentObject* obj, const SequenceObjectsExtractor& extractor)
    : Task(tr("Align sequences to alignment task"), TaskFlags_NR_FOSE_COSC), maObj(obj), stateLock(NULL), docStateLock(NULL), 
    sequencesMaxLength(extractor.getMaxSequencesLength()), extr(extractor)
{
    fillSettingsByDefault();
    settings.addedSequencesRefs = extractor.getSequenceRefs();
    settings.addedSequencesNames = extractor.getSequenceNames();
    settings.maxSequenceLength = extractor.getMaxSequencesLength();
    settings.alphabet = extractor.getAlphabet()->getId();
    usedDocuments = extractor.getUsedDocuments();
    initialMAlignmentAlphabet = obj->getAlphabet();
}

void AlignSequencesToAlignmentTask::prepare()
{
    if (maObj.isNull()) {
        stateInfo.setError(tr("Object is empty."));
        return;
    }

    if (maObj->isStateLocked()) {
        stateInfo.setError(tr("Object is locked for modifications."));
        return;
    }
    Document* document =  maObj->getDocument();
    SAFE_POINT(NULL != document, "MSA object is NULL.", );
    docStateLock = new StateLock("Lock MSA for align sequences to alignment", StateLockFlag_LiveLock);
    document->lockState(docStateLock);
    foreach(Document* curDoc, usedDocuments) {
        curDoc->lockState(docStateLock);
    }

    stateLock = new StateLock("Align sequences to alignment", StateLockFlag_LiveLock);
    maObj->lockState(stateLock);

    AlignmentAlgorithmsRegistry* alignmentRegistry = AppContext::getAlignmentAlgorithmsRegistry();
    SAFE_POINT(NULL != alignmentRegistry, "AlignmentAlgorithmsRegistry is NULL.", );
    AlignmentAlgorithm* addAlgorithm = alignmentRegistry->getAlgorithm(settings.algorithmName);
    SAFE_POINT_EXT(NULL != addAlgorithm, setError(QString("Can not find \"%1\" algorithm").arg(settings.algorithmName)), );
    addSubTask(addAlgorithm->getFactory()->getTaskInstance(&settings));
}

void AlignSequencesToAlignmentTask::fillSettingsByDefault() {
    AlignmentAlgorithmsRegistry* alignmentRegistry = AppContext::getAlignmentAlgorithmsRegistry();
    SAFE_POINT(NULL != alignmentRegistry, "AlignmentAlgorithmsRegistry is NULL.", );
    if(alignmentRegistry->getAvailableAlgorithmIds(AddToAlignment).contains(BaseAlignmentAlgorithmsIds::ALIGN_SEQUENCES_TO_ALIGNMENT_BY_MAFFT) 
        && maObj->getMAlignment().getNumRows() != 0) {
        settings.algorithmName = BaseAlignmentAlgorithmsIds::ALIGN_SEQUENCES_TO_ALIGNMENT_BY_MAFFT;
    } else {
        settings.algorithmName = BaseAlignmentAlgorithmsIds::ALIGN_SEQUENCES_TO_ALIGNMENT_BY_UGENE;
    }
    settings.addAsFragments = sequencesMaxLength < 100 && maObj->getLength() / sequencesMaxLength > 3;
    settings.msaRef = maObj->getEntityRef();
    settings.inNewWindow = false;
}

Task::ReportResult AlignSequencesToAlignmentTask::report() {
    if(stateLock != NULL) {
        maObj->unlockState(stateLock);
        delete stateLock;
    }

    if(docStateLock != NULL) {
        Document* document =  maObj->getDocument();
        document->unlockState(docStateLock);

        foreach(Document* curDoc, usedDocuments) {
            curDoc->unlockState(docStateLock);
        }

        delete docStateLock;
    }
    MAlignmentModInfo mi;
    mi.alphabetChanged = extr.getAlphabet()->getId() != initialMAlignmentAlphabet->getId();
    mi.sequenceListChanged = true;
    if(!hasError() && !isCanceled()) {
        maObj->updateCachedMAlignment(mi);
    }

    return ReportResult_Finished;
}

/************************************************************************/
/* LoadSequencesAndAlignToAlignmentTask */
/************************************************************************/
LoadSequencesAndAlignToAlignmentTask::LoadSequencesAndAlignToAlignmentTask(MAlignmentObject* obj, const QStringList& urls)
: Task(tr("Load sequences and add to alignment task"), TaskFlag_NoRun), urls(urls), maObj(obj), loadSequencesTask(NULL) {
    SAFE_POINT_EXT(obj != NULL, setError("MAlignmentObject is null"), );
    loadSequencesTask = new LoadSequencesTask(obj->getAlphabet(), urls);
    loadSequencesTask->setSubtaskProgressWeight(5);
    addSubTask(loadSequencesTask);
}

QList<Task*> LoadSequencesAndAlignToAlignmentTask::onSubTaskFinished(Task* subTask) {
    QList<Task*> subTasks;
    propagateSubtaskError();
    if(subTask == loadSequencesTask && !loadSequencesTask->hasError() && !loadSequencesTask->isCanceled()) {
        AlignSequencesToAlignmentTask* alignSequencesToAlignmentTask = new AlignSequencesToAlignmentTask(maObj, loadSequencesTask->getExtractor());
        alignSequencesToAlignmentTask->setSubtaskProgressWeight(95);
        subTasks << alignSequencesToAlignmentTask;
    }
    return subTasks;
}

}
