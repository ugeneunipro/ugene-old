/**
 * UGENE - Integrated Bioinformatics Tools.
 * Copyright (C) 2008-2013 UniPro <ugene@unipro.ru>
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

#include <U2Core/U2SafePoints.h>

#include <U2Lang/WizardWidgetVisitor.h>
#include <U2Lang/WorkflowUtils.h>

#include "WizardWidget.h"

namespace U2 {

/**********************************
* WizardWidget
*********************************/
WizardWidget::WizardWidget() {

}

WizardWidget::~WizardWidget() {

}

void WizardWidget::validate(const QList<Actor*> & /*actors*/, U2OpStatus & /*os*/) const {

}

/**********************************
* LogoWidget
*********************************/
const QString LogoWidget::ID("logo");

LogoWidget::LogoWidget(const QString &_logoPath)
: WizardWidget(), logoPath(_logoPath)
{

}

LogoWidget::~LogoWidget() {

}

void LogoWidget::accept(WizardWidgetVisitor *visitor) {
    visitor->visit(this);
}

void LogoWidget::setLogoPath(const QString &value) {
    logoPath = value;
}

const QString & LogoWidget::getLogoPath() const {
    return logoPath;
}

bool LogoWidget::isDefault() const {
    return ("" == logoPath);
}

/**********************************
* WidgetsArea
*********************************/
WidgetsArea::WidgetsArea(const QString &_name, const QString &_title)
: WizardWidget(), titleable(true), name(_name), title(_title)
{
    labelSize = -1;
}

WidgetsArea::WidgetsArea(const QString &_name)
: WizardWidget(), titleable(false), name(_name)
{
    labelSize = -1;
}

WidgetsArea::~WidgetsArea() {

}

void WidgetsArea::accept(WizardWidgetVisitor *visitor) {
    visitor->visit(this);
}

void WidgetsArea::validate(const QList<Actor*> &actors, U2OpStatus &os) const {
    foreach (WizardWidget *w, widgets) {
        w->validate(actors, os);
        CHECK_OP(os, );
    }
}

void WidgetsArea::addWidget(WizardWidget *widget) {
    widgets << widget;
}

const QList<WizardWidget*> & WidgetsArea::getWidgets() const {
    return widgets;
}

const QString & WidgetsArea::getName() const {
    return name;
}

const QString & WidgetsArea::getTitle() const {
    return title;
}

void WidgetsArea::setTitle(const QString &value) {
    titleable = true;
    title = value;
}

bool WidgetsArea::hasLabelSize() const {
    return (-1 != labelSize);
}

int WidgetsArea::getLabelSize() const {
    return labelSize;
}

void WidgetsArea::setLabelSize(int value) {
    labelSize = value;
}

/**********************************
* GroupWidget
*********************************/
const QString GroupWidget::ID("group");

GroupWidget::GroupWidget()
: WidgetsArea(ID), type(DEFAULT)
{

}

GroupWidget::GroupWidget(const QString &title, Type _type)
: WidgetsArea(ID, title), type(_type)
{

}

GroupWidget::~GroupWidget() {

}

void GroupWidget::accept(WizardWidgetVisitor *visitor) {
    visitor->visit(this);
}

void GroupWidget::setType(Type value) {
    type = value;
}

GroupWidget::Type GroupWidget::getType() const {
    return type;
}

/**********************************
* AttributeInfo
*********************************/
const QString AttributeInfo::TYPE("type");
const QString AttributeInfo::DEFAULT("default");
const QString AttributeInfo::DATASETS("datasets");
const QString AttributeInfo::LABEL("label");

AttributeInfo::AttributeInfo(const QString &_actorId, const QString &_attrId, const QVariantMap &_hints)
: actorId(_actorId), attrId(_attrId), hints(_hints)
{

}

