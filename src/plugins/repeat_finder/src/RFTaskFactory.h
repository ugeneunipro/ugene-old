#ifndef _U2_REPEAT_FINDER_ALG_IMPL_H_
#define _U2_REPEAT_FINDER_ALG_IMPL_H_

#include <U2Algorithm/RepeatFinderTaskFactory.h>
#include <U2Algorithm/RepeatFinderSettings.h>
#include <U2Core/Task.h>

#include <U2Core/DNASequence.h>

namespace U2 {

class RevComplSequenceTask;

class RFTaskFactory: public RepeatFinderTaskFactory {
public:
    RFTaskFactory() {};

    virtual Task* getTaskInstance(const RepeatFinderSettings& config) const;
    virtual void setRFResultsListener(Task*, RFResultsListener*);
};

class ReverseAndCreateTask : public Task {
    Q_OBJECT
public:
    ReverseAndCreateTask(const RepeatFinderSettings& c);
    QList<Task*> onSubTaskFinished(Task* subTask);

private:
    RepeatFinderSettings c;
    RevComplSequenceTask *revTask;

};

} // namespace

#endif
