#ifndef _U2_SEC_STRUCT_PREDICT_TASK_H_
#define _U2_SEC_STRUCT_PREDICT_TASK_H_

#include <U2Core/Task.h>
#include <U2Core/AnnotationData.h>

namespace U2 {

class U2ALGORITHM_EXPORT SecStructPredictTask : public Task {
    Q_OBJECT
public:
    SecStructPredictTask(const QByteArray& seq);
    const QList<SharedAnnotationData>& getResults() const { return results; }
    const QByteArray getSSFormatResults() const { return output; }
    
protected:
    QByteArray sequence, output;
    QList<SharedAnnotationData> results;
};


class U2ALGORITHM_EXPORT SecStructPredictTaskFactory {
public:
    virtual SecStructPredictTask* createTaskInstance(const QByteArray& inputSeq) = 0;
    virtual ~SecStructPredictTaskFactory() {}
};

#define SEC_STRUCT_PREDICT_TASK_FACTORY(c) \
public: \
    static const QString taskName; \
    class Factory : public SecStructPredictTaskFactory { \
    public: \
        Factory() { } \
        SecStructPredictTask* createTaskInstance(const QByteArray& inputSeq) { return new c(inputSeq); } \
    };


} //namespace

#endif // _U2_SEC_STRUCT_PREDICT_TASK_H_
