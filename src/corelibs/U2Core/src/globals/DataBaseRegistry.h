#ifndef _DATABASE_REGISTRY_
#define _DATABASE_REGISTRY_

#include <QtCore/QMap>
#include <QtCore/QString>
#include <QtCore/QList>
#include <QtCore/QObject>

#include <U2Core/global.h>
#include <U2Core/AnnotationData.h>
#include <U2Core/Task.h>

namespace U2 {

class HttpRequest {
public:
    virtual void sendRequest(const QString &program,const QString &query) = 0;
    virtual QList<SharedAnnotationData> getAnnotations() {return result;}
    virtual QString getError() { return error; }
    virtual QByteArray getOutputFile() {return NULL;}
    virtual ~HttpRequest() {};
    bool connectionError;
protected:
    HttpRequest(Task *_task):connectionError(false),error(""),task(_task){};
    QString error;
    QList<SharedAnnotationData> result;	
    Task *task;
};

class DataBaseFactory {
public:
    virtual ~DataBaseFactory() {}
    virtual HttpRequest * getRequest(Task *t) = 0;
};

class U2CORE_EXPORT DataBaseRegistry:public QObject{
    Q_OBJECT
public:
    DataBaseRegistry(QObject *o = 0);
    ~DataBaseRegistry();
    bool registerDataBase(DataBaseFactory *f, const QString &id);
    DataBaseFactory *getFactoryById(const QString& id);
    bool isRegistered(const QString& id);

private:
    QMap<QString,DataBaseFactory *> factories;
};

}

#endif