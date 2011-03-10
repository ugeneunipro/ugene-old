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

bool Configuration::hasParameter(const QString & name) const {
    return params.contains(name);
}

ConfigurationEditor * Configuration::getEditor() {
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

bool Configuration::validate(QStringList& errorList) const {
    bool good = true;
    foreach(Attribute* a, getParameters()) {
        if( !a->isRequiredAttribute() ) {
            continue;
        }
        if( a->isEmpty() || a->isEmptyString() ) {
            good = false;
            errorList.append(U2::WorkflowUtils::tr("Required parameter is not set: %1").arg(a->getDisplayName()));
        }
    }
    if (validator) {
        good &= validator->validate(this, errorList);
    }
    return good;
}

QList<Attribute*> Configuration::getAttributes() const {
    return /*params.values()*/attrs;;
}

} // U2
