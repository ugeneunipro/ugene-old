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

#include <U2Core/AnnotationTableObject.h>
#include <U2Core/AppContext.h>
#include <U2Core/AppResources.h>
#include <U2Core/AppSettings.h>
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

#include <U2Formats/GenbankFeatures.h>

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
    result[BaseSlots::MULTIPLE_ALIGNMENT_SLOT().getId()] = qVariantFromValue<SharedDbiDataHandler>(alignTask->getAlignment());
    result[BaseSlots::DNA_SEQUENCE_SLOT().getId()] = qVariantFromValue<SharedDbiDataHandler>(reference);
    result[BaseSlots::ANNOTATION_TABLE_SLOT().getId()] = qVariantFromValue<SharedDbiDataHandler>(alignTask->getAnnotations());
    return result;
}

MessageMetadata AlignToReferenceWorker::generateMetadata(const QString &datasetName) const {
    SAFE_POINT(NULL != referenceDoc, L10N::nullPointerError("Reference sequence document"), BaseDatasetWorker::generateMetadata(datasetName));
    return MessageMetadata(referenceDoc->getURLString(), datasetName);
}

/************************************************************************/
/* AlignToReferenceTask */
/************************************************************************/
namespace {
    qint64 calcMemUsageBytes(DbiDataStorage *storage, const SharedDbiDataHandler &seqId, U2OpStatus &os) {
        QScopedPointer<U2SequenceObject> object(StorageUtils::getSequenceObject(storage, seqId));
        CHECK_EXT(!object.isNull(), os.setError(L10N::nullPointerError("Sequence object")), 0);

        return object->getSequenceLength();
    }

    int toMb(qint64 bytes) {
        return 0.5 + (double(bytes) / (1024 * 1024));
    }
}

AlignToReferenceTask::AlignToReferenceTask(const SharedDbiDataHandler &reference, const QList<SharedDbiDataHandler> &reads, DbiDataStorage *storage)
: Task(tr("Align to reference"), TaskFlags_NR_FOSE_COSC), reference(reference), reads(reads), composeSubTask(NULL), storage(storage), subTasksCount(0)
{
    setMaxParallelSubtasks(AppContext::getAppSettings()->getAppResourcePool()->getIdealThreadCount());
}

void AlignToReferenceTask::prepare() {
    foreach (const SharedDbiDataHandler &read, reads) {
        PairwiseAlignmentTask *subTask = new PairwiseAlignmentTask(reference, read, storage);
        subTasks << subTask;
        subTask->setSubtaskProgressWeight(1.0f / (2 * reads.count()));
        addSubTask(subTask);
    }
    subTasksCount = subTasks.size();
}

QList<Task*> AlignToReferenceTask::onSubTaskFinished(Task *subTask) {
    QList<Task*> result;
    if (1 == subTasksCount) {
        composeSubTask = new ComposeResultSubTask(reference, reads, subTasks, storage);
        composeSubTask->setSubtaskProgressWeight(0.5f);
        result << composeSubTask;
    }
    subTasksCount--;
    return result;
}

SharedDbiDataHandler AlignToReferenceTask::getAlignment() const {
    CHECK(NULL != composeSubTask, SharedDbiDataHandler());
    return composeSubTask->getAlignment();
}

SharedDbiDataHandler AlignToReferenceTask::getAnnotations() const {
    CHECK(NULL != composeSubTask, SharedDbiDataHandler());
    return composeSubTask->getAnnotations();
}

/************************************************************************/
/* ComposeResultSubTask */
/************************************************************************/
ComposeResultSubTask::ComposeResultSubTask(const SharedDbiDataHandler &reference, const QList<SharedDbiDataHandler> &reads, const QList<PairwiseAlignmentTask*> subTasks, DbiDataStorage *storage)
: Task(tr("Compose alignment"), TaskFlags_FOSE_COSC), reference(reference), reads(reads), subTasks(subTasks), storage(storage)
{
    tpm = Task::Progress_Manual;
}

