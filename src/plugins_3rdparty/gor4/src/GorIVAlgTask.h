#ifndef _U2_GORIV_ALG_TASK_H_
#define _U2_GORIV_ALG_TASK_H_

#include <QtCore/QMutex>
#include <U2Algorithm/SecStructPredictTask.h>


namespace U2 {

class GorIVAlgTask : public SecStructPredictTask {
    Q_OBJECT
public:
    GorIVAlgTask(const QByteArray& sequence);
    virtual void run();
    SEC_STRUCT_PREDICT_TASK_FACTORY(GorIVAlgTask)
private:
    static QMutex runLock;
};

} //namespace

#endif // _U2_GORIV_ALG_TASK_H_


