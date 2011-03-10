#include <U2Core/AppContext.h>
#include "ProjectModel.h"

namespace U2 {

void Project::setupToEngine(QScriptEngine *engine)
{
    Document::setupToEngine(engine);
    qScriptRegisterMetaType(engine, toScriptValue, fromScriptValue);
    qScriptRegisterSequenceMetaType<QList<Document*> >(engine);
};
QScriptValue Project::toScriptValue(QScriptEngine *engine, Project* const &in)
{ 
    return engine->newQObject(in); 
}

void Project::fromScriptValue(const QScriptValue &object, Project* &out) 
{
    out = qobject_cast<Project*>(object.toQObject()); 
}

}//namespace
