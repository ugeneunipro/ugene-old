#include "SmithWatermanTaskFactoryRegistry.h"

#include <QMutexLocker>
#include <QStringList>


namespace U2 {

SmithWatermanTaskFactoryRegistry::SmithWatermanTaskFactoryRegistry(QObject* pOwn):
    QObject(pOwn)
{
}

SmithWatermanTaskFactoryRegistry::~SmithWatermanTaskFactoryRegistry()
{
    foreach (const SmithWatermanTaskFactory* factory, factories) {
        delete factory;
    }
}

bool SmithWatermanTaskFactoryRegistry::registerFactory(
                                                SmithWatermanTaskFactory* factory, 
                                                const QString& factoryId)
{
    QMutexLocker locker(&mutex);
    if (factories.contains(factoryId)) {
        return false;
    }
    factories[factoryId] = factory;
    return true;
}

SmithWatermanTaskFactory* SmithWatermanTaskFactoryRegistry::getFactory(const QString& factoryId) {
    return factories.value(factoryId, 0);
}

QStringList SmithWatermanTaskFactoryRegistry::getListFactoryNames() {
    return factories.keys();
}

bool SmithWatermanTaskFactoryRegistry::hadRegistered(const QString& factoryId) {
    return factories.contains(factoryId);
}

} // namespace
