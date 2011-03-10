#ifndef _BASE64_FILE_H_ 
#define _BASE64_FILE_H_

#include <QtCore/QFile>

extern "C" 
{
#include "base64/cencode.h"
} 

namespace U2 {

class Base64File : public QIODevice 
{
public:
    Base64File(const QString& url); 
    virtual bool isSequential() const { return true; }
    virtual bool open(OpenMode mode);
    virtual qint64 size() const;
    QString fileName() const { return file.fileName(); }
protected:
    virtual qint64 readData(char *data, qint64 maxlen);
    virtual qint64 writeData(const char* data, qint64 maxlen);

private:
    QByteArray          buf;
    qint64              bufLen;
    qint64              bufOffset;
    QFile               file;
    base64_encodestate  encodeState; 
};


} // namespace


#endif // _BASE64_FILE_H_
