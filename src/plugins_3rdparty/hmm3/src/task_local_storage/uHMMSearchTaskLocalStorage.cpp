#include <cassert>

#include <QtCore/QMutexLocker>

#include "uHMMSearchTaskLocalStorage.h"

namespace U2 {

QHash< qint64, UHMM3SearchTaskLocalData* > UHMM3SearchTaskLocalStorage::data;
QThreadStorage< ContextId* > UHMM3SearchTaskLocalStorage::tls;
QMutex UHMM3SearchTaskLocalStorage::mutex;
const UHMM3SearchTaskLocalData UHMM3SearchTaskLocalStorage::defaultData;

const UHMM3SearchTaskLocalData* UHMM3SearchTaskLocalStorage::current() {
    ContextId* idc = tls.localData();
    if( NULL != idc ) {
        QMutexLocker locker( &mutex );
        UHMM3SearchTaskLocalData* res = data.value( idc->id );
        assert( NULL != res );
        return res;
    } else {
        return &defaultData;
    }
}

UHMM3SearchTaskLocalData* UHMM3SearchTaskLocalStorage::createTaskContext( qint64 ctxId ) {
    QMutexLocker locker( &mutex );
    assert( !data.contains( ctxId ) );
    UHMM3SearchTaskLocalData* ctx = new UHMM3SearchTaskLocalData();
    data[ctxId] = ctx;
    
    assert(!tls.hasLocalData());
    ContextId* idc = new ContextId( ctxId );
    tls.setLocalData( idc );
    
    return ctx;
}

void UHMM3SearchTaskLocalStorage::freeTaskContext( qint64 ctxId ) {
    QMutexLocker locker( &mutex );
    
    UHMM3SearchTaskLocalData* lData = data.value( ctxId );
    assert( NULL != lData );
    
    int howMany = data.remove( ctxId );
    Q_UNUSED( howMany );
    assert( 1 == howMany );
    
    delete lData;
}

} // U2
