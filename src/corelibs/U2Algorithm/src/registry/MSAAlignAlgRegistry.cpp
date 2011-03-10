#include "MSAAlignAlgRegistry.h"

#include <U2Algorithm/MSAAlignTask.h>
#include <U2View/MSAAlignGUIExtension.h>

namespace U2 {

MSAAlignAlgorithmEnv::MSAAlignAlgorithmEnv(const QString& _id, MSAAlignTaskFactory* _tf, 
                                                 MSAAlignGUIExtensionsFactory* _g)
: id(_id), taskFactory(_tf), guiExtFactory(_g)
{
}

MSAAlignAlgorithmEnv::~MSAAlignAlgorithmEnv() {
    delete taskFactory;
    delete guiExtFactory;
}

MSAAlignAlgRegistry::MSAAlignAlgRegistry( QObject* pOwn /*= 0*/ ) : QObject(pOwn) {
}

MSAAlignAlgRegistry::~MSAAlignAlgRegistry() {
    foreach( MSAAlignAlgorithmEnv* algo, algorithms.values()) {
        delete algo;
    }
}

bool MSAAlignAlgRegistry::registerAlgorithm(MSAAlignAlgorithmEnv* algo) {
    QMutexLocker locker(&mutex);

    if (algorithms.contains(algo->getId())){
        return false;
    }
    algorithms.insert(algo->getId(), algo);
    return true;

}

MSAAlignAlgorithmEnv* MSAAlignAlgRegistry::unregisterAlgorithm(const QString& id) {
    QMutexLocker locker(&mutex);
    
    if (!algorithms.contains(id)) {
        return NULL;
    }
    MSAAlignAlgorithmEnv* res = algorithms.value(id);
    algorithms.remove(id);
    return res;
}

MSAAlignAlgorithmEnv* MSAAlignAlgRegistry::getAlgorithm( const QString& id) const {
    QMutexLocker locker(&mutex);
    return algorithms.value(id);
}


QStringList MSAAlignAlgRegistry::getRegisteredAlgorithmIds() const {
    return algorithms.keys();
}

} //namespace

