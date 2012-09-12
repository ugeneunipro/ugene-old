#include "RandomDereplicationTask.h"

#include "DereplicationTask.h"

namespace SPB {

DereplicationData::DereplicationData(const QList<SharedDbiDataHandler> &_srcSeqs,
        DbiDataStorage *_storage,
        const QString &_comparingAlgoId,
        double _accuracy)
: srcSeqs(_srcSeqs), storage(_storage), comparingAlgoId(_comparingAlgoId),
accuracy(_accuracy)
{

}

QList<SharedDbiDataHandler> & DereplicationData::getSeqs() {
    return srcSeqs;
}

DbiDataStorage * DereplicationData::getStorage() const {
    return storage;
}

QString DereplicationData::getComparingAlgoId() const {
    return comparingAlgoId;
}

double DereplicationData::getAccuracy() const {
    return accuracy;
}

SharedDbiDataHandler DereplicationData::takeRandomSequence() {
    if (0 == srcSeqs.size()) {
        return SharedDbiDataHandler();
    }

    int num = qrand() % srcSeqs.size();
    return srcSeqs.takeAt(num);
}

DereplicationTask::DereplicationTask(const QString &taskName, const DereplicationData &_data)
: Task(taskName, TaskFlag_None), data(_data)
{

}

void DereplicationTask::cleanup() {
    result.clear();
}

QList<SharedDbiDataHandler> DereplicationTask::takeResult() {
    QList<SharedDbiDataHandler> ret = result;
    result.clear();

    return ret;
}

DereplicationTask * DereplicationTaskFactory::createTask(const QString &taskId, const DereplicationData &data) {
    if (RANDOM == taskId) {
        return new RandomDereplicationTask(data);
    }
    return NULL;
}

const QString DereplicationTaskFactory::RANDOM("Random");

} //  SPB
