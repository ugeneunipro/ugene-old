#ifndef _U2_DBXREF_H_
#define _U2_DBXREF_H_

#include <U2Core/global.h>

#include <QtCore/QMap>
#include <QtCore/QObject>
#include <QtScript/QScriptValue>
#include <QtScript/QScriptEngine>
#include <QtCore/QStringList>

namespace U2 {

class U2CORE_EXPORT DBXRefInfo {
public:
    DBXRefInfo(){};
    DBXRefInfo(const QString& _name, const QString& _url, 
               const QString& _fileUrl, const QString& _comment) 
               : name(_name), url(_url), fileUrl(_fileUrl), comment(_comment){}

    QString name;
    QString url;
    QString fileUrl;
    QString comment;

    static void setupToEngine(QScriptEngine *engine);
private:    
    static QScriptValue toScriptValue(QScriptEngine *engine, DBXRefInfo const &in);
    static void fromScriptValue(const QScriptValue &object, DBXRefInfo &out);
};


class U2CORE_EXPORT DBXRefRegistry : public QObject {
    Q_OBJECT
public:
    DBXRefRegistry(QObject* p = NULL);

    Q_INVOKABLE DBXRefInfo getRefByKey(const QString& dbxrefKey)const {return refsByKey.value(dbxrefKey);}

    static void setupToEngine(QScriptEngine *engine);
    const QMap<QString,DBXRefInfo>& getEntries() { return refsByKey; }
private:
    static QScriptValue toScriptValue(QScriptEngine *engine, DBXRefRegistry* const &in);
    static void fromScriptValue(const QScriptValue &object, DBXRefRegistry* &out);

    QMap<QString,DBXRefInfo> refsByKey;
};
}//namespace
Q_DECLARE_METATYPE(U2::DBXRefRegistry*)
Q_DECLARE_METATYPE(U2::DBXRefInfo)

#endif
