#ifndef _U2_DBI_REGISTRY_H_
#define _U2_DBI_REGISTRY_H_

#include <U2Core/U2Dbi.h>

#include <QtCore/QHash>
#include <QtCore/QMutex>

namespace U2 {

class U2DbiPool;

/** 
    Keep all DBI types registered in the system
*/
class U2CORE_EXPORT U2DbiRegistry : public QObject {
    Q_OBJECT
public:
    U2DbiRegistry(QObject *parent = NULL);
    ~U2DbiRegistry();

    virtual bool registerDbiFactory(U2DbiFactory *factory);

    virtual QList<U2DbiFactoryId> getRegisteredDbiFactories() const;

    virtual U2DbiFactory *getDbiFactoryById(U2DbiFactoryId id) const;

    U2DbiPool* getGlobalDbiPool() const {return pool;}
private:
    QHash<U2DbiFactoryId, U2DbiFactory *> factories;
    U2DbiPool* pool;
};

/** 
    Class to access to DBI connections
    Roles: 
        creates new DBIs on user request
        track connection live range (using ref counters)
        closes unused DBIs
*/
class U2CORE_EXPORT U2DbiPool : public QObject {
    Q_OBJECT
public:
    U2DbiPool(QObject* p = NULL);
    
    U2Dbi* openDbi(U2DbiFactoryId id, const QString& url, bool create, U2OpStatus& os);
    void releaseDbi(U2Dbi* dbi, U2OpStatus& os);

private:
    QHash<QString, U2Dbi*> dbiByUrl;
    QHash<QString, int> dbiCountersByUrl;
    QMutex lock;
};

} // namespace U2

#endif // _U2_DBI_REGISTRY_H_
