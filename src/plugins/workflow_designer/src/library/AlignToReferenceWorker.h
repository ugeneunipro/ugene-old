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

    qint64 getMaxChunkSize() const;
    QList<U2MsaGap> getReferenceGaps() const;
    QList<U2MsaGap> getReadGaps() const;

private:
    PairwiseAlignmentTaskFactory * getPairwiseAlignmentTaskFactory(U2OpStatus &os) const;
    void createAlignment(U2SequenceObject *refObject, U2SequenceObject *readObject);
    PairwiseAlignmentTaskSettings * createSettings();

private:
    const SharedDbiDataHandler reference;
    const SharedDbiDataHandler read;
    DbiDataStorage *storage;
    SharedDbiDataHandler msa;
    qint64 maxChunkSize;
    QList<U2MsaGap> referenceGaps;
    QList<U2MsaGap> readGaps;
};

class AlignToReferenceTask : public Task {
public:
    AlignToReferenceTask(const SharedDbiDataHandler &reference, const QList<SharedDbiDataHandler> &reads, DbiDataStorage *storage);
    void prepare();
    void run();
    SharedDbiDataHandler getResult() const;

private:
    QByteArray getReverseComplement(const QByteArray &sequence, const DNAAlphabet *alphabet);
    SharedDbiDataHandler createRcRead(const SharedDbiDataHandler &read);
    KAlignSubTask * getBestSubTask(int readNum, bool &rc);
    DNASequence getReadSequence(int readNum, bool rc);
    DNASequence getReferenceSequence();

    QList<U2MsaGap> getReferenceGaps();
    QList<U2MsaGap> getShiftedGaps(int rowNum);
    void insertShiftedGapsIntoReference(MAlignment &alignment, const QList<U2MsaGap> &gaps);
    void insertShiftedGapsIntoRead(MAlignment &alignment, int readNum, const QList<U2MsaGap> &gaps);

private:
    const SharedDbiDataHandler reference;
    const QList<SharedDbiDataHandler> reads;
    QList<SharedDbiDataHandler> rcReads;
    QList<KAlignSubTask*> subTasks;
    QList<KAlignSubTask*> rcSubTasks;
    DbiDataStorage *storage;
    SharedDbiDataHandler msa;
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
