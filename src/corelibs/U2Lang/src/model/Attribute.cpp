/**
 * UGENE - Integrated Bioinformatics Tools.
 * Copyright (C) 2008-2012 UniPro <ugene@unipro.ru>
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

#include <cassert>

#include <QtCore/QStringList>

#include <U2Lang/HRSchemaSerializer.h>
#include "Attribute.h"

namespace U2 {

/*************************************
 *  Attribute
 *************************************/
Attribute::Attribute(const Descriptor& d, const DataTypePtr t, bool req, const QVariant & defaultValue )
: Descriptor(d), type(t), required(req), defaultValue(defaultValue) {
    value = defaultValue;
    debugCheckAttributeId();
}

void Attribute::debugCheckAttributeId() const {
    QString id = getId(); Q_UNUSED(id);
    assert(id != HRSchemaSerializer::TYPE_ATTR);
    assert(id != HRSchemaSerializer::NAME_ATTR);
    assert(id != HRSchemaSerializer::SCRIPT_ATTR);
    assert(id != HRSchemaSerializer::ELEM_ID_ATTR);
}

const DataTypePtr Attribute::getAttributeType()const {
    return type;
}

bool Attribute::isRequiredAttribute() const {
    return required;
}

void Attribute::setAttributeValue(const QVariant & newVal) {
    if (QVariant() == newVal) {
        value = defaultValue;
    } else {
        value = newVal;
    }
}

const QVariant &Attribute::getAttributePureValue() const {
    return value;
}

const QVariant &Attribute::getDefaultPureValue() const {
    return defaultValue;
}

bool Attribute::isDefaultValue() const {
    return (value == defaultValue);
}

const AttributeScript & Attribute::getAttributeScript() const {
    return scriptData;
}

AttributeScript & Attribute::getAttributeScript() {
    return scriptData;
}

QVariant Attribute::toVariant() const {
    QVariantList res;
    res << value;
    res << qVariantFromValue<QString>(scriptData.getScriptText());
    QVariantList scriptVars;
    foreach( const Descriptor & varDesc, scriptData.getScriptVars().keys() ) {
        scriptVars << qVariantFromValue<QString>(varDesc.getId());
    }
    res << QVariant(scriptVars);
    return res;
}

bool Attribute::fromVariant(const QVariant& variant) {
    if( !variant.canConvert( QVariant::List ) ) {
        return false;
    }
    QVariantList args = variant.toList();
    if( args.size() != 3 ) {
        return false;
    }
    value = args.at(0);
    QVariant scriptTextVal = args.at(1);
    QString scriptText;
    if( scriptTextVal.canConvert(QVariant::String) ) {
        scriptText = scriptTextVal.toString();
    }
    scriptData.setScriptText(scriptText);
    
    QVariant descs = args.at(2);
    if( descs.canConvert(QVariant::List) ) {
        QVariantList descList = descs.toList();
        for( int i = 0; i < descList.size(); ++i ) {
            scriptData.setScriptVar(Descriptor(descList.at(i).value<QString>(), "", "" ), QVariant());
        }
    }
    return true;
}

bool Attribute::isEmpty() const {
    return !value.isValid() || value.isNull();
}

bool Attribute::isEmptyString() const {
    return value.type() == QVariant::String && getAttributeValueWithoutScript<QString>().isEmpty();
}

void Attribute::addRelation(const AttributeRelation *relation) {
    relations.append(relation);
}

QVector<const AttributeRelation*> &Attribute::getRelations() {
    return relations;
}

Attribute *Attribute::clone() {
    return new Attribute(*this);
}

AttributeGroup Attribute::getGroup() {
    return COMMON_GROUP;
}

/*************************************
*  AttributeScript
*************************************/
AttributeScript::AttributeScript(const QString & t) : text(t) {}

bool AttributeScript::isEmpty() const {
    return text.isEmpty();
}

void AttributeScript::setScriptText(const QString & t) {
    text = t;
}

const QString & AttributeScript::getScriptText() const {
    return text;
}

const QMap<Descriptor, QVariant> & AttributeScript::getScriptVars() const {
    return vars;
}

void AttributeScript::clearScriptVars() {
    vars.clear();
}

void AttributeScript::setScriptVar(const Descriptor & desc, const QVariant & val) {
    vars.insert(desc, val);
}

bool AttributeScript::hasVarWithId(const QString & varName) const {
    foreach(const Descriptor & varDesc, vars.keys()) {
        if( varDesc.getId() == varName ) {
            return true;
        }
    }
    return false;
}

bool AttributeScript::hasVarWithDesc(const QString & varName) const {
    foreach(const Descriptor & varDesc, vars.keys()) {
        if( varDesc.getDisplayName() == varName ) {
            return true;
        }
    }
    return false;
}

void AttributeScript::setVarValueWithId(const QString & varName, const QVariant & value) {
    foreach(const Descriptor & varDesc, vars.keys()) {
        if( varDesc.getId() == varName ) {
            vars[varDesc] = value;
        }
    }
}

} // U2
