#include <U2Algorithm/SecStructPredictTask.h>
#include "SecStructPredictAlgRegistry.h"
#include <QtCore/QStringList>

namespace U2 {

SecStructPredictAlgRegistry::SecStructPredictAlgRegistry( QObject* pOwn /*= 0*/ ) : QObject(pOwn)
{
   

}

SecStructPredictAlgRegistry::~SecStructPredictAlgRegistry()
{
    foreach( SecStructPredictTaskFactory* factory, algMap.values()) {
        delete factory;
    }
}

bool SecStructPredictAlgRegistry::registerAlgorithm( SecStructPredictTaskFactory* alg, const QString& algId )
{
    QMutexLocker locker(&mutex);

    if (algMap.contains(algId)){
        return false;
    }
    algMap.insert(algId, alg);
    return true;

}

bool SecStructPredictAlgRegistry::hadRegistered( const QString& algId )
{
    return algMap.contains(algId);
}

SecStructPredictTaskFactory* SecStructPredictAlgRegistry::getAlgorithm( const QString& algId )
{
    if (algMap.contains(algId)) {
        return algMap.value(algId);
    } else {
        return NULL;
    }
}

QStringList SecStructPredictAlgRegistry::getAlgNameList()
{
    return algMap.keys();
}

    
}


