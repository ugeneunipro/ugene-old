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

#include <U2Algorithm/PairwiseAlignmentRegistry.h>
#include <U2Algorithm/PairwiseAlignmentTask.h>

#include <U2Core/AppContext.h>
#include <U2Core/DNAAlphabet.h>
#include <U2Core/DNATranslation.h>
#include <U2Core/L10n.h>
#include <U2Core/LoadDocumentTask.h>
#include <U2Core/MAlignmentImporter.h>
#include <U2Core/TextUtils.h>
#include <U2Core/U2OpStatusUtils.h>
#include <U2Core/U2SafePoints.h>
#include <U2Core/U2SequenceUtils.h>

#include <U2Designer/DelegateEditors.h>

#include <U2Lang/ActorPrototypeRegistry.h>
#include <U2Lang/BaseActorCategories.h>
#include <U2Lang/BasePorts.h>
#include <U2Lang/BaseSlots.h>
#include <U2Lang/BaseTypes.h>
#include <U2Lang/WorkflowEnv.h>

#include "AlignToReferenceWorker.h"

namespace U2 {
namespace LocalWorkflow {

const QString AlignToReferenceWorkerFactory::ACTOR_ID("align-to-reference");
namespace {
    const QString OUT_PORT_ID = "out";
    const QString REF_ATTR_ID = "reference";
}

/************************************************************************/
/* AlignToReferenceWorkerFactory */
/************************************************************************/
AlignToReferenceWorkerFactory::AlignToReferenceWorkerFactory()
: DomainFactory(ACTOR_ID)
{

}

Worker * AlignToReferenceWorkerFactory::createWorker(Actor *a) {
    return new AlignToReferenceWorker(a);
}

void AlignToReferenceWorkerFactory::init() {
    QList<PortDescriptor*> ports;
    {
        Descriptor inDesc(BasePorts::IN_SEQ_PORT_ID(), AlignToReferencePrompter::tr("Input sequence"), AlignToReferencePrompter::tr("Input sequence."));
        Descriptor outDesc(OUT_PORT_ID, AlignToReferencePrompter::tr("Aligned data"), AlignToReferencePrompter::tr("Aligned data."));

        QMap<Descriptor, DataTypePtr> inType;
        inType[BaseSlots::DNA_SEQUENCE_SLOT()] = BaseTypes::DNA_SEQUENCE_TYPE();

        QMap<Descriptor, DataTypePtr> outType;
        outType[BaseSlots::DNA_SEQUENCE_SLOT()] = BaseTypes::DNA_SEQUENCE_TYPE();
        outType[BaseSlots::ANNOTATION_TABLE_SLOT()] = BaseTypes::ANNOTATION_TABLE_TYPE();
        outType[BaseSlots::MULTIPLE_ALIGNMENT_SLOT()] = BaseTypes::MULTIPLE_ALIGNMENT_TYPE();

        ports << new PortDescriptor(inDesc, DataTypePtr(new MapDataType(ACTOR_ID + "-in", inType)), true /*input*/);
        ports << new PortDescriptor(outDesc, DataTypePtr(new MapDataType(ACTOR_ID + "-out", outType)), false /*input*/, true /*multi*/);
    }
    QList<Attribute*> attributes;
    {
        Descriptor refDesc(REF_ATTR_ID, AlignToReferencePrompter::tr("Reference URL"), AlignToReferencePrompter::tr("A URL to the file with a reference sequence."));
        attributes << new Attribute(refDesc, BaseTypes::STRING_TYPE(), true);
    }
    QMap<QString, PropertyDelegate*> delegates;
    {
        delegates[REF_ATTR_ID] = new URLDelegate("", "", false, false, false);
    }

    Descriptor desc(ACTOR_ID, AlignToReferenceWorker::tr("Align to Reference"),
        AlignToReferenceWorker::tr("Align input sequences (e.g. Sanger reads) to the reference sequence."));
    ActorPrototype *proto = new IntegralBusActorPrototype(desc, ports, attributes);
    proto->setEditor(new DelegateEditor(delegates));
    proto->setPrompter(new AlignToReferencePrompter(NULL));
    WorkflowEnv::getProtoRegistry()->registerProto(BaseActorCategories::CATEGORY_ALIGNMENT(), proto);

    DomainFactory *localDomain = WorkflowEnv::getDomainRegistry()->getById(LocalDomainFactory::ID);
    localDomain->registerEntry(new AlignToReferenceWorkerFactory());
}

/************************************************************************/
/* AlignToReferencePrompter */
/************************************************************************/
AlignToReferencePrompter::AlignToReferencePrompter(Actor *a)
: PrompterBase<AlignToReferencePrompter>(a)
{

}

QString AlignToReferencePrompter::composeRichDoc() {
    IntegralBusPort *input = qobject_cast<IntegralBusPort*>(target->getPort(BasePorts::IN_SEQ_PORT_ID()));
    SAFE_POINT(NULL != input, "No input port", "");
    Actor *producer = input->getProducer(BaseSlots::DNA_SEQUENCE_SLOT().getId());
    const QString unsetStr = "<font color='red'>"+tr("unset")+"</font>";
    const QString producerName = (NULL != producer) ? producer->getLabel() : unsetStr;
    const QString refLink = getHyperlink(REF_ATTR_ID, getURL(REF_ATTR_ID));
    return tr("Aligns each sequence from <u>%1</u> to the reference sequence from <u>%2</u>.").arg(producerName).arg(refLink);
}

/************************************************************************/
/* AlignToReferenceWorker */
/************************************************************************/
AlignToReferenceWorker::AlignToReferenceWorker(Actor *a)
: BaseDatasetWorker(a, BasePorts::IN_SEQ_PORT_ID(), OUT_PORT_ID), referenceDoc(NULL)
{

}

void AlignToReferenceWorker::cleanup() {
    delete referenceDoc;
    BaseDatasetWorker::cleanup();
}

Task * AlignToReferenceWorker::createPrepareTask(U2OpStatus &os) const {
    QString referenceUrl = getValue<QString>(REF_ATTR_ID);
    QVariantMap hints;
    hints[DocumentFormat::DBI_REF_HINT] = qVariantFromValue(context->getDataStorage()->getDbiRef());
    LoadDocumentTask *task = LoadDocumentTask::getDefaultLoadDocTask(referenceUrl, hints);
    if (NULL == task) {
        os.setError(tr("Can not read the reference file: ") + referenceUrl);
    }
    return task;
}

void AlignToReferenceWorker::onPrepared(Task *task, U2OpStatus &os) {
    LoadDocumentTask *loadTask = dynamic_cast<LoadDocumentTask*>(task);
    CHECK_EXT(NULL != loadTask, os.setError(L10N::internalError("Unexpected prepare task")), );

    QScopedPointer<Document> doc(loadTask->takeDocument());
    CHECK_EXT(!doc.isNull(), os.setError(tr("Can't read the file: ") + loadTask->getURLString()), );
    QList<GObject*> objects = doc->findGObjectByType(GObjectTypes::SEQUENCE);
    CHECK_EXT(!objects.isEmpty(), os.setError(tr("No reference sequence in the file: ") + loadTask->getURLString()), );
    CHECK_EXT(1 == objects.size(), os.setError(tr("More than one sequence in the reference file: ") + loadTask->getURLString()), );

    referenceDoc = doc.take();
    referenceDoc->setDocumentOwnsDbiResources(false);
    reference = context->getDataStorage()->getDataHandler(objects.first()->getEntityRef());
}

Task * AlignToReferenceWorker::createTask(const QList<Message> &messages) const {
    QList<SharedDbiDataHandler> reads;
    foreach (const Message &message, messages) {
        QVariantMap data = message.getData().toMap();
        if (data.contains(BaseSlots::DNA_SEQUENCE_SLOT().getId())) {
            reads << data[BaseSlots::DNA_SEQUENCE_SLOT().getId()].value<SharedDbiDataHandler>();
        }
    }
    return new AlignToReferenceTask(reference, reads, context->getDataStorage());
}

QVariantMap AlignToReferenceWorker::getResult(Task *task, U2OpStatus &os) const {
    AlignToReferenceTask *alignTask = dynamic_cast<AlignToReferenceTask*>(task);
    CHECK_EXT(NULL != alignTask, os.setError(L10N::internalError("Unexpected task")), QVariantMap());
    QVariantMap result;
    result[BaseSlots::MULTIPLE_ALIGNMENT_SLOT().getId()] = qVariantFromValue<SharedDbiDataHandler>(alignTask->getResult());
    return result;
}

/************************************************************************/
/* AlignToReferenceTask */
/************************************************************************/
AlignToReferenceTask::AlignToReferenceTask(const SharedDbiDataHandler &reference, const QList<SharedDbiDataHandler> &reads, DbiDataStorage *storage)
: Task(tr("Align to reference"), TaskFlags_FOSE_COSC), reference(reference), reads(reads), storage(storage)
{

}

SharedDbiDataHandler AlignToReferenceTask::getResult() const {
    return msa;
}

void AlignToReferenceTask::prepare() {
    foreach (const SharedDbiDataHandler &read, reads) {
        SharedDbiDataHandler rcRead = createRcRead(read);
        CHECK_OP(stateInfo, );

        rcReads << rcRead;

        KAlignSubTask *subTask = new KAlignSubTask(reference, read, storage);
        subTasks << subTask;
        addSubTask(subTask);

        KAlignSubTask *rcSubTask = new KAlignSubTask(reference, rcRead, storage);
        rcSubTasks << rcSubTask;
        addSubTask(rcSubTask);
    }
}

void AlignToReferenceTask::run() {
    DNASequence referenceSeq = getReferenceSequence();
    CHECK_OP(stateInfo, );
    QList<U2MsaGap> referenceGaps = getReferenceGaps();
    CHECK_OP(stateInfo, );

    MAlignment result("Aligned reads", referenceSeq.alphabet);

    // add the reference row
    result.addRow(referenceSeq.getName(), referenceSeq.seq, 0, stateInfo);
    CHECK_OP(stateInfo, );

    insertShiftedGapsIntoReference(result, referenceGaps);
    CHECK_OP(stateInfo, );

    for (int i=0; i<reads.size(); i++) {
        // add the read row
        bool rc = false;
        KAlignSubTask *subTask = getBestSubTask(i, rc);
        CHECK_OP(stateInfo, );

        DNASequence readSeq = getReadSequence(i, rc);
        CHECK_OP(stateInfo, );

        result.addRow(readSeq.getName(), readSeq.seq, i + 1, stateInfo);
        CHECK_OP(stateInfo, );
        result.setRowGapModel(i + 1, subTask->getReadGaps());

        // add reference gaps to the read
        insertShiftedGapsIntoRead(result, i, referenceGaps);
        CHECK_OP(stateInfo, );
    }

    U2EntityRef msaRef = MAlignmentImporter::createAlignment(storage->getDbiRef(), result, stateInfo);
    CHECK_OP(stateInfo, );
    msa = storage->getDataHandler(msaRef);
}

QByteArray AlignToReferenceTask::getReverseComplement(const QByteArray &sequence, const DNAAlphabet *alphabet) {
    DNATranslation *translator = AppContext::getDNATranslationRegistry()->lookupComplementTranslation(alphabet);
    CHECK_EXT(NULL != translator, setError(tr("Can't translate read sequence to reverse complement")), "");

    QByteArray translation(sequence.length(), 0);
    translator->translate(sequence.constData(), sequence.length(), translation.data(), translation.length());
    TextUtils::reverse(translation.data(), translation.length());
    return translation;
}

SharedDbiDataHandler AlignToReferenceTask::createRcRead(const SharedDbiDataHandler &read) {
    QScopedPointer<U2SequenceObject> readObject(StorageUtils::getSequenceObject(storage, read));
    CHECK_EXT(!readObject.isNull(), setError(L10N::nullPointerError("Read sequence")), SharedDbiDataHandler());

    DNASequence seq = readObject->getWholeSequence();
    seq.setName(seq.getName() + "_rev");
    seq.seq = getReverseComplement(seq.seq, readObject->getAlphabet());
    CHECK_OP(stateInfo, SharedDbiDataHandler());

    U2EntityRef rcRef = U2SequenceUtils::import(storage->getDbiRef(), seq, stateInfo);
    CHECK_OP(stateInfo, SharedDbiDataHandler());

    return storage->getDataHandler(rcRef);
}

KAlignSubTask * AlignToReferenceTask::getBestSubTask(int readNum, bool &rc) {
    CHECK_EXT(readNum < subTasks.size(), setError(L10N::internalError("Wrong reads number")), NULL);
    KAlignSubTask *subTask = subTasks[readNum];
    KAlignSubTask *rcSubTask = rcSubTasks[readNum];
    rc = (subTask->getMaxChunkSize() < rcSubTask->getMaxChunkSize());
    if (rc) {
        return rcSubTask;
    } else {
        return subTask;
    }
}

DNASequence AlignToReferenceTask::getReadSequence(int readNum, bool rc) {
    SharedDbiDataHandler read;
    if (rc) {
        read = rcReads[readNum];
    } else {
        read = reads[readNum];
    }
    QScopedPointer<U2SequenceObject> readObject(StorageUtils::getSequenceObject(storage, read));
    CHECK_EXT(!readObject.isNull(), setError(L10N::nullPointerError("Read sequence")), DNASequence());
    return readObject->getWholeSequence();
}

DNASequence AlignToReferenceTask::getReferenceSequence() {
    QScopedPointer<U2SequenceObject> refObject(StorageUtils::getSequenceObject(storage, reference));
    CHECK_EXT(!refObject.isNull(), setError(L10N::nullPointerError("Reference sequence")), DNASequence());
    return refObject->getWholeSequence();
}

namespace {
    bool compare(const U2MsaGap &gap1, const U2MsaGap &gap2) {
        return gap1.offset < gap2.offset;
    }
}

QList<U2MsaGap> AlignToReferenceTask::getReferenceGaps() {
    QList<U2MsaGap> result;

    for (int i=0; i<reads.size(); i++) {
        result << getShiftedGaps(i);
        CHECK_OP(stateInfo, result);
    }
    qSort(result.begin(), result.end(), compare);
    return result;
}

QList<U2MsaGap> AlignToReferenceTask::getShiftedGaps(int rowNum) {
    QList<U2MsaGap> result;

    bool rc = false;
    KAlignSubTask *subTask = getBestSubTask(rowNum, rc);
    CHECK_OP(stateInfo, result);

    qint64 wholeGap = 0;
    foreach (const U2MsaGap &gap, subTask->getReferenceGaps()) {
        result << U2MsaGap(gap.offset - wholeGap, gap.gap);
        wholeGap += gap.gap;
    }
    return result;
}

void AlignToReferenceTask::insertShiftedGapsIntoReference(MAlignment &alignment, const QList<U2MsaGap> &gaps) {
    for (int i=gaps.size() - 1; i>=0; i--) {
        U2MsaGap gap = gaps[i];
        alignment.insertGaps(0, gap.offset, gap.gap, stateInfo);
        CHECK_OP(stateInfo, );
    }
}

void AlignToReferenceTask::insertShiftedGapsIntoRead(MAlignment &alignment, int readNum, const QList<U2MsaGap> &gaps) {
    QList<U2MsaGap> ownGaps = getShiftedGaps(readNum);
    CHECK_OP(stateInfo, );

    qint64 globalOffset = 0;
    foreach (const U2MsaGap &gap, gaps) {
        if (ownGaps.contains(gap)) { // task own gaps into account but don't insert them
            globalOffset += gap.gap;
            ownGaps.removeOne(gap);
            continue;
        }
        alignment.insertGaps(readNum + 1, globalOffset + gap.offset, gap.gap, stateInfo);
        CHECK_OP(stateInfo, );
        globalOffset += gap.gap;
    }
}

/************************************************************************/
/* KAlignSubTask */
/************************************************************************/
KAlignSubTask::KAlignSubTask(const SharedDbiDataHandler &reference, const SharedDbiDataHandler &read, DbiDataStorage *storage)
: Task("KAlign task wrapper", TaskFlags_FOSE_COSC), reference(reference), read(read), storage(storage), maxChunkSize(0)
{

}

qint64 KAlignSubTask::getMaxChunkSize() const {
    return maxChunkSize;
}

QList<U2MsaGap> KAlignSubTask::getReferenceGaps() const {
    return referenceGaps;
}

QList<U2MsaGap> KAlignSubTask::getReadGaps() const {
    return readGaps;
}

void KAlignSubTask::prepare() {
    QScopedPointer<U2SequenceObject> refObject(StorageUtils::getSequenceObject(storage, reference));
    CHECK_EXT(!refObject.isNull(), setError(L10N::nullPointerError("Reference sequence")), );
    QScopedPointer<U2SequenceObject> readObject(StorageUtils::getSequenceObject(storage, read));
    CHECK_EXT(!readObject.isNull(), setError(L10N::nullPointerError("Read sequence")), );

    createAlignment(refObject.data(), readObject.data());
    CHECK_OP(stateInfo, );

    PairwiseAlignmentTaskFactory *factory = getPairwiseAlignmentTaskFactory(stateInfo);
    CHECK_OP(stateInfo, );

    PairwiseAlignmentTaskSettings *settings = createSettings();
    CHECK_OP(stateInfo, );
    addSubTask(factory->getTaskInstance(settings));
}

void KAlignSubTask::run() {
    QScopedPointer<MAlignmentObject> msaObject(StorageUtils::getMsaObject(storage, msa));
    CHECK_EXT(!msaObject.isNull(), setError(L10N::nullPointerError("MSA object")), );
    int rowCount = msaObject->getNumRows();
    CHECK_EXT(2 == rowCount, setError(L10N::internalError("Wrong rows count: " + QString::number(rowCount))), );

    MAlignmentRow readRow = msaObject->getRow(1);
    referenceGaps = msaObject->getRow(0).getGapModel();
    readGaps = readRow.getGapModel();

    maxChunkSize = 0;
    qint64 pos = 0;
    foreach (const U2MsaGap &gap, readRow.getGapModel()) {
        qint64 chunkSize = gap.offset - pos;
        if (chunkSize > maxChunkSize) {
            maxChunkSize = chunkSize;
        }
        pos = gap.offset + gap.gap;
    }
    qint64 rowLength = readRow.getRowLengthWithoutTrailing();
    if (pos < rowLength) {
        qint64 chunkSize = rowLength - pos;
        if (chunkSize > maxChunkSize) {
            maxChunkSize = chunkSize;
        }
    }
    algoLog.details(QString::number(maxChunkSize));
}

PairwiseAlignmentTaskFactory * KAlignSubTask::getPairwiseAlignmentTaskFactory(U2OpStatus &os) const {
    PairwiseAlignmentAlgorithm *algo = AppContext::getPairwiseAlignmentRegistry()->getAlgorithm("Hirschberg (KAlign)");
    CHECK_EXT(NULL != algo, os.setError(tr("The KAlign algorithm is not found. Add the KAlign plugin.")), NULL);

    const QStringList implementations = algo->getRealizationsList();
    CHECK_EXT(!implementations.isEmpty(), os.setError(tr("Implementation of the KAlign algorithm is not found. Check that the KAlign plugin is up to date.")), NULL);

    AlgorithmRealization *algoImpl = algo->getAlgorithmRealization(implementations.first());
    CHECK_EXT(NULL != algoImpl, os.setError(tr("The KAlign algorithm is not found. Check that the KAlign plugin is up to date.")), NULL);

    return algoImpl->getTaskFactory();
}

void KAlignSubTask::createAlignment(U2SequenceObject *refObject, U2SequenceObject *readObject) {
    MAlignment alignment("msa", refObject->getAlphabet());
    alignment.addRow(refObject->getSequenceName(), refObject->getWholeSequenceData(), stateInfo);
    CHECK_OP(stateInfo, );
    alignment.addRow(readObject->getSequenceName(), readObject->getWholeSequenceData(), stateInfo);
    CHECK_OP(stateInfo, );

    U2EntityRef msaRef = MAlignmentImporter::createAlignment(storage->getDbiRef(), alignment, stateInfo);
    CHECK_OP(stateInfo, );
    msa = storage->getDataHandler(msaRef);
}

PairwiseAlignmentTaskSettings * KAlignSubTask::createSettings() {
    QScopedPointer<MAlignmentObject> msaObject(StorageUtils::getMsaObject(storage, msa));
    CHECK_EXT(!msaObject.isNull(), setError(L10N::nullPointerError("MSA object")), NULL);

    U2DataId refrenceId = msaObject->getRow(0).getRowDBInfo().sequenceId;
    U2DataId readId = msaObject->getRow(1).getRowDBInfo().sequenceId;

    PairwiseAlignmentTaskSettings *settings = new PairwiseAlignmentTaskSettings();
    settings->alphabet = msaObject->getAlphabet()->getId();
    settings->inNewWindow = false;
    settings->msaRef = msaObject->getEntityRef();
    settings->firstSequenceRef = U2EntityRef(msaObject->getEntityRef().dbiRef, refrenceId);
    settings->secondSequenceRef = U2EntityRef(msaObject->getEntityRef().dbiRef, readId);
    return settings;
}

} // LocalWorkflow
} // U2
