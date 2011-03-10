#ifndef _U2_HTTP_FILE_ADAPTER_H_
#define _U2_HTTP_FILE_ADAPTER_H_

#include <U2Core/IOAdapter.h>
#include <U2Core/NetworkConfiguration.h>

#include <QtCore/QEventLoop>
#include <QtCore/QLinkedList>


class QHttp;
class QHttpResponseHeader;
class QMutex;

namespace U2 {

class U2CORE_EXPORT HttpFileAdapterFactory: public IOAdapterFactory {
    Q_OBJECT
public:
    HttpFileAdapterFactory(QObject* p = NULL);

    virtual IOAdapter* createIOAdapter();

    virtual IOAdapterId getAdapterId() const {return BaseIOAdapters::HTTP_FILE;}

    virtual const QString& getAdapterName() const {return name;}

    virtual bool isIOModeSupported(IOAdapterMode m)  const {return m == IOAdapterMode_Read;} //read-only

    QNetworkProxy getProxyByUrl( const QUrl & url ) const;

    virtual TriState isResourceAvailable(const GUrl& ) const  {return TriState_Unknown;}

protected:
    QString name;
};

class U2CORE_EXPORT GzippedHttpFileAdapterFactory : public HttpFileAdapterFactory {
    Q_OBJECT
public:
    GzippedHttpFileAdapterFactory( QObject * obj = 0 );
    virtual IOAdapter * createIOAdapter();
    virtual IOAdapterId getAdapterId() const { return BaseIOAdapters::GZIPPED_HTTP_FILE; }
};

class HttpFileAdapter: public IOAdapter {
    Q_OBJECT
public:
    HttpFileAdapter(HttpFileAdapterFactory* f, QObject* o = NULL);
    ~HttpFileAdapter() {if (isOpen()) close();}

    virtual bool open(const GUrl& url, IOAdapterMode m);

    bool open(const QString& host, const QString & what, const QNetworkProxy & p, quint16 port=80, bool https=false);

    virtual bool isOpen() const {return (bool)http;}

    virtual void close();

    virtual qint64 readBlock(char* data, qint64 maxSize);

    virtual qint64 writeBlock(const char* data, qint64 size);

    virtual bool skip(qint64 nBytes);
    
    virtual qint64 left() const;

    virtual int getProgress() const;

    virtual GUrl getURL() const {return gurl;}

private:
    void        init();
    qint64      stored() const; 
    inline bool singleChunk() const { return chunk_list.size() == 1; }
    inline int  firstChunkContains() const {return (singleChunk() ? (isEmpty() ? 0 : end_ptr - begin_ptr) :
                                           CHUNKSIZE - begin_ptr);}
    inline void Empty() { assert( singleChunk() ); begin_ptr = -1; end_ptr = 0; }
    inline bool isEmpty() const { return -1 == begin_ptr && 0 == end_ptr; }
    void popFront();
    void readFromChunk( char * data, int size );
    void skipFromChunk( qint64 size );
    qint64 skipAhead( qint64 nBytes );
    qint64 waitData( qint64 until );

    static const int CHUNKSIZE = 32 * 1024;
    QLinkedList<QByteArray> chunk_list; 
    QByteArray cache;
    bool is_cached;
    int begin_ptr; //pointer to the first byte of data in first chunk
    int end_ptr; //pointer to the first free byte in last chunk 

    QHttp * http;
    bool badstate;
    bool is_downloaded;
    int downloaded;
    int total;

//    QMutex condmut;
//    QWaitCondition cond;
    QMutex rwmut;
    QEventLoop loop;
    GUrl gurl;
private slots:
    void add_data( const QHttpResponseHeader & resp );
    void done( bool error );
    void state( int state ); //debug only
    void progress( int done, int total );
};


}//namespace

#endif
