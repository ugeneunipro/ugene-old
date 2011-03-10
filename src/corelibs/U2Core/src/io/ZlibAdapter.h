#ifndef _U2_ZLIB_ADAPTER_H_
#define _U2_ZLIB_ADAPTER_H_

#include <U2Core/IOAdapter.h>
#include "RingBuffer.h"

namespace U2 {

class GzipUtil;
struct GZipIndex;
struct GZipIndexAccessPoint;

class U2CORE_EXPORT ZlibAdapter: public IOAdapter {
    Q_OBJECT
public:
    ZlibAdapter(IOAdapter* io);
    ~ZlibAdapter();
    
    virtual bool open(const GUrl& url, IOAdapterMode m_ );

    virtual bool isOpen() const {return io->isOpen();}

    virtual void close();

    virtual qint64 readBlock(char* data, qint64 maxSize);

    virtual qint64 writeBlock(const char* data, qint64 size);

    virtual bool skip(qint64 nBytes);
    
    virtual qint64 left() const {return -1;}

    virtual int getProgress() const {return io->getProgress();}
    
    virtual qint64 bytesRead() const;
    
    virtual GUrl getURL() const;

    // should be invoked after open() ( needs z not null )
    bool skip( const GZipIndexAccessPoint& point, qint64 offset );
    
    // on error *ok set to false and GZipIndex() is returned
    // io - opened ioadapter, on the beginning of the file
    static GZipIndex buildGzipIndex( IOAdapter* io, qint64 span, bool* ok = NULL );
    
private:
    static const int BUFLEN = 32768;
    IOAdapter* io;
    GzipUtil* z;
    RingBuffer* buf; // seek buffer
    int rewinded; // how much should read from seek buffer
};

struct GZipIndexAccessPoint {
    qint64     out;    // corresponding offset in uncompressed data
    qint64     in;     // offset in input file of first full byte
    int        bits;   // number of bits (1-7) from byte at in - 1, or 0
    QByteArray window; //preceding WINSIZE of uncompressed data
};

struct U2CORE_EXPORT GZipIndex {
    static const int    WINSIZE = 32768;
    static const qint64 SPAN    = 1048576L;
    static const int    CHUNK   = 16384;
    
    QList< GZipIndexAccessPoint > points;
}; // GZipIndex

};//namespace

#endif
