#ifndef _U2_LOCAL_FILE_ADAPTER_H_
#define _U2_LOCAL_FILE_ADAPTER_H_

#include <U2Core/IOAdapter.h>

#include <QtCore/QFileInfo>

namespace U2 {

class U2CORE_EXPORT LocalFileAdapterFactory: public IOAdapterFactory {
    Q_OBJECT
public:
    LocalFileAdapterFactory(QObject* p = NULL);

    virtual IOAdapter* createIOAdapter();

    virtual IOAdapterId getAdapterId() const {return BaseIOAdapters::LOCAL_FILE;}

    virtual const QString& getAdapterName() const {return name;}

    virtual bool isIOModeSupported(IOAdapterMode m)  const {Q_UNUSED(m); return true;} //files can be read and be written

    virtual TriState isResourceAvailable(const GUrl& url) const {return QFileInfo(url.getURLString()).exists() ? TriState_Yes : TriState_No;}

protected:
    QString name;
};

class U2CORE_EXPORT GzippedLocalFileAdapterFactory: public LocalFileAdapterFactory {
    Q_OBJECT
public:
    GzippedLocalFileAdapterFactory(QObject* p = NULL);

    virtual IOAdapter* createIOAdapter();

    virtual IOAdapterId getAdapterId() const {return BaseIOAdapters::GZIPPED_LOCAL_FILE;}
};



class U2CORE_EXPORT LocalFileAdapter: public IOAdapter {
    Q_OBJECT
public:
    LocalFileAdapter(LocalFileAdapterFactory* f, QObject* o = NULL);
    ~LocalFileAdapter() {if (isOpen()) close();}

    virtual bool open(const GUrl& url, IOAdapterMode m);

    virtual bool isOpen() const {return f!=NULL;}

    virtual void close();

    virtual qint64 readBlock(char* data, qint64 maxSize);

    virtual qint64 writeBlock(const char* data, qint64 size);

    virtual bool skip(qint64 nBytes);
    
    virtual qint64 left() const;
    virtual int getProgress() const;
    
    virtual qint64 bytesRead() const;
    
    virtual GUrl getURL() const;
    
private:
    QFile* f;
};


}//namespace

#endif
