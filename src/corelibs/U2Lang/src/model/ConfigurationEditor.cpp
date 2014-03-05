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

#include <qglobal.h>
#if (QT_VERSION < 0x050000) //Qt 5
#include <QtGui/QHBoxLayout>
#else
#include <QtWidgets/QHBoxLayout>
#endif

#include "ConfigurationEditor.h"

namespace U2 {

PropertyWidget::PropertyWidget(QWidget *parent, DelegateTags *_tags)
    : QWidget(parent), _tags(_tags), schemaConfig(NULL)
{
    QHBoxLayout *l = new QHBoxLayout();
    l->setContentsMargins(0, 0, 0, 0);
    l->setSpacing(0);
    this->setLayout(l);
}

PropertyWidget::~PropertyWidget() {

}

void PropertyWidget::addMainWidget(QWidget *w) {
    w->setSizePolicy(QSizePolicy::MinimumExpanding, QSizePolicy::MinimumExpanding);
    layout()->addWidget(w);
    setFocusProxy(w);
}

QWidget * PropertyWidget::getField() {
    return NULL;
}

void PropertyWidget::setRequired() {

}

void PropertyWidget::activate() {

}

const DelegateTags * PropertyWidget::tags() const {
    return _tags;
}

void PropertyWidget::setDelegateTags(const DelegateTags *value) {
    _tags = value;
}

void PropertyWidget::setSchemaConfig(SchemaConfig *value) {
    schemaConfig = value;
}

PropertyDelegate::PropertyDelegate(QObject *parent)
: QItemDelegate(parent), schemaConfig(NULL)
{
    _tags = new DelegateTags();
    _tags->setParent(this);
}

PropertyDelegate::~PropertyDelegate() {

}

QVariant PropertyDelegate::getDisplayValue(const QVariant &v) const {
    return v;
}

PropertyDelegate * PropertyDelegate::clone() {
    return new PropertyDelegate(parent());
}

PropertyWidget * PropertyDelegate::createWizardWidget(U2OpStatus &os, QWidget * /*parent*/) const {
    os.setError("Unsupported operation");
    return NULL;
}

PropertyDelegate::Type PropertyDelegate::type() const {
    return NO_TYPE;
}

DelegateTags * PropertyDelegate::tags() const {
    return _tags;
}

void PropertyDelegate::setSchemaConfig(SchemaConfig *value) {
    schemaConfig = value;
}

DelegateTags::DelegateTags(QObject *parent)
: QObject(parent)
{

}

DelegateTags::DelegateTags(const DelegateTags &other) : QObject(NULL) {
    tags = other.tags;
}

QVariant DelegateTags::get(const QString &name) const {
    return tags.value(name);
}

void DelegateTags::set(const QString &name, const QVariant &value) {
    tags[name] = value;
}

void DelegateTags::set(const DelegateTags &other) {
    tags = other.tags;
}

QString DelegateTags::getString(const DelegateTags *tags, const QString &name) {
    CHECK(NULL != tags, "");
    return tags->get(name).toString();
}

} // U2
