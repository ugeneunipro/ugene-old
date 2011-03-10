#include "DnaAssemblyAlgRegistry.h"

#include <U2Algorithm/DnaAssemblyTask.h>
#include <U2View/DnaAssemblyGUIExtension.h>

namespace U2 {

DnaAssemblyAlgorithmEnv::DnaAssemblyAlgorithmEnv(const QString& _id, DnaAssemblyToRefTaskFactory* _tf, 
                                                 DnaAssemblyGUIExtensionsFactory* _g, bool indexSupported)
: id(_id), taskFactory(_tf), guiExtFactory(_g), supportsIndexFiles(indexSupported)
{
}

DnaAssemblyAlgorithmEnv::~DnaAssemblyAlgorithmEnv() {
    delete taskFactory;
    delete guiExtFactory;
}

DnaAssemblyAlgRegistry::DnaAssemblyAlgRegistry( QObject* pOwn /*= 0*/ ) : QObject(pOwn) {
}

DnaAssemblyAlgRegistry::~DnaAssemblyAlgRegistry() {
    foreach( DnaAssemblyAlgorithmEnv* algo, algorithms.values()) {
        delete algo;
    }
}

bool DnaAssemblyAlgRegistry::registerAlgorithm(DnaAssemblyAlgorithmEnv* algo) {
    QMutexLocker locker(&mutex);

    if (algorithms.contains(algo->getId())){
        return false;
    }
    algorithms.insert(algo->getId(), algo);
    return true;

}

DnaAssemblyAlgorithmEnv* DnaAssemblyAlgRegistry::unregisterAlgorithm(const QString& id) {
    QMutexLocker locker(&mutex);
    
    if (!algorithms.contains(id)) {
        return NULL;
    }
    DnaAssemblyAlgorithmEnv* res = algorithms.value(id);
    algorithms.remove(id);
    return res;
}

DnaAssemblyAlgorithmEnv* DnaAssemblyAlgRegistry::getAlgorithm( const QString& id) const {
    QMutexLocker locker(&mutex);
    return algorithms.value(id);
}


QStringList DnaAssemblyAlgRegistry::getRegisteredAlgorithmIds() const {
    return algorithms.keys();
}

QStringList DnaAssemblyAlgRegistry::getRegisteredAlgorithmsWithIndexFileSupport() const {
    QStringList result;
    foreach( DnaAssemblyAlgorithmEnv* algo, algorithms.values()) {
        if (algo->isIndexFilesSupported()) {
            result << algo->getId();
        }
    }
    return result;
}

} //namespace

