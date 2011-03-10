#include "DBXRefRegistry.h"
#include <U2Core/Log.h>

#include <QtCore/QFile>
#include <QtCore/QTextStream>
#include <QtGui/QApplication>

namespace U2 {

#define DB_XREF_FILE_NAME "DBXRefRegistry.txt"

DBXRefRegistry::DBXRefRegistry(QObject *p) : QObject(p) {
    QFile file( QString(PATH_PREFIX_DATA)+ ":" + DB_XREF_FILE_NAME );

    if(!file.exists() || !file.open(QIODevice::ReadOnly)){
        coreLog.error(tr("File with db_xref mappings not found: %1").arg(DB_XREF_FILE_NAME));
        return;
    } 
    QTextStream in(&file);
    while (!in.atEnd()) {
        QString line = in.readLine();
        if(line.length()==0 || line.startsWith("#")) {
            continue;
        }
        QStringList fields = line.split("|");
        if (fields.size()!=4) {
            coreLog.error(tr("Illegal db_xref file entry: %1").arg(line));
            continue;
        }
        DBXRefInfo info(fields[0], fields[1], fields[2],fields[3].trimmed());
        refsByKey[info.name] = info;
    }
    file.close();
}
QScriptValue DBXRefRegistry::toScriptValue(QScriptEngine *engine, DBXRefRegistry* const &in)
{ return engine->newQObject(in); }

void DBXRefRegistry::fromScriptValue(const QScriptValue &object, DBXRefRegistry* &out)
{ out = (DBXRefRegistry*)(object.toQObject()); }

void DBXRefRegistry::setupToEngine(QScriptEngine *engine){
    DBXRefInfo::setupToEngine(engine);
    qScriptRegisterMetaType(engine, toScriptValue, fromScriptValue);
}

QScriptValue DBXRefInfo::toScriptValue(QScriptEngine *engine, DBXRefInfo const &in)
{ 
    QScriptValue res=engine->newObject();
    res.setProperty("name", QScriptValue(engine,in.name));
    res.setProperty("url", QScriptValue(engine,in.url));
    res.setProperty("fileUrl", QScriptValue(engine,in.fileUrl));
    res.setProperty("comment", QScriptValue(engine,in.comment));
    return res;
}

void DBXRefInfo::fromScriptValue(const QScriptValue &object, DBXRefInfo &out)
{ out = *((DBXRefInfo*)(object.toQObject())); }

void DBXRefInfo::setupToEngine(QScriptEngine *engine){
    qScriptRegisterMetaType(engine, toScriptValue, fromScriptValue);
}

}//namespace
