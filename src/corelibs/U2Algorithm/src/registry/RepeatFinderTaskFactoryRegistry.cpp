#include "RepeatFinderTaskFactoryRegistry.h"

#include <QMutexLocker>
#include <QStringList>


namespace U2 {

RepeatFinderTaskFactoryRegistry::RepeatFinderTaskFactoryRegistry(QObject* pOwn):
    QObject(pOwn)
{
}

RepeatFinderTaskFactoryRegistry::~RepeatFinderTaskFactoryRegistry()
{
    foreach (const RepeatFinderTaskFactory* factory, factories) {
        delete factory;
    }
}

bool RepeatFinderTaskFactoryRegistry::registerFactory(
                                                RepeatFinderTaskFactory* factory, 
                                                const QString& factoryId)
{
    QMutexLocker locker(&mutex);
    if (factories.contains(factoryId)) {
        return false;
    }
    factories[factoryId] = factory;
    return true;
}

RepeatFinderTaskFactory* RepeatFinderTaskFactoryRegistry::getFactory(const QString& factoryId) {
    return factories.value(factoryId, 0);
}

QStringList RepeatFinderTaskFactoryRegistry::getListFactoryNames() {
    return factories.keys();
}

bool RepeatFinderTaskFactoryRegistry::hadRegistered(const QString& factoryId) {
    return factories.contains(factoryId);
}

} // namespace
