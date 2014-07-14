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

#ifndef _U2_EXTRACTMSACONSENSUSWORKER_H_
#define _U2_EXTRACTMSACONSENSUSWORKER_H_

#include <U2Lang/LocalDomain.h>
#include <U2Lang/WorkflowUtils.h>

namespace U2 {
    class MSAConsensusAlgorithm;
    class ExportConsensusTask;
namespace LocalWorkflow {
using namespace Workflow;

class ExtractMSAConsensusWorker : public BaseWorker {
    Q_OBJECT
public:
    ExtractMSAConsensusWorker(Actor *actor);

    void init();
    Task * tick();
    void cleanup();

private slots:
    void sl_taskFinished();

private:
    bool hasMsa() const;
    MAlignment takeMsa(U2OpStatus &os);
    Task * createTask(const MAlignment &msa);
    void finish();
    void sendResult(const SharedDbiDataHandler &seqId);
};

class ExtractMSAConsensusTaskHelper : public Task {
public:
    ExtractMSAConsensusTaskHelper(const QString &algoId, int threshold, bool keepGaps, const MAlignment &msa, const U2DbiRef &targetDbi);


    void prepare();
    U2EntityRef getResult() const;

private:
    const QString algoId;
    const int threshold;
    const bool keepGaps;
    MAlignment msa;
    const U2DbiRef targetDbi;
    U2Sequence resultSequence;

private:
    MSAConsensusAlgorithm * createAlgorithm();
    QString getResultName () const;
};

class ExtractMSAConsensusWorkerFactory : public DomainFactory {
public:
    ExtractMSAConsensusWorkerFactory();

    Worker * createWorker(Actor *actor);

    static void init();

    static const QString ACTOR_ID;
};

class ExtractMSAConsensusWorkerPrompter : public PrompterBase<ExtractMSAConsensusWorkerPrompter>{
    Q_OBJECT
public:
    ExtractMSAConsensusWorkerPrompter(Actor *actor = NULL);

protected:
    QString composeRichDoc();
};

} // LocalWorkflow
} // U2

#endif // _U2_EXTRACTMSACONSENSUSWORKER_H_
