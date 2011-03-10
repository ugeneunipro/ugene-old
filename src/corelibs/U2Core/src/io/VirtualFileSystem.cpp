
#include <cassert>
#include <QtCore/QFile>

#include <U2Core/AppContext.h>
#include <U2Core/IOAdapter.h>

#include "VirtualFileSystem.h"

namespace U2 {

/*******************************************
 * VirtualFileSystem
 *******************************************/

const QString VirtualFileSystem::URL_PREFIX = "VFS";
const QString VirtualFileSystem::URL_NAME_SEPARATOR = "!@#$";

VirtualFileSystem::VirtualFileSystem() {
}

VirtualFileSystem::VirtualFileSystem( const QString & fsName ) : fileSystemName( fsName ) {
}

VirtualFileSystem::~VirtualFileSystem() {
}

bool VirtualFileSystem::createFile( const QString & filename, const QByteArray & data ) {
    if( files.contains( filename ) ) {
        return false;
    }
    files[filename] = data;
    return true;
}

bool VirtualFileSystem::mapFile( const QString & filename, const QString & filePath ) {
    IOAdapterFactory * iof = AppContext::getIOAdapterRegistry()->getIOAdapterFactoryById( BaseIOAdapters::url2io( filePath ) );
    assert( NULL != iof );
    IOAdapter * io( iof->createIOAdapter() );
    if( NULL == io ) {
        return false;
    }
    if( !io->open( filePath, IOAdapterMode_Read ) ) {
        delete io;
        return false;
    }
    
    QByteArray bytes;
    while( !io->isEof() ) {
        QByteArray bytesBlock( READ_BLOCK_SZ, '\0' );
        qint64 howMany = io->readBlock( bytesBlock.data(), READ_BLOCK_SZ );
        if( -1 == howMany ) {
            io->close();
            delete io;
            return false;
        } else if( 0 == howMany) {
            assert( io->isEof() );
            continue;
        }
        bytes.append( QByteArray( bytesBlock.data(), howMany ) );
    }
    
    modifyFile( filename, bytes );
    io->close();
    delete io;
    return true;
}

bool VirtualFileSystem::mapBack( const QString & filename, const QString & filePath ) const {
    if( !files.contains( filename ) ) {
        return false;
    }
    
    IOAdapterFactory * iof = AppContext::getIOAdapterRegistry()->getIOAdapterFactoryById( BaseIOAdapters::url2io( filePath ) );
    assert( NULL != iof );
    IOAdapter * io( iof->createIOAdapter() );
    if( NULL == io ) {
        return false;
    }
    if( !io->open( filePath, IOAdapterMode_Write ) ) {
        delete io;
        return false;
    }
    
    io->writeBlock( files[filename] );
    io->close();
    delete io;
    return true;
}

void VirtualFileSystem::modifyFile( const QString & filename, const QByteArray & data ) {
    files[filename] = data;
}

QByteArray VirtualFileSystem::removeFile( const QString & filename ) {
    return files.take( filename );
}

void VirtualFileSystem::removeAllFiles() {
    QStringList keys = files.keys();
    foreach( const QString & key, keys ) {
        removeFile( key );
    }
}

bool VirtualFileSystem::fileExists( const QString & filename ) const {
    return files.contains( filename );
}

QByteArray & VirtualFileSystem::getFileByName( const QString & filename ) {
    return files[filename];
}

QByteArray VirtualFileSystem::getFileByName( const QString & filename ) const {
    return files.value( filename );
}

void VirtualFileSystem::setId( const QString & id ) {
    fileSystemName = id;
}

QString VirtualFileSystem::getId() const {
    return fileSystemName;
}

QStringList VirtualFileSystem::getAllFilenames() const {
    return files.keys();
}

/*******************************************
* VirtualFileSystemRegistry
*******************************************/

VirtualFileSystemRegistry::VirtualFileSystemRegistry() {
}

VirtualFileSystemRegistry::~VirtualFileSystemRegistry() {
    qDeleteAll( registry.values() );
}

bool VirtualFileSystemRegistry::registerFileSystem( VirtualFileSystem * entry ) {
    assert( NULL != entry );
    QString id = entry->getId();
    if( registry.contains( id ) ) {
        return false;
    }
    registry[id] = entry;
    return true;
}

VirtualFileSystem * VirtualFileSystemRegistry::unregisterFileSystem( const QString & id ) {
    return registry.take( id );
}

VirtualFileSystem * VirtualFileSystemRegistry::getFileSystemById( const QString & id ) const {
    return registry[id];
}

QList< VirtualFileSystem* > VirtualFileSystemRegistry::getAllFileSystems() const {
    return registry.values();
}

QStringList VirtualFileSystemRegistry::getAllIds() const {
    return registry.keys();
}

} // U2
