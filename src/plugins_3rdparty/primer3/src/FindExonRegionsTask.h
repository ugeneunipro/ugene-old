#ifndef _FIND_EXON_REGIONS_TASK_H_
#define _FIND_EXON_REGIONS_TASK_H_

#include <U2Core/U2Region.h>
#include <U2Core/Task.h>

namespace U2 {

class LoadRemoteDocumentTask;
class SplicedAlignmentTask;
class U2SequenceObject;

class FindExonRegionsTask : public Task
{
    Q_OBJECT
public:
    FindExonRegionsTask(U2SequenceObject* dnaObj, const QString& rnaSeqId);
    QList<U2Region> getRegions() { return exonRegions; }

    void prepare();
    QList<Task*> onSubTaskFinished(Task *subTask);
    ReportResult report();

    void cleanup();

private:
    QList<U2Region> exonRegions;
    U2SequenceObject* dnaObj;
    LoadRemoteDocumentTask* loadDocumentTask;
    SplicedAlignmentTask* alignmentTask;

};

}

#endif // _FIND_EXON_REGIONS_TASK_H_
