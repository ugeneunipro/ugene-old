#include "DbiTest.h"

#include <U2Core/U2DbiRegistry.h>
#include <U2Core/U2OpStatusUtils.h>
#include <U2Core/AppContext.h>
#include <U2Core/AppSettings.h>
#include <U2Core/U2SafePoints.h>

#include <QtCore/QDir>


namespace U2 {

TestDbiProvider::TestDbiProvider(){
    initialized = false;
    dbi = NULL;
}
TestDbiProvider::~TestDbiProvider(){
    close();
}
void TestDbiProvider::init(const QString& _dbUrl, bool _create, bool _useConnectionPool){
    if(initialized){
        close();
        initialized = false;
    }
    dbUrl = _dbUrl;
    useConnectionPool = _useConnectionPool;

    U2DbiFactory *factory = AppContext::getDbiRegistry()->getDbiFactoryById(SQLITE_DBI_ID);
    SAFE_POINT(factory!=NULL, "No dbi factory", );
    U2OpStatusImpl opStatus;

    if(useConnectionPool){
        U2DbiRef ref;
        ref.dbiFactoryId = factory->getId();
        ref.dbiId = dbUrl;
        dbi = AppContext::getDbiRegistry()->getGlobalDbiPool()->openDbi(ref, _create, opStatus);
        CHECK_OP(opStatus, );
    }else{
        dbi = factory->createDbi();
        SAFE_POINT(NULL != dbi, "dbi not created", );
        QHash<QString, QString> properties;
        if(_create){
           properties[U2_DBI_OPTION_CREATE] = U2_DBI_VALUE_ON;
        }
        properties["url"] = dbUrl;
        QVariantMap persistentData;
        dbi->init(properties, persistentData, opStatus);
        SAFE_POINT_OP(opStatus, );
    }

    initialized = true;
}
void TestDbiProvider::close(){
    U2OpStatusImpl opStatus;
    if(dbi){
        if(useConnectionPool){
            AppContext::getDbiRegistry()->getGlobalDbiPool()->releaseDbi(dbi, opStatus);
        }else{
            dbi->shutdown(opStatus);
            SAFE_POINT_OP(opStatus, );
            delete dbi;
        }
    }
    dbi = NULL;
    initialized = false;
}
U2Dbi* TestDbiProvider::getDbi(){
    SAFE_POINT(initialized, "Dbi Provider is not initialized", NULL);
    return dbi;
}

} //namespace
