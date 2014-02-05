/**
 * UGENE - Integrated Bioinformatics Tools.
 * Copyright (C) 2008-2014 UniPro <ugene@unipro.ru>
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

#include <U2Lang/WorkflowUtils.h>

#include "ConfigurationEditor.h"
#include "Configuration.h"

namespace U2 {

Configuration::Configuration() : editor(NULL), validator(NULL) {
}

Configuration::~Configuration() {
    qDeleteAll(params.values());
}

QMap<QString, Attribute*> Configuration::getParameters() const {
    return params;
}

Attribute * Configuration::getParameter(const QString & name) const {
    return params.value(name);
}

Attribute * Configuration::removeParameter( const QString & name ) {
    Attribute *attr = params.take(name);
    attrs.removeOne(attr);
    return attr;
}

void Configuration::addParameter( const QString & name, Attribute * attr ) {
    assert(attr != NULL);
    params[name] = attr;
    attrs << attr;
}

void Configuration::setParameter(const QString& name, const QVariant& val) {
    if(params.contains(name)) {
        params[name]->setAttributeValue(val);
    }
}

void Configuration::setParameters(const QVariantMap& cfg) {
    QMapIterator<QString,QVariant> it(cfg);
    while (it.hasNext()) {
        it.next();
        setParameter(it.key(), it.value());
    }
}

QVariantMap Configuration::getValues() const {
    QVariantMap result;
    foreach (Attribute *attr, params) {
        result[attr->getId()] = attr->getAttributePureValue();
    }
    return result;
}

bool Configuration::hasParameter(const QString & name) const {
    return params.contains(name);
}

ConfigurationEditor * Configuration::getEditor() const {
    return editor;
}

void Configuration::setEditor(ConfigurationEditor* ed) {
    assert(ed != NULL);
    editor = ed;
}

ConfigurationValidator * Configuration::getValidator() {
    return validator;
}

void Configuration::setValidator(ConfigurationValidator* v) {
    assert(v != NULL);
    validator = v;
}

bool Configuration::validate(ProblemList &problemList) const {
    bool good = true;
    foreach(Attribute* a, getParameters()) {
        good &= a->validate(problemList);
    }
    if (validator) {
        good &= validator->validate(this, problemList);
    }
    return good;
}

QList<Attribute*> Configuration::getAttributes() const {
    return /*params.values()*/attrs;;
}

} // U2
