#ifndef _DB_REQUEST_FACTORY_
#define _DB_REQUEST_FACTORY_

#include <U2Core/DataBaseRegistry.h>

#include "HttpRequest.h"

namespace U2 {

class BLASTFactory: public DataBaseFactory {
    HttpRequest* getRequest(Task *t) {return new HttpRequestBLAST(t);}
};

class CDDFactory: public DataBaseFactory {
    HttpRequest* getRequest(Task* t) {return new HttpRequestCDD(t);}
};

}


#endif