void ComposeResultSubTask::prepare() {
    qint64 memUsage = calcMemUsageBytes(storage, reference, stateInfo);
    CHECK_OP(stateInfo, );

    foreach (const SharedDbiDataHandler &read, reads) {
        memUsage += calcMemUsageBytes(storage, read, stateInfo);
        CHECK_OP(stateInfo, );
    }
    if (toMb(memUsage) > 0) {
        addTaskResource(TaskResourceUsage(RESOURCE_MEMORY, toMb(memUsage), false));
    }
}

void ComposeResultSubTask::run() {
    MAlignment alignment = createAlignment();
    CHECK_OP(stateInfo, );

    createAnnotations(alignment);
    CHECK_OP(stateInfo, );
}

SharedDbiDataHandler ComposeResultSubTask::getAlignment() const {
    return msa;
}

SharedDbiDataHandler ComposeResultSubTask::getAnnotations() const {
    return annotations;
}

MAlignment ComposeResultSubTask::createAlignment() {
    MAlignment result("Aligned reads");

    DNASequence referenceSeq = getReferenceSequence();
    CHECK_OP(stateInfo, result);
    result.setAlphabet(referenceSeq.alphabet);

    // add the reference row
    result.addRow(referenceSeq.getName(), referenceSeq.seq, 0, stateInfo);
    CHECK_OP(stateInfo, result);

    QList<U2MsaGap> referenceGaps = getReferenceGaps();
    CHECK_OP(stateInfo, result);

    insertShiftedGapsIntoReference(result, referenceGaps);
    CHECK_OP(stateInfo, result);

    for (int i=0; i<reads.size(); i++) {
        // add the read row
        DNASequence readSeq = getReadSequence(i);
        CHECK_OP(stateInfo, result);

        result.addRow(readSeq.getName(), readSeq.seq, i + 1, stateInfo);
        CHECK_OP(stateInfo, result);

        PairwiseAlignmentTask *subTask = getPATask(i);
        CHECK_OP(stateInfo, result);
        foreach (const U2MsaGap &gap, subTask->getReadGaps()) {
            result.insertGaps(i + 1, gap.offset, gap.gap, stateInfo);
            CHECK_OP(stateInfo, result);
        }

        // add reference gaps to the read
        insertShiftedGapsIntoRead(result, i, referenceGaps);
        CHECK_OP(stateInfo, result);
    }

    U2EntityRef msaRef = MAlignmentImporter::createAlignment(storage->getDbiRef(), result, stateInfo);
    CHECK_OP(stateInfo, result);
    msa = storage->getDataHandler(msaRef);

    // remove gap columns
    QScopedPointer<MAlignmentObject> msaObject(StorageUtils::getMsaObject(storage, msa));
    CHECK_EXT(!msaObject.isNull(), setError(L10N::nullPointerError("MSA object")), result);
    msaObject->deleteColumnWithGaps(GAP_COLUMN_ONLY, stateInfo);

    return msaObject->getMAlignment();
}

void ComposeResultSubTask::createAnnotations(const MAlignment &alignment) {
    const MAlignmentRow &referenceRow = alignment.getRow(0);
    QScopedPointer<AnnotationTableObject> annsObject(new AnnotationTableObject(referenceRow.getName() + " features", storage->getDbiRef()));

    for (int i=1; i<alignment.getNumRows(); i++) {
        const MAlignmentRow &readRow = alignment.getRow(i);
        U2Region region = getReadRegion(readRow, referenceRow);
        PairwiseAlignmentTask *task = getPATask(i - 1);
        CHECK_OP(stateInfo, );

        AnnotationData ann;
        ann.location = getLocation(region, task->isComplement(), referenceRow.getSequence().length());
        ann.name = GBFeatureUtils::getKeyInfo(GBFeatureKey_misc_feature).text;
        ann.qualifiers << U2Qualifier("label", task->getInitialReadName());
        annsObject->addAnnotation(ann);
    }

    annotations = storage->getDataHandler(annsObject->getEntityRef());
}

