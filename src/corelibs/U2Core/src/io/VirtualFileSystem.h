
#ifndef _U2_VIRTUAL_FILE_SYSTEM_H_
#define _U2_VIRTUAL_FILE_SYSTEM_H_

#include <QtCore/QString>
#include <QtCore/QStringList>
#include <QtCore/QByteArray>
#include <QtCore/QMap>

#include <U2Core/global.h>

namespace U2 {

/*
 * VirtualFileSystem represents files mapped to memory
 * usage: map files to memory then use MemBufAdapter to read them
 */
class U2CORE_EXPORT VirtualFileSystem {
public:
    // if you want MemBufAdapter reading-writing to your vfs: make url of your file as URL_PREFIX+vfsname+URL_NAME_SEPARATOR+filename
    static const QString    URL_PREFIX;
    static const QString    URL_NAME_SEPARATOR; // NOTE: vfs name should not contain this separator
    static const int        READ_BLOCK_SZ = 4096;
    
public:
    VirtualFileSystem();
    VirtualFileSystem( const QString & fsName );
    ~VirtualFileSystem();
    
    // if file exist in vfs: return false, do nothing
    bool createFile( const QString & filename, const QByteArray & data );
    
    // maps filePath file from disk to memory and adds it to vfs
    // if filename exists in vfs: replace prev
    bool mapFile( const QString & filename, const QString & filePath );
    // maps back to disk (from memory)
    bool mapBack( const QString & filename, const QString & filePath ) const;
    // if file does not exist: create new file
    void modifyFile( const QString & filename, const QByteArray & data );
    
    QByteArray removeFile(  const QString & filename );
    void removeAllFiles();
    
    bool fileExists( const QString & filename ) const;
    QByteArray & getFileByName( const QString & filename );
    QByteArray getFileByName( const QString & filename ) const;
    
    void setId( const QString & id );
    QString getId() const;
    QStringList getAllFilenames() const;
    
private:
    QString fileSystemName; // unique id of fs
    QMap< QString, QByteArray > files;
    
}; // VirtualFileSystem

class U2CORE_EXPORT VirtualFileSystemRegistry {
public:
    VirtualFileSystemRegistry();
    ~VirtualFileSystemRegistry();
    
    bool registerFileSystem( VirtualFileSystem * entry );
    VirtualFileSystem * unregisterFileSystem( const QString & id );
    
    VirtualFileSystem * getFileSystemById( const QString & id ) const;
    QList< VirtualFileSystem* > getAllFileSystems() const;
    QStringList getAllIds() const;
    
private:
    QMap< QString, VirtualFileSystem* > registry;
    
}; // VirtualFileSystemRegistry

} // U2

#endif // _U2_VIRTUAL_FILE_SYSTEM_H_
