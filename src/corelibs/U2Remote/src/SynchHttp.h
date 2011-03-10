#ifndef __SYNC_HTTP_H__
#define __SYNC_HTTP_H__

#include <QtNetwork/QHttp>
#include <QtCore/QEventLoop>
#include <QtCore/QBuffer>

#include <U2Core/global.h>

namespace U2 {

class U2REMOTE_EXPORT SyncHTTP : public QHttp {
    Q_OBJECT
public:
    SyncHTTP(const QString& hostName, quint16 port=80, QObject* parent=0);
    QString syncGet(const QString& path);
    QString syncPost(const QString & path, QIODevice * data);
protected slots:
    virtual void finished(int idx, bool err);

private:
    int requestID;
    QEventLoop loop;
};

} // U2

#endif
