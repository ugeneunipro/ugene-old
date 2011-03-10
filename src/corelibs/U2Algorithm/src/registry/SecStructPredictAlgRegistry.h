#ifndef _U2_SEC_STRUCT_PREDICT_ALG_REGISTRY_H_
#define _U2_SEC_STRUCT_PREDICT_ALG_REGISTRY_H_

#include <QList>
#include <QString>
#include <QMutex>
#include <QObject>

#include <U2Core/global.h>

namespace U2 {

class SecStructPredictTaskFactory;
class SecStructPredictTask;

class U2ALGORITHM_EXPORT SecStructPredictAlgRegistry : public QObject {
    Q_OBJECT
public:
    SecStructPredictAlgRegistry(QObject* pOwn = 0);
    ~SecStructPredictAlgRegistry();
    bool registerAlgorithm(SecStructPredictTaskFactory* alg, const QString& algId);
    bool hadRegistered(const QString& algId);
    SecStructPredictTaskFactory* getAlgorithm(const QString& algId);
    QStringList getAlgNameList();
private:
    QMutex mutex;
    QMap<QString, SecStructPredictTaskFactory*> algMap;
    // Copy prohibition
    SecStructPredictAlgRegistry(const SecStructPredictAlgRegistry& m);
    SecStructPredictAlgRegistry& operator=(const SecStructPredictAlgRegistry& m);
};

} // namespace

#endif
