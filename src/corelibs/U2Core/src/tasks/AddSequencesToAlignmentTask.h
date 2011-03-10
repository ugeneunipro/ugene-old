#ifndef _U2_ADD_SEQUENCES_TO_ALIGNMENT_TASK_H_
#define _U2_ADD_SEQUENCES_TO_ALIGNMENT_TASK_H_

#include <U2Core/Task.h>
#include <U2Core/MAlignmentObject.h>

#include <QtCore/QPointer>


namespace U2 {

class LoadDocumentTask;

class U2CORE_EXPORT AddSequencesToAlignmentTask : public Task {
    Q_OBJECT
public:
    AddSequencesToAlignmentTask(MAlignmentObject* obj, const QString& fileWithSequences);

    QList<Task*> onSubTaskFinished(Task* subTask);

    ReportResult report();

    QPointer<MAlignmentObject>  maObj;
    LoadDocumentTask*           loadTask;
};

}// namespace

#endif //_U2_ADD_SEQUENCES_TO_ALIGNMENT_TASK_H_
