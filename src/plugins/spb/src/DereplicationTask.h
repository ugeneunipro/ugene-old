#ifndef _SPB_DEREPLICATION_TASK_H_
#define _SPB_DEREPLICATION_TASK_H_

#include <U2Core/Task.h>

#include <U2Lang/DbiDataHandler.h>
#include <U2Lang/DbiDataStorage.h>

using namespace U2;
using namespace U2::Workflow;

namespace SPB {

class DereplicationData {
public:
    DereplicationData(const QList<SharedDbiDataHandler> &srcSeqs,
        DbiDataStorage *storage,
        const QString &comparingAlgoId,
        double accuracy);

    QList<SharedDbiDataHandler> & getSeqs();
    DbiDataStorage * getStorage() const;
    QString getComparingAlgoId() const;
    double getAccuracy() const;

    SharedDbiDataHandler takeRandomSequence();

private:
    QList<SharedDbiDataHandler> srcSeqs;
    DbiDataStorage *storage;
    QString comparingAlgoId;
    double accuracy;
};

class DereplicationTask : public Task {
public:
    DereplicationTask(const QString &taskName, const DereplicationData &data);

    virtual void run() = 0;

    virtual void cleanup();

    QList<SharedDbiDataHandler> takeResult();

protected:
    DereplicationData data;

    QList<SharedDbiDataHandler> result;
};

class DereplicationTaskFactory {
public:
    static DereplicationTask *createTask(const QString &taskId, const DereplicationData &data);

    static const QString RANDOM;
};

} // SPB

#endif // _SPB_DEREPLICATION_TASK_H_
