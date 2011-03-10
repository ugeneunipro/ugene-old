#ifndef _U2_MEM_BUF_ADAPTER_H_
#define _U2_MEM_BUF_ADAPTER_H_

#include <U2Core/IOAdapter.h>

namespace U2 {

class U2CORE_EXPORT VFSAdapterFactory: public IOAdapterFactory {
    Q_OBJECT
public:
    VFSAdapterFactory(QObject* p = NULL);

    virtual IOAdapter* createIOAdapter();

    virtual IOAdapterId getAdapterId() const {return BaseIOAdapters::VFS_FILE;}

    virtual const QString& getAdapterName() const {return name;}

    virtual bool isIOModeSupported(IOAdapterMode m)  const {Q_UNUSED(m); return true;} //files can be read and be written

    virtual TriState isResourceAvailable(const GUrl& url) const {
        assert(url.isVFSFile());
        Q_UNUSED(url);
        return TriState_Yes;
    }

protected:
    QString name;
};


class U2CORE_EXPORT VFSAdapter: public IOAdapter {
    Q_OBJECT
public:
    VFSAdapter(VFSAdapterFactory* f, QObject* o = NULL);
    ~VFSAdapter() {if (isOpen()) close();}

    virtual bool open(const GUrl& url, IOAdapterMode m);

    virtual bool isOpen() const {return buffer != NULL;}

    virtual void close();

    virtual qint64 readBlock(char* data, qint64 maxSize);

    virtual qint64 writeBlock(const char* data, qint64 size);

    virtual bool skip(qint64 nBytes);

    virtual qint64 left() const;

    virtual int getProgress() const;

    virtual qint64 bytesRead() const;

    virtual GUrl getURL() const {return url;}

private:
    GUrl        url;
    QBuffer*    buffer;
};


}//namespace

#endif // _U2_MEM_BUF_ADAPTER_H_