U2Region ComposeResultSubTask::getReadRegion(const MAlignmentRow &readRow, const MAlignmentRow &referenceRow) const {
    U2Region region(0, readRow.getRowLengthWithoutTrailing());

    // calculate read start
    if (!readRow.getGapModel().isEmpty()) {
        U2MsaGap firstGap = readRow.getGapModel().first();
        if (0 == firstGap.offset) {
            region.startPos += firstGap.gap;
            region.length -= firstGap.gap;
        }
    }

    qint64 leftGap = 0;
    qint64 innerGap = 0;
    foreach (const U2MsaGap &gap, referenceRow.getGapModel()) {
        qint64 endPos = gap.offset + gap.gap;
        if (gap.offset < region.startPos) {
            leftGap += gap.gap;
        } else if (endPos <= region.endPos()) {
            innerGap += gap.gap;
        } else {
            break;
        }
    }

    region.startPos -= leftGap;
    region.length -= innerGap;
    return region;
}

U2Location ComposeResultSubTask::getLocation(const U2Region &region, bool isComplement, qint64 referenceLength) {
    U2Location result;
    result->strand = isComplement ? U2Strand(U2Strand::Complementary) : U2Strand(U2Strand::Direct);

    if (region.startPos < 0) {
        // TODO: just trim the region because it is incorrect to make the annotation circular: the left (negative) part of the read is not aligned.
        // Fix it when the task can work with circular references.
        result->regions << U2Region(0, region.length + region.startPos);
    } else {
        result->regions << region;
    }

    return result;
}

PairwiseAlignmentTask * ComposeResultSubTask::getPATask(int readNum) {
    CHECK_EXT(readNum < subTasks.size(), setError(L10N::internalError("Wrong reads number")), NULL);
    return subTasks[readNum];
}

DNASequence ComposeResultSubTask::getReadSequence(int readNum) {
    PairwiseAlignmentTask *subTask = getPATask(readNum);
    CHECK_OP(stateInfo, DNASequence());

    QScopedPointer<U2SequenceObject> readObject(StorageUtils::getSequenceObject(storage, subTask->getRead()));
    CHECK_EXT(!readObject.isNull(), setError(L10N::nullPointerError("Read sequence")), DNASequence());
    return readObject->getWholeSequence();
}

DNASequence ComposeResultSubTask::getReferenceSequence() {
    QScopedPointer<U2SequenceObject> refObject(StorageUtils::getSequenceObject(storage, reference));
    CHECK_EXT(!refObject.isNull(), setError(L10N::nullPointerError("Reference sequence")), DNASequence());
    return refObject->getWholeSequence();
}

namespace {
    bool compare(const U2MsaGap &gap1, const U2MsaGap &gap2) {
        return gap1.offset < gap2.offset;
    }
}

QList<U2MsaGap> ComposeResultSubTask::getReferenceGaps() {
    QList<U2MsaGap> result;

    for (int i=0; i<reads.size(); i++) {
        result << getShiftedGaps(i);
        CHECK_OP(stateInfo, result);
    }
    qSort(result.begin(), result.end(), compare);
    return result;
}

QList<U2MsaGap> ComposeResultSubTask::getShiftedGaps(int rowNum) {
    QList<U2MsaGap> result;

    PairwiseAlignmentTask *subTask = getPATask(rowNum);
    CHECK_OP(stateInfo, result);

    qint64 wholeGap = 0;
    foreach (const U2MsaGap &gap, subTask->getReferenceGaps()) {
        result << U2MsaGap(gap.offset - wholeGap, gap.gap);
        wholeGap += gap.gap;
    }
    return result;
}

void ComposeResultSubTask::insertShiftedGapsIntoReference(MAlignment &alignment, const QList<U2MsaGap> &gaps) {
    for (int i=gaps.size() - 1; i>=0; i--) {
        U2MsaGap gap = gaps[i];
        alignment.insertGaps(0, gap.offset, gap.gap, stateInfo);
        CHECK_OP(stateInfo, );
    }
}

