#ifndef _U2_SMITH_WATERMAN_ALG_REGISTRY_H_
#define _U2_SMITH_WATERMAN_ALG_REGISTRY_H_

#include <QHash>
#include <QString>
#include <QMutex>
#include <QObject>

#include <U2Core/global.h>
#include <U2Algorithm/SmithWatermanTaskFactory.h>


namespace U2 {

class U2ALGORITHM_EXPORT SmithWatermanTaskFactoryRegistry: public QObject {
    Q_OBJECT
public:
    SmithWatermanTaskFactoryRegistry(QObject* pOwn = 0);
    ~SmithWatermanTaskFactoryRegistry();

    bool registerFactory(SmithWatermanTaskFactory* factory, const QString& factoryId);
    bool hadRegistered(const QString& factoryId);
    SmithWatermanTaskFactory* getFactory(const QString& factoryId);
    QStringList getListFactoryNames();

private:
    QMutex mutex;
    QHash<QString, SmithWatermanTaskFactory*> factories;
};

} // namespace

#endif
