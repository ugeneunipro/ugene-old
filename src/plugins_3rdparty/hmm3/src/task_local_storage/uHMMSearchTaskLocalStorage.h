#ifndef _GB2_HMMER3_SEARCH_TLS_H_
#define _GB2_HMMER3_SEARCH_TLS_H_

#include <QtCore/QHash>
#include <QtCore/QMutex>
#include <QtCore/QThreadStorage>

#include "uHMMSearchTaskLocalData.h"

namespace U2 {

struct ContextId {
    ContextId( qint64 what ) : id( what ) {}
    qint64 id;
}; // ContextId

class UHMM3SearchTaskLocalStorage {
public:
    static const UHMM3SearchTaskLocalData* current();
    
    static UHMM3SearchTaskLocalData* createTaskContext( qint64 ctxId );
    
    static void freeTaskContext( qint64 ctxId );
    
private:
    static QHash< qint64, UHMM3SearchTaskLocalData* > data;
    static QThreadStorage< ContextId* > tls;
    static QMutex mutex;
    static const UHMM3SearchTaskLocalData defaultData;
    
}; // UHMMSearchTaskLocalStorage

} // U2

#endif // _GB2_HMMER3_SEARCH_TLS_H_