void AttributeInfo::validate(const QList<Actor*> &actors, U2OpStatus &os) const {
    Actor *actor = WorkflowUtils::actorById(actors, actorId);
    if (NULL == actor) {
        os.setError(QObject::tr("Unknown actor id: %1").arg(actorId));
        return;
    }
    if (!actor->hasParameter(attrId)) {
        os.setError(QObject::tr("Actor '%1' does not have this parameter: %2").arg(actorId).arg(attrId));
        return;
    }
}

bool AttributeInfo::operator== (const AttributeInfo &other) const {
    return toString() == other.toString();
}

QString AttributeInfo::toString() const {
    return actorId + ":" + attrId;
}

AttributeInfo AttributeInfo::fromString(const QString &value, U2OpStatus &os) {
    QStringList tokens = value.split(":");
    if (2 != tokens.size()) {
        os.setError("Bad attribute value: " + value);
        return AttributeInfo("", "");
    }
    return AttributeInfo(tokens[0], tokens[1]);
}

/**********************************
* AttributeWidget
*********************************/
AttributeWidget::AttributeWidget()
: WizardWidget(), info("", "")
{

}

AttributeWidget::~AttributeWidget() {

}

void AttributeWidget::accept(WizardWidgetVisitor *visitor) {
    visitor->visit(this);
}

void AttributeWidget::validate(const QList<Actor*> &actors, U2OpStatus &os) const {
    info.validate(actors, os);
}

QString AttributeWidget::getActorId() const {
    return info.actorId;
}

QString AttributeWidget::getAttributeId() const {
    return info.attrId;
}

void AttributeWidget::setInfo(const AttributeInfo &value) {
    info = value;
}

const AttributeInfo & AttributeWidget::getInfo() const {
    return info;
}

const QVariantMap & AttributeWidget::getWigdetHints() const {
    return info.hints;
}

QVariantMap AttributeWidget::getProperties() const {
    QVariantMap extHints = info.hints;
    extHints[AttributeInfo::TYPE] = getProperty(AttributeInfo::TYPE);
    extHints[AttributeInfo::LABEL] = getProperty(AttributeInfo::LABEL);
    return extHints;
}

QString AttributeWidget::getProperty(const QString &id) const {
    QString value = info.hints.value(id, "").toString();
    if (AttributeInfo::TYPE == id && value.isEmpty()) {
        return AttributeInfo::DEFAULT;
    } else if (AttributeInfo::LABEL == id && value.isEmpty()) {
        return "";
    }
    return value;
}

/************************************************************************/
/* DatasetsWizardWidget */
/************************************************************************/
const QString PairedReadsWidget::ID = "paired-reads-datasets";

PairedReadsWidget::PairedReadsWidget()
: WizardWidget()
{

}

void PairedReadsWidget::accept(WizardWidgetVisitor *visitor) {
    visitor->visit(this);
}

void PairedReadsWidget::validate(const QList<Actor*> &actors, U2OpStatus &os) const {
    foreach (const AttributeInfo &info, infos) {
        info.validate(actors, os);
        CHECK_OP(os, );
    }
}

void PairedReadsWidget::addInfo(const AttributeInfo &value) {
    infos << value;
}

QList<AttributeInfo> PairedReadsWidget::getInfos() const {
    return infos;
}

/************************************************************************/
/* RadioWidget */
/************************************************************************/
const QString RadioWidget::ID("radio");
RadioWidget::Value::Value(QString _id, QString _label)
: id(_id), label(_label)
{

}

RadioWidget::RadioWidget()
: WizardWidget()
{

}


void RadioWidget::accept(WizardWidgetVisitor *visitor) {
    visitor->visit(this);
}

const QString & RadioWidget::var() const {
    return _var;
}

void RadioWidget::setVar(const QString &value) {
    _var = value;
}

const QList<RadioWidget::Value> & RadioWidget::values() const {
    return _values;
}

void RadioWidget::add(const Value &value) {
    _values << value;
}

} // U2
