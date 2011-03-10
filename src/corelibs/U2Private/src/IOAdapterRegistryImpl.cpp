#include "IOAdapterRegistryImpl.h"

#include <U2Core/LocalFileAdapter.h>
#include <U2Core/HttpFileAdapter.h>
#include <U2Core/VFSAdapter.h>

namespace U2 {

bool IOAdapterRegistryImpl::registerIOAdapter(IOAdapterFactory* io) {
    if (getIOAdapterFactoryById(io->getAdapterId())!=NULL) {
        return false;
    }
    adapters.push_back(io);
    return true;
}

bool IOAdapterRegistryImpl::unregisterIOAdapter(IOAdapterFactory* io)  {
    int n = adapters.removeAll(io);
    return n > 0;
}


IOAdapterFactory* IOAdapterRegistryImpl::getIOAdapterFactoryById(IOAdapterId id) const  {
    foreach (IOAdapterFactory* io, adapters) {
        if (io->getAdapterId() == id) {
            return io;
        }
    }
    return NULL;
}


void IOAdapterRegistryImpl::init() {
    registerIOAdapter(new LocalFileAdapterFactory(this));
    registerIOAdapter(new GzippedLocalFileAdapterFactory(this));
    registerIOAdapter( new HttpFileAdapterFactory(this) );
    registerIOAdapter( new GzippedHttpFileAdapterFactory(this) );
    registerIOAdapter( new VFSAdapterFactory(this) );
}

}//namespace
