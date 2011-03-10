#include <U2Core/U2DbiRegistry.h>
#include <U2Core/AppContext.h>
#include <U2Core/U2OpStatus.h>

namespace U2 {

U2DbiRegistry::U2DbiRegistry(QObject *parent) : QObject(parent) {
    pool = new U2DbiPool(this);
}

U2DbiRegistry::~U2DbiRegistry() {
    qDeleteAll(factories.values());
}

bool U2DbiRegistry::registerDbiFactory(U2DbiFactory *factory) {
    if(factories.contains(factory->getId())) {
        return false;
    }
    factories.insert(factory->getId(), factory);
    return true;
}


QList<U2DbiFactoryId> U2DbiRegistry::getRegisteredDbiFactories() const {
    return factories.keys();
}

U2DbiFactory *U2DbiRegistry::getDbiFactoryById(U2DbiFactoryId id) const {
    return factories.value(id, NULL);
}

//////////////////////////////////////////////////////////////////////////
// U2DbiPool

U2DbiPool::U2DbiPool(QObject* p) : QObject(p) {
}

U2Dbi* U2DbiPool::openDbi(U2DbiFactoryId id, const QString& url, bool create, U2OpStatus& os) {
    QMutexLocker m(&lock);

    U2Dbi* dbi = NULL;
    if (url.isEmpty()) {
        os.setError(tr("No URL provided!"));
        return NULL;
    }
    if (dbiByUrl.contains(url)) {
        dbi = dbiByUrl[url];
        int cnt = dbiCountersByUrl[url];
        dbiCountersByUrl[url] = cnt + 1;
    } else {
        U2DbiFactory* f = AppContext::getDbiRegistry()->getDbiFactoryById(id);
        if (f == NULL) {
            os.setError(tr("Invalid database type: %1").arg(id));
            return NULL;
        }
        dbi = f->createDbi();
        QHash<QString, QString> initProperties;
        initProperties[U2_DBI_OPTION_URL] = url;
        if (create) {
            initProperties[U2_DBI_OPTION_CREATE] = U2_DBI_VALUE_ON;
        }
        dbi->init(initProperties, QVariantMap(), os);
        if (os.hasError()) {
            delete dbi;
            return NULL;
        }
        dbiByUrl[url] = dbi;
        dbiCountersByUrl[url] = 1;
    }
    return dbi;
}

void U2DbiPool::releaseDbi(U2Dbi* dbi, U2OpStatus& os) {
    QMutexLocker m(&lock);
    QString url = dbi->getInitProperties().value(U2_DBI_OPTION_URL);
    if (!dbiByUrl.contains(url)) {    
        os.setError(tr("DbiPool: DBI not found! URL: %1").arg(url));
        return;
    }
    int cnt = dbiCountersByUrl[url];
    cnt--;
    if (cnt > 0) {
        dbiCountersByUrl[url] = cnt;
        return;
    }
    dbi->shutdown(os);
    delete dbi;

    dbiByUrl.remove(url);
    dbiCountersByUrl.remove(url);
}


} // namespace U2
