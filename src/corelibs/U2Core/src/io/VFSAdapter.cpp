
#include <U2Core/AppContext.h>

#include "VFSAdapter.h"

namespace U2 {

VFSAdapterFactory::VFSAdapterFactory(QObject* o) : IOAdapterFactory(o) {
    name = tr("Memory buffer");
}

IOAdapter* VFSAdapterFactory::createIOAdapter() {
    return new VFSAdapter(this);
}

VFSAdapter::VFSAdapter(VFSAdapterFactory* factory, QObject* o) : IOAdapter(factory, o), url("", GUrl_VFSFile), buffer(NULL) {
}


bool VFSAdapter::open(const GUrl& _url, IOAdapterMode m) {
    assert(!isOpen());
    assert(buffer == NULL);
    
    // assume that all membuf adapters work with files in some vfs
    if( !_url.getURLString().startsWith( VirtualFileSystem::URL_PREFIX ) ) {
        return false; // not a file in vfs
    }
    VirtualFileSystemRegistry * vfsReg = AppContext::getVirtualFileSystemRegistry();
    assert( NULL != vfsReg );
    QStringList urlArgs = _url.getURLString().mid( VirtualFileSystem::URL_PREFIX.size() ).
        split( VirtualFileSystem::URL_NAME_SEPARATOR, QString::SkipEmptyParts );
    if( 2 != urlArgs.size() ) { // urlArgs - vfsname and filename
        return false;
    }
    VirtualFileSystem * vfs = vfsReg->getFileSystemById( urlArgs[0] );
    if( NULL == vfs ) {
        return false; // no such vfs registered
    }
    
    if( !vfs->fileExists( urlArgs[1] ) ) {
        if( IOAdapterMode_Read == m ) {
            return false;
        } else {
            vfs->createFile( urlArgs[1], QByteArray() );
        }
    }
    
    buffer = new QBuffer( &vfs->getFileByName( urlArgs[1] ) );
    QIODevice::OpenMode iomode = m == IOAdapterMode_Read ? QIODevice::ReadOnly : QIODevice::WriteOnly | QIODevice::Truncate;
    if (!buffer->open(iomode)) {
        return false;
    }
    url = _url;
    return true;
}

void VFSAdapter::close() {
    assert(isOpen());
    if (!isOpen()) {
        return;
    }
    delete buffer;
    buffer = NULL;
    url = GUrl("", GUrl_VFSFile);
}

qint64 VFSAdapter::readBlock(char* data, qint64 size) {
    qint64 l = buffer->read(data, size);
    return l;
}

qint64 VFSAdapter::writeBlock(const char* data, qint64 size) {
    qint64 l = buffer->write(data, size);
    return l;
}

bool VFSAdapter::skip(qint64 nBytes) {
    assert(isOpen());
    if (!isOpen()) {
        return false;
    }
    qint64 p = buffer->pos();
    return buffer->seek(p+nBytes);
}

qint64 VFSAdapter::left() const {
    assert(isOpen());
    qint64 p = buffer->pos();
    qint64 len = buffer->size();
    return len - p;
}

int VFSAdapter::getProgress() const {
    assert(isOpen());
    return int(100 * float(buffer->pos()) / buffer->size());
}

qint64 VFSAdapter::bytesRead() const {
    return buffer->pos();
}


};//namespace
