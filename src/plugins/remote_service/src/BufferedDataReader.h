#ifndef _BUFFERED_DATA_READER_H_ 
#define _BUFFERED_DATA_READER_H_

#include <QtCore/QList>
#include <QtCore/QStringList>
#include <QtCore/QFile>
#include <QtCore/QBuffer>

#include "Base64File.h"

namespace U2 {

class BufferedDataReader : public QIODevice 
{
public:
    BufferedDataReader(const QStringList& inputUrls, const QByteArray& requestTemplate, const QByteArray& splitMarker); 
    ~BufferedDataReader();
    virtual bool isSequential() const { return true; }
    virtual bool open(OpenMode mode);
    virtual qint64 size() const;
    void setError(const QString& errMsg);
protected:
    virtual qint64 readData(char *data, qint64 maxlen);
    virtual qint64 writeData(const char* data, qint64 maxlen);

private:
    QList<QIODevice*>   inputDevs;
    QList<Base64File*>       inputFiles;
    QList<QBuffer*>     inputBufs;
    QList<QByteArray>   buffersData;
    int                 curIdx;
    bool                hasErrors;
    
};

} //namespace

#endif // _BUFFERED_DATA_READER_H_

