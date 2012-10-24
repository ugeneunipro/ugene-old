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

#include "WizardWidget.h"

namespace U2 {

/**********************************
* WizardWidget
*********************************/
WizardWidget::WizardWidget() {

}

WizardWidget::~WizardWidget() {

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
* AttributeWidget
*********************************/
const QString AttributeWidgetHints::TYPE("type");
const QString AttributeWidgetHints::DEFAULT("default");
const QString AttributeWidgetHints::DATASETS("datasets");
const QString AttributeWidgetHints::LABEL("label");

AttributeWidget::AttributeWidget(Workflow::Actor *_actor, Attribute *_attr)
: WizardWidget(), actor(_actor), attr(_attr)
{

}

AttributeWidget::~AttributeWidget() {

}

void AttributeWidget::accept(WizardWidgetVisitor *visitor) {
    visitor->visit(this);
}

QString AttributeWidget::getActorId() const {
    return actor->getId();
}

QString AttributeWidget::getAttributeId() const {
    return attr->getId();
}

void AttributeWidget::setWigdetHints(const QVariantMap &value) {
    hints = value;
}

const QVariantMap & AttributeWidget::getWigdetHints() const {
    return hints;
}

QVariantMap AttributeWidget::getProperties() const {
    QVariantMap extHints = hints;
    extHints[AttributeWidgetHints::TYPE] = getProperty(AttributeWidgetHints::TYPE);
    extHints[AttributeWidgetHints::LABEL] = getProperty(AttributeWidgetHints::LABEL);
    return extHints;
}

QString AttributeWidget::getProperty(const QString &id) const {
    QString value = hints.value(id, "").toString();
    if (AttributeWidgetHints::TYPE == id && value.isEmpty()) {
        return AttributeWidgetHints::DEFAULT;
    } else if (AttributeWidgetHints::LABEL == id && value.isEmpty()) {
        return attr->getDisplayName();
    }
    return value;
}

Workflow::Actor * AttributeWidget::getActor() {
    return actor;
}

Attribute * AttributeWidget::getAttribute() {
    return attr;
}

} // U2
