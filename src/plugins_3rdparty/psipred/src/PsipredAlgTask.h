#ifndef _U2_PSIPRED_ALG_TASK_H_
#define _U2_PSIPRED_ALG_TASK_H_

#include <QtCore/QMutex>
#include <U2Algorithm/SecStructPredictTask.h>


namespace U2 {

class PsipredAlgTask : public SecStructPredictTask {
    Q_OBJECT
public:
    PsipredAlgTask(const QByteArray& sequence);
    virtual void run();
    SEC_STRUCT_PREDICT_TASK_FACTORY(PsipredAlgTask)
private:
    static QMutex runLock;
};

} //namespace

#endif // _U2_PSIPRED_ALG_TASK_H_


