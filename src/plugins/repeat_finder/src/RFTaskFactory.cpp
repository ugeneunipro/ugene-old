#include "RFTaskFactory.h"
#include "RFBase.h"
#include "FindRepeatsTask.h"

namespace U2 {

Task* RFTaskFactory::getTaskInstance( const RepeatFinderSettings& c) const {

    if (c.inverted) {
        return new ReverseAndCreateTask(c);
/*
        result = new Task("Make reverse sequence and find repeats", TaskFlag_NoRun);

        RevComplSequenceTask *revTask = new RevComplSequenceTask(DNASequence(QByteArray(c.seqX), c.al), U2Region(0, c.sizeX));
        result->addSubTask(revTask);

        Task *rfBase = RFAlgorithmBase::createTask(
            c.l,
            revTask->complementSequence.constData(),
            c.sizeX,
            c.seqY,
            c.sizeY,
            c.al,
            c.w,
            c.mismatches,
            c.alg,
            c.nThreads
        );
        result->addSubTask(rfBase);
        */
    }
    else {
        return RFAlgorithmBase::createTask(
            c.l,
            c.seqX,
            c.sizeX,
            c.seqY,
            c.sizeY,
            c.al,
            c.w,
            c.mismatches,
            c.alg,
            c.nThreads
        );
    }
}

void RFTaskFactory::setRFResultsListener(Task* b, RFResultsListener* l) {
    RFAlgorithmBase* rfTask = qobject_cast<RFAlgorithmBase*>(b);
    if (rfTask) {
        rfTask->setRFResultsListener(l);
    }
}

ReverseAndCreateTask::ReverseAndCreateTask(const RepeatFinderSettings& c)
: Task("Make reverse sequence and find repeats", TaskFlag_NoRun)
{
    this->c = c;

    revTask = new RevComplSequenceTask(DNASequence(QByteArray(c.seqX), c.al), U2Region(0, c.sizeX));
    addSubTask(revTask);
}

QList<Task*> ReverseAndCreateTask::onSubTaskFinished(Task* subTask) {

    QList<Task*> subTasks;

    if (revTask == subTask) {

        Task *rfBase = RFAlgorithmBase::createTask(
            c.l,
            revTask->complementSequence.constData(),
            c.sizeX,
            c.seqY,
            c.sizeY,
            c.al,
            c.w,
            c.mismatches,
            c.alg,
            c.nThreads
        );
        subTasks.append(rfBase);
    }

    return subTasks;
}

} // namespace
