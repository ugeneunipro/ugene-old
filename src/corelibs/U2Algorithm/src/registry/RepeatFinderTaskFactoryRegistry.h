#ifndef _U2_REPEAT_FINDER_ALG_REGISTRY_H_
#define _U2_REPEAT_FINDER_ALG_REGISTRY_H_

#include <QHash>
#include <QString>
#include <QMutex>
#include <QObject>

#include <U2Core/global.h>
#include <U2Algorithm/RepeatFinderTaskFactory.h>


namespace U2 {

enum RepeatFinderTaskFactoryId {};

class U2ALGORITHM_EXPORT RepeatFinderTaskFactoryRegistry: public QObject {
    Q_OBJECT
public:
    RepeatFinderTaskFactoryRegistry(QObject* pOwn = 0);
    ~RepeatFinderTaskFactoryRegistry();

    bool registerFactory(RepeatFinderTaskFactory* factory, const QString& factoryId);
    bool hadRegistered(const QString& factoryId);
    RepeatFinderTaskFactory* getFactory(const QString& factoryId);
    QStringList getListFactoryNames();

private:
    QMutex mutex;
    QHash<QString, RepeatFinderTaskFactory*> factories;
};

} // namespace

#endif