void ComposeResultSubTask::insertShiftedGapsIntoRead(MAlignment &alignment, int readNum, const QList<U2MsaGap> &gaps) {
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
const qint64 KAlignSubTask::MAX_GAP_SIZE = 10;
const qint64 KAlignSubTask::EXTENSION_COEF = 2;

KAlignSubTask::KAlignSubTask(const SharedDbiDataHandler &reference, const SharedDbiDataHandler &read, DbiDataStorage *storage)
: Task("KAlign task wrapper", TaskFlags_FOSE_COSC), reference(reference), read(read), storage(storage), maxRegionSize(0)
{

}

const SharedDbiDataHandler KAlignSubTask::getRead() const {
    return read;
}

qint64 KAlignSubTask::getMaxRegionSize() const {
    return maxRegionSize;
}

U2Region KAlignSubTask::getCoreRegion() const {
    return coreRegion;
}

void KAlignSubTask::prepare() {
    qint64 memUsage = calcMemUsageBytes(storage, reference, stateInfo) + calcMemUsageBytes(storage, read, stateInfo);
    CHECK_OP(stateInfo, );
    if (toMb(memUsage) > 0) {
        addTaskResource(TaskResourceUsage(RESOURCE_MEMORY, toMb(memUsage), false));
    }

    createAlignment();
    CHECK_OP(stateInfo, );

    PairwiseAlignmentTaskFactory *factory = getPairwiseAlignmentTaskFactory("Hirschberg (KAlign)", "KAlign", stateInfo);
    CHECK_OP(stateInfo, );

    QScopedPointer<PairwiseAlignmentTaskSettings> settings(createSettings(storage, msa, stateInfo));
    CHECK_OP(stateInfo, );
    addSubTask(factory->getTaskInstance(settings.take()));
}

void KAlignSubTask::run() {
    QScopedPointer<MAlignmentObject> msaObject(StorageUtils::getMsaObject(storage, msa));
    CHECK_EXT(!msaObject.isNull(), setError(L10N::nullPointerError("MSA object")), );
    int rowCount = msaObject->getNumRows();
    CHECK_EXT(2 == rowCount, setError(L10N::internalError("Wrong rows count: " + QString::number(rowCount))), );

    MAlignmentRow readRow = msaObject->getRow(1);
    QList<U2Region> regions = getRegions(readRow.getGapModel(), readRow.getRowLengthWithoutTrailing());
    calculateCoreRegion(regions);
    extendCoreRegion(regions);

    algoLog.details(tr("Max region size: %1").arg(maxRegionSize));
    algoLog.details(tr("Core region: %1-%2").arg(coreRegion.startPos).arg(coreRegion.endPos()-1));
}

QList<U2Region> KAlignSubTask::getRegions(const QList<U2MsaGap> &gaps, qint64 rowLength) const {
    QList<U2Region> regions;
    qint64 startPos = 0;
    foreach (const U2MsaGap &gap, gaps) {
        qint64 length = gap.offset - startPos;
        if (length > 0) {
            regions << U2Region(startPos, length);
        }
        startPos = gap.offset + gap.gap;
    }
    if (startPos < rowLength) {
        qint64 length = rowLength - startPos;
        regions << U2Region(startPos, length);
    }
    return regions;
}

void KAlignSubTask::calculateCoreRegion(const QList<U2Region> &regions) {
    coreRegion = U2Region(0, 0);
    maxRegionSize = 0;
    U2Region currentRegion = coreRegion;
    foreach (const U2Region &region, regions) {
        if (maxRegionSize < region.length) {
            maxRegionSize = region.length;
        }
        if (0 == currentRegion.length) {
            currentRegion = region;
            continue;
        }
        if (region.startPos - currentRegion.endPos() < MAX_GAP_SIZE) {
            currentRegion.length = region.endPos() - currentRegion.startPos;
        } else {
            if (coreRegion.length < currentRegion.length) {
                coreRegion = currentRegion;
            }
            currentRegion = region;
        }
    }
    if (coreRegion.length < currentRegion.length) {
        coreRegion = currentRegion;
    }
}

void KAlignSubTask::extendCoreRegion(const QList<U2Region> &regions) {
    qint64 leftSize = 0;
    qint64 rightSize = 0;
    foreach (const U2Region &region, regions) {
        if (region.endPos() - 1 < coreRegion.startPos) {
            leftSize += region.length;
        }
        if (region.startPos > coreRegion.endPos() - 1) {
            rightSize += region.length;
        }
    }
    coreRegion.startPos -= leftSize * EXTENSION_COEF;
    coreRegion.length += (leftSize + rightSize) * EXTENSION_COEF;
}

PairwiseAlignmentTaskFactory * KAlignSubTask::getPairwiseAlignmentTaskFactory(const QString &algoId, const QString &implId, U2OpStatus &os) {
    PairwiseAlignmentAlgorithm *algo = AppContext::getPairwiseAlignmentRegistry()->getAlgorithm(algoId);
    CHECK_EXT(NULL != algo, os.setError(tr("The %1 algorithm is not found. Add the %1 plugin.").arg(algoId)), NULL);

    AlgorithmRealization *algoImpl = algo->getAlgorithmRealization(implId);
    CHECK_EXT(NULL != algoImpl, os.setError(tr("The %1 algorithm is not found. Check that the %1 plugin is up to date.").arg(algoId)), NULL);

    return algoImpl->getTaskFactory();
}

void KAlignSubTask::createAlignment() {
    QScopedPointer<U2SequenceObject> refObject(StorageUtils::getSequenceObject(storage, reference));
    CHECK_EXT(!refObject.isNull(), setError(L10N::nullPointerError("Reference sequence")), );
    QScopedPointer<U2SequenceObject> readObject(StorageUtils::getSequenceObject(storage, read));
    CHECK_EXT(!readObject.isNull(), setError(L10N::nullPointerError("Read sequence")), );

    MAlignment alignment("msa", refObject->getAlphabet());
    alignment.addRow(refObject->getSequenceName(), refObject->getWholeSequenceData(), stateInfo);
    CHECK_OP(stateInfo, );
    alignment.addRow(readObject->getSequenceName(), readObject->getWholeSequenceData(), stateInfo);
    CHECK_OP(stateInfo, );

    U2EntityRef msaRef = MAlignmentImporter::createAlignment(storage->getDbiRef(), alignment, stateInfo);
    CHECK_OP(stateInfo, );
    msa = storage->getDataHandler(msaRef);
}

PairwiseAlignmentTaskSettings * KAlignSubTask::createSettings(DbiDataStorage *storage, const SharedDbiDataHandler &msa, U2OpStatus &os) {
    QScopedPointer<MAlignmentObject> msaObject(StorageUtils::getMsaObject(storage, msa));
    CHECK_EXT(!msaObject.isNull(), os.setError(L10N::nullPointerError("MSA object")), NULL);

    U2DataId referenceId = msaObject->getRow(0).getRowDBInfo().sequenceId;
    U2DataId readId = msaObject->getRow(1).getRowDBInfo().sequenceId;

    PairwiseAlignmentTaskSettings *settings = new PairwiseAlignmentTaskSettings();
    settings->alphabet = msaObject->getAlphabet()->getId();
    settings->inNewWindow = false;
    settings->msaRef = msaObject->getEntityRef();
    settings->firstSequenceRef = U2EntityRef(msaObject->getEntityRef().dbiRef, referenceId);
    settings->secondSequenceRef = U2EntityRef(msaObject->getEntityRef().dbiRef, readId);
    return settings;
}

/************************************************************************/
/* PairwiseAlignmentTask */
/************************************************************************/
PairwiseAlignmentTask::PairwiseAlignmentTask(const SharedDbiDataHandler &reference, const SharedDbiDataHandler &read, DbiDataStorage *storage)
: Task("Pairwise Alignment", TaskFlags_FOSE_COSC), reference(reference), read(read), storage(storage), kalign(NULL), rKalign(NULL), cKalign(NULL), rcKalign(NULL), reverse(false), complement(false), offset(0)
{
    setMaxParallelSubtasks(2);
}

void PairwiseAlignmentTask::prepare() {
    qint64 memUsage = calcMemUsageBytes(storage, reference, stateInfo) + calcMemUsageBytes(storage, read, stateInfo);
    CHECK_OP(stateInfo, );
    if (toMb(memUsage) > 0) {
        addTaskResource(TaskResourceUsage(RESOURCE_MEMORY, toMb(memUsage), false));
    }

    createRcReads();
    CHECK_OP(stateInfo, );

    kalign = new KAlignSubTask(reference, read, storage);
    rKalign = new KAlignSubTask(reference, rRead, storage);
    cKalign = new KAlignSubTask(reference, cRead, storage);
    rcKalign = new KAlignSubTask(reference, rcRead, storage);
    addSubTask(kalign);
    addSubTask(rKalign);
    addSubTask(cKalign);
    addSubTask(rcKalign);
}

QList<Task*> PairwiseAlignmentTask::onSubTaskFinished(Task *subTask) {
    QList<Task*> result;
    CHECK((kalign == subTask) || (rKalign == subTask) || (cKalign == subTask) || (rcKalign == subTask), result);
    CHECK(kalign->isFinished() && rKalign->isFinished() && cKalign->isFinished() && rcKalign->isFinished(), result);

    createSWAlignment(initRc());

    PairwiseAlignmentTaskFactory *factory = KAlignSubTask::getPairwiseAlignmentTaskFactory("Smith-Waterman", "SW_classic", stateInfo);
    CHECK_OP(stateInfo, result);

    QScopedPointer<PairwiseAlignmentTaskSettings> settings(KAlignSubTask::createSettings(storage, msa, stateInfo));
    CHECK_OP(stateInfo, result);
    settings->setCustomValue("SW_gapOpen", -10);
    settings->setCustomValue("SW_gapExtd", -1);
    settings->setCustomValue("SW_scoringMatrix", "dna");

    result << factory->getTaskInstance(settings.take());
    return result;
}

void PairwiseAlignmentTask::run() {
    QScopedPointer<MAlignmentObject> msaObject(StorageUtils::getMsaObject(storage, msa));
    CHECK_EXT(!msaObject.isNull(), setError(L10N::nullPointerError("MSA object")), );
    int rowCount = msaObject->getNumRows();
    CHECK_EXT(2 == rowCount, setError(L10N::internalError("Wrong rows count: " + QString::number(rowCount))), );

    referenceGaps = msaObject->getRow(0).getGapModel();
    readGaps = msaObject->getRow(1).getGapModel();

    CHECK(offset > 0, );
    shiftGaps(referenceGaps);
    shiftGaps(readGaps);
    readGaps.prepend(U2MsaGap(0, offset));
}

bool PairwiseAlignmentTask::isReverse() const {
    return reverse;
}

bool PairwiseAlignmentTask::isComplement() const {
    return complement;
}

SharedDbiDataHandler PairwiseAlignmentTask::getRead() const {
    if (reverse && complement) {
        return rcKalign->getRead();
    } else if (reverse) {
        return rKalign->getRead();
    } else if (complement) {
        return cKalign->getRead();
    } else {
        return kalign->getRead();
    }
}

QList<U2MsaGap> PairwiseAlignmentTask::getReferenceGaps() const {
    return referenceGaps;
}

QList<U2MsaGap> PairwiseAlignmentTask::getReadGaps() const {
    return readGaps;
}

QString PairwiseAlignmentTask::getInitialReadName() const {
    return initialReadName;
}

QByteArray PairwiseAlignmentTask::getComplement(const QByteArray &sequence, const DNAAlphabet *alphabet) {
    DNATranslation *translator = AppContext::getDNATranslationRegistry()->lookupComplementTranslation(alphabet);
    CHECK_EXT(NULL != translator, setError(tr("Can't translate read sequence to reverse complement")), "");

    QByteArray translation(sequence.length(), 0);
    translator->translate(sequence.constData(), sequence.length(), translation.data(), translation.length());
    return translation;
}

QByteArray PairwiseAlignmentTask::getReverse(const QByteArray &sequence) const {
    QByteArray result = sequence;
    TextUtils::reverse(result.data(), result.length());
    return result;
}

QByteArray PairwiseAlignmentTask::getReverseComplement(const QByteArray &sequence, const DNAAlphabet *alphabet) {
    return getReverse(getComplement(sequence, alphabet));
}

void PairwiseAlignmentTask::createRcReads() {
    QScopedPointer<U2SequenceObject> readObject(StorageUtils::getSequenceObject(storage, read));
    CHECK_EXT(!readObject.isNull(), setError(L10N::nullPointerError("Read sequence")), );

    DNASequence seq = readObject->getWholeSequence();
    QByteArray sequence = seq.seq;
    initialReadName = seq.getName();

    seq.seq = getReverse(sequence);
    seq.setName(initialReadName + "_rev");
    U2EntityRef rRef = U2SequenceUtils::import(storage->getDbiRef(), seq, stateInfo);
    CHECK_OP(stateInfo, );
    rRead = storage->getDataHandler(rRef);

    seq.seq = getComplement(sequence, readObject->getAlphabet());
    seq.setName(initialReadName + "_compl");
    U2EntityRef cRef = U2SequenceUtils::import(storage->getDbiRef(), seq, stateInfo);
    CHECK_OP(stateInfo, );
    cRead = storage->getDataHandler(cRef);

    seq.seq = getReverseComplement(sequence, readObject->getAlphabet());
    seq.setName(initialReadName + "_rev_compl");
    U2EntityRef rcRef = U2SequenceUtils::import(storage->getDbiRef(), seq, stateInfo);
    CHECK_OP(stateInfo, );
    rcRead = storage->getDataHandler(rcRef);
}

KAlignSubTask * PairwiseAlignmentTask::initRc() {
    QList<qint64> values;
    values << kalign->getMaxRegionSize();
    values << rKalign->getMaxRegionSize();
    values << cKalign->getMaxRegionSize();
    values << rcKalign->getMaxRegionSize();
    qSort(values);
    qint64 max = values.last();

    if (kalign->getMaxRegionSize() == max) {
        reverse = false;
        complement = false;
    } else if (rKalign->getMaxRegionSize() == max) {
        reverse = true;
        complement = false;
    } else if (cKalign->getMaxRegionSize() == max) {
        reverse = false;
        complement = true;
    } else {
        assert(rcKalign->getMaxRegionSize() == max);
        reverse = true;
        complement = true;
    }

    if (reverse && complement) {
        return rcKalign;
    } else if (reverse) {
        return rKalign;
    } else if (complement) {
        return cKalign;
    } else {
        return kalign;
    }
}

void PairwiseAlignmentTask::createSWAlignment(KAlignSubTask *task) {
    QScopedPointer<U2SequenceObject> refObject(StorageUtils::getSequenceObject(storage, reference));
    CHECK_EXT(!refObject.isNull(), setError(L10N::nullPointerError("Reference sequence")), );
    QScopedPointer<U2SequenceObject> readObject(StorageUtils::getSequenceObject(storage, task->getRead()));
    CHECK_EXT(!readObject.isNull(), setError(L10N::nullPointerError("Read sequence")), );

    QByteArray referenceData = refObject->getSequenceData(task->getCoreRegion(), stateInfo);
    CHECK_OP(stateInfo, );

    MAlignment alignment("msa", refObject->getAlphabet());
    alignment.addRow(refObject->getSequenceName(), referenceData, stateInfo);
    CHECK_OP(stateInfo, );
    alignment.addRow(readObject->getSequenceName(), readObject->getWholeSequenceData(), stateInfo);
    CHECK_OP(stateInfo, );

    U2EntityRef msaRef = MAlignmentImporter::createAlignment(storage->getDbiRef(), alignment, stateInfo);
    CHECK_OP(stateInfo, );
    msa = storage->getDataHandler(msaRef);
    offset = task->getCoreRegion().startPos;
}

void PairwiseAlignmentTask::shiftGaps(QList<U2MsaGap> &gaps) const {
    for (int i=0; i<gaps.size(); i++) {
        gaps[i].offset += offset;
    }
}

} // LocalWorkflow
} // U2
