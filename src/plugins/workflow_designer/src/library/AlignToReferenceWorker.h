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

#ifndef _U2_ALIGN_TO_REFERENCE_WORKER_H_
#define _U2_ALIGN_TO_REFERENCE_WORKER_H_

#include <U2Lang/BaseDatasetWorker.h>
#include <U2Lang/WorkflowUtils.h>

namespace U2 {
class PairwiseAlignmentTaskFactory;
class PairwiseAlignmentTaskSettings;
namespace LocalWorkflow {

class KAlignSubTask : public Task {
public:
    KAlignSubTask(const SharedDbiDataHandler &reference, const SharedDbiDataHandler &read, DbiDataStorage *storage);
    void prepare();
    void run();

    const SharedDbiDataHandler getRead() const;
    qint64 getMaxRegionSize() const;
    U2Region getCoreRegion() const;

    static PairwiseAlignmentTaskSettings * createSettings(DbiDataStorage *storage, const SharedDbiDataHandler &msa, U2OpStatus &os);
    static PairwiseAlignmentTaskFactory * getPairwiseAlignmentTaskFactory(const QString &algoId, const QString &implId, U2OpStatus &os);

private:
    void createAlignment();
    QList<U2Region> getRegions(const QList<U2MsaGap> &gaps, qint64 rowLength) const;
    void calculateCoreRegion(const QList<U2Region> &regions);
    void extendCoreRegion(const QList<U2Region> &regions);

private:
    const SharedDbiDataHandler reference;
    const SharedDbiDataHandler read;
    DbiDataStorage *storage;
    SharedDbiDataHandler msa;

    qint64 maxRegionSize;
    U2Region coreRegion;

    static const qint64 MAX_GAP_SIZE;
    static const qint64 EXTENSION_COEF;
};

class PairwiseAlignmentTask : public Task {
public:
    PairwiseAlignmentTask(const SharedDbiDataHandler &reference, const SharedDbiDataHandler &read, DbiDataStorage *storage);
    void prepare();
    QList<Task*> onSubTaskFinished(Task *subTask);
    void run();

    bool isReverse() const;
    bool isComplement() const;
    SharedDbiDataHandler getRead() const;
    QList<U2MsaGap> getReferenceGaps() const;
    QList<U2MsaGap> getReadGaps() const;
    QString getInitialReadName() const;

private:
    QByteArray getComplement(const QByteArray &sequence, const DNAAlphabet *alphabet);
    QByteArray getReverse(const QByteArray &sequence) const;
    QByteArray getReverseComplement(const QByteArray &sequence, const DNAAlphabet *alphabet);
    void createRcReads();
    KAlignSubTask * initRc();
    void createSWAlignment(KAlignSubTask *task);
    void shiftGaps(QList<U2MsaGap> &gaps) const;

private:
    const SharedDbiDataHandler reference;
    const SharedDbiDataHandler read;
    SharedDbiDataHandler rRead;
    SharedDbiDataHandler cRead;
    SharedDbiDataHandler rcRead;
    DbiDataStorage *storage;

    KAlignSubTask *kalign;
    KAlignSubTask *rKalign;
    KAlignSubTask *cKalign;
    KAlignSubTask *rcKalign;
    bool reverse;
    bool complement;
    qint64 offset;

    SharedDbiDataHandler msa;
    qint64 maxChunkSize;
    QList<U2MsaGap> referenceGaps;
    QList<U2MsaGap> readGaps;
    QString initialReadName;
};

class ComposeResultSubTask : public Task {
public:
    ComposeResultSubTask(const SharedDbiDataHandler &reference, const QList<SharedDbiDataHandler> &reads, const QList<PairwiseAlignmentTask*> subTasks, DbiDataStorage *storage);
    void prepare();
    void run();
    SharedDbiDataHandler getAlignment() const;
    SharedDbiDataHandler getAnnotations() const;

private:
    PairwiseAlignmentTask * getPATask(int readNum);
    DNASequence getReadSequence(int readNum);
    DNASequence getReferenceSequence();
    QList<U2MsaGap> getReferenceGaps();
    QList<U2MsaGap> getShiftedGaps(int rowNum);
    void insertShiftedGapsIntoReference(MAlignment &alignment, const QList<U2MsaGap> &gaps);
    void insertShiftedGapsIntoRead(MAlignment &alignment, int readNum, const QList<U2MsaGap> &gaps);
    MAlignment createAlignment();
    void createAnnotations(const MAlignment &alignment);
    U2Region getReadRegion(const MAlignmentRow &readRow, const MAlignmentRow &referenceRow) const;
    U2Location getLocation(const U2Region &region, bool isComplement, qint64 referenceLength);

private:
    const SharedDbiDataHandler reference;
    const QList<SharedDbiDataHandler> reads;
    const QList<PairwiseAlignmentTask*> subTasks;
    DbiDataStorage *storage;
    SharedDbiDataHandler msa;
    SharedDbiDataHandler annotations;
};

class AlignToReferenceTask : public Task {
public:
    AlignToReferenceTask(const SharedDbiDataHandler &reference, const QList<SharedDbiDataHandler> &reads, DbiDataStorage *storage);
    void prepare();
    QList<Task*> onSubTaskFinished(Task *subTask);
    SharedDbiDataHandler getAlignment() const;
    SharedDbiDataHandler getAnnotations() const;

private:
    const SharedDbiDataHandler reference;
    const QList<SharedDbiDataHandler> reads;
    QList<PairwiseAlignmentTask*> subTasks;
    ComposeResultSubTask *composeSubTask;
    DbiDataStorage *storage;
    SharedDbiDataHandler msa;
    SharedDbiDataHandler annotations;
    int subTasksCount;
};

class AlignToReferencePrompter : public PrompterBase<AlignToReferencePrompter> {
    Q_OBJECT
public:
    AlignToReferencePrompter(Actor *a);

protected:
    QString composeRichDoc();
};

class AlignToReferenceWorker : public BaseDatasetWorker {
    Q_OBJECT
public:
    AlignToReferenceWorker(Actor *a);

    void cleanup();

protected:
    Task * createPrepareTask(U2OpStatus &os) const;
    void onPrepared(Task *task, U2OpStatus &os);
    Task * createTask(const QList<Message> &messages) const;
    QVariantMap getResult(Task *task, U2OpStatus &os) const;
    MessageMetadata generateMetadata(const QString &datasetName) const;

private:
    Document *referenceDoc;
    SharedDbiDataHandler reference;
};

class AlignToReferenceWorkerFactory : public DomainFactory {
public:
    AlignToReferenceWorkerFactory();
    Worker * createWorker(Actor *a);

    static const QString ACTOR_ID;
    static void init();
};

} // LocalWorkflow
} // U2

#endif // _U2_ALIGN_TO_REFERENCE_WORKER_H_
