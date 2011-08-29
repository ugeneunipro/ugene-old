/**
 * UGENE - Integrated Bioinformatics Tools.
 * Copyright (C) 2008-2011 UniPro <ugene@unipro.ru>
 * http://ugene.unipro.ru
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston,
 * MA 02110-1301, USA.
 */

#ifndef _U2_WORKFLOW_ATTR_H_
#define _U2_WORKFLOW_ATTR_H_

#include <cassert>

#include <QtCore/QVariant>
#include <QtScript/QScriptValue>
#include <QtScript/QScriptEngine>

#include <U2Core/Task.h>
#include <U2Core/ScriptTask.h>

#include <U2Lang/AttributeRelation.h>
#include <U2Lang/Datatype.h>
#include <U2Lang/Descriptor.h>
#include <U2Lang/ScriptLibrary.h>

namespace U2 {

typedef QString ActorId;
inline ActorId str2aid(const QString& s) {return s;}

/**
 * attribute value can be obtained from script
 */
class U2LANG_EXPORT AttributeScript {
public:
    AttributeScript(const QString & text);
    AttributeScript() {}
    
    bool isEmpty() const;
    
    void setScriptText(const QString & t);
    const QString & getScriptText() const;
    
    void setScriptVar(const Descriptor & desc, const QVariant & val);
    const QMap<Descriptor, QVariant> & getScriptVars() const;
    void clearScriptVars();
    
    bool hasVarWithId(const QString & varName)const;
    bool hasVarWithDesc(const QString & varName)const;
    void setVarValueWithId(const QString & varName, const QVariant & value);
    
private:
    QString                         text;
    QMap<Descriptor, QVariant>      vars; // (desc, val)
    
}; // AttributeScript

/**
 * Value of certain type that can be identified by (descriptors) id
 */
class U2LANG_EXPORT Attribute : public Descriptor {
public:
    Attribute(const Descriptor& d, const DataTypePtr type, bool required = false, const QVariant & defaultValue = QVariant());
    
    // getters/setters
    const DataTypePtr getAttributeType()const;
    bool isRequiredAttribute() const;
    
    void setAttributeValue(const QVariant & newVal);
    // attribute value is kept in qvariant
    // but it can be transformed to value of specific type using scripting or not (see getAttributeValue)
    const QVariant & getAttributePureValue() const;
    
    // base realization without scripting. to support scripting for other types: see template realizations
    template<typename T> T getAttributeValue() const {
        return getAttributeValueWithoutScript<T>();
    }
    
    const AttributeScript & getAttributeScript() const;
    // used to change script data
    AttributeScript & getAttributeScript();
    
    // stores value and script data in variant
    // used in saving schema to xml
    QVariant toVariant() const;
    // reads value and script from variant
    // used in reading schema from xml
    bool fromVariant(const QVariant& variant);
    
    bool isEmpty() const;
    bool isEmptyString() const;
    bool isVisible(const QVariantMap &values) const;
    void addRelation(const AttributeRelation *relation);
    
private:
    template<typename T> T getAttributeValueWithoutScript() const {
        return value.value<T>();
    }
    
    void debugCheckAttributeId() const;
    
private:
    // type of value
    const DataTypePtr   type;
    // attribute can be required or not
    // values of required attributes cannot be empty
    // used in configuration validations
    const bool          required;
    // pure value. if script exists, value should be processed throw it
    QVariant            value;
    // script text and variable values for script evaluating
    // script variables get values only in runtime
    AttributeScript     scriptData;

    QVector<const AttributeRelation*> relations;
    
}; // Attribute


// getAttributeValue function realizations with scripting support
template<>
inline QString Attribute::getAttributeValue() const {
    if( scriptData.isEmpty() ) {
        return getAttributeValueWithoutScript<QString>();
    }
    // run script
    QScriptEngine engine;
    QMap<QString, QScriptValue> scriptVars;
    foreach( const Descriptor & key, scriptData.getScriptVars().uniqueKeys() ) {
        assert(!key.getId().isEmpty());
        scriptVars[key.getId()] = engine.newVariant(scriptData.getScriptVars().value(key));
    }
    
    TaskStateInfo tsi;
    WorkflowScriptLibrary::initEngine(&engine);
    QScriptValue scriptResult = ScriptTask::runScript(&engine, scriptVars, scriptData.getScriptText(), tsi);
    
    // FIXME: report errors!
    // FIXME: write to log
    if( tsi.cancelFlag ) {
        if( !tsi.hasError() ) {
            tsi.setError("Script task canceled");
        }
    }
    if(tsi.hasError()) {
        return QString();
    }
    if( scriptResult.isString() ) {
        return scriptResult.toString();
    }
    
    return QString();
}

template<>
inline int Attribute::getAttributeValue() const {
    if( scriptData.isEmpty() ) {
        return getAttributeValueWithoutScript<int>();
    }

    QScriptEngine engine;
    QMap<QString, QScriptValue> scriptVars;
    foreach( const Descriptor & key, scriptData.getScriptVars().uniqueKeys() ) {
        assert(!key.getId().isEmpty());
        scriptVars[key.getId()] = engine.newVariant(scriptData.getScriptVars().value(key));
    }

    TaskStateInfo tsi;


    QScriptValue scriptResult = ScriptTask::runScript(&engine, scriptVars, scriptData.getScriptText(), tsi);

    if( tsi.cancelFlag ) {
        if( !tsi.hasError() ) {
            tsi.setError("Script task canceled");
        }
    }
    if(tsi.hasError()) {
        return 0;
    }
    if( scriptResult.isNumber() ) {
        return scriptResult.toInt32();
    }

    return 0;
}

} // U2 namespace

Q_DECLARE_METATYPE(U2::AttributeScript)

#endif
