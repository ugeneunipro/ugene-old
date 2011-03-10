#ifndef _IOADAPTER_REGISTRY_IMPL_H_
#define _IOADAPTER_REGISTRY_IMPL_H_

#include "private.h"
#include <U2Core/IOAdapter.h>

namespace U2 {

class U2PRIVATE_EXPORT IOAdapterRegistryImpl  : public IOAdapterRegistry {
public:
    IOAdapterRegistryImpl(QObject* p = NULL) : IOAdapterRegistry(p) {init();}
    
    virtual bool registerIOAdapter(IOAdapterFactory* io) ;

    virtual bool unregisterIOAdapter(IOAdapterFactory* io);

    virtual const QList<IOAdapterFactory*>& getRegisteredIOAdapters() const {return adapters;}

    virtual IOAdapterFactory* getIOAdapterFactoryById(IOAdapterId id) const;
    
private:
    void init();

    QList<IOAdapterFactory*> adapters;
};

}//namespace
#endif
