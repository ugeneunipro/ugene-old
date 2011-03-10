#include "SynchHttp.h"

namespace U2 {

SyncHTTP::SyncHTTP(const QString& hostName, quint16 port, QObject* parent)
: QHttp(hostName,port,parent), requestID(-1)
{
    connect(this,SIGNAL(requestFinished(int,bool)),SLOT(finished(int,bool)));
}

QString SyncHTTP::syncGet(const QString& path) {
    assert(requestID == -1);
    QBuffer to;
    requestID = get(path, &to);
    loop.exec();
    return QString(to.data());
}

QString SyncHTTP::syncPost(const QString & path, QIODevice * data) {
    assert(requestID == -1);
    QBuffer to;
    requestID = post(path, data, &to);
    loop.exec();
    return QString(to.data());
}

void SyncHTTP::finished(int idx, bool err) {
    assert(idx = requestID);Q_UNUSED(err); Q_UNUSED(idx);
    loop.exit();
}

}  // U2
