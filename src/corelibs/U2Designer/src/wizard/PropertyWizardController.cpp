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

#include <QHBoxLayout>
#include <QLineEdit>
#include <QSpinBox>

#include <U2Core/U2SafePoints.h>

#include "../PropertyWidget.h"

#include <U2Lang/BaseTypes.h>

#include "PropertyWizardController.h"

namespace U2 {

/************************************************************************/
/* WizardAttributeController */
/************************************************************************/
PropertyWizardController::PropertyWizardController(AttributeWidget *_widget)
: widget(_widget)
{

}

PropertyWizardController::~PropertyWizardController() {

}

void PropertyWizardController::assignPropertyValue() {
    widget->getAttribute()->setAttributeValue(getResult());
}

/************************************************************************/
/* InUrlDatasetsController */
/************************************************************************/
InUrlDatasetsController::InUrlDatasetsController(AttributeWidget *widget)
: PropertyWizardController(widget), dsc(NULL)
{
    URLAttribute *attr = dynamic_cast<URLAttribute*>(widget->getAttribute());
    SAFE_POINT(NULL != attr, "NULL url attribute in widget", );
    foreach (const Dataset &set, attr->getDatasets()) {
        sets << set;
    }
    dsc = new DatasetsController(sets);
}

InUrlDatasetsController::~InUrlDatasetsController() {
    delete dsc;
}

QWidget * InUrlDatasetsController::createGUI(U2OpStatus & os) {
    CHECK_EXT(NULL != dsc, os.setError("NULL datasets controller"), NULL);
    return dsc->getWigdet();
}

QVariant InUrlDatasetsController::getResult() {
    CHECK(NULL != dsc, QVariant());
    return qVariantFromValue< QList<Dataset> >(dsc->getDatasets());
}

/************************************************************************/
/* SimpleAttributeWidget */
/************************************************************************/
LabeledPropertyWidget::LabeledPropertyWidget(const QString &labelText, PropertyWidget *widget, QWidget *parent)
: QWidget(parent)
{
    QHBoxLayout *layout = new QHBoxLayout(this);
    setLayout(layout);

    label = new QLabel(labelText, this);
    layout->addWidget(label);
    layout->addWidget(widget);

    layout->setContentsMargins(0, 0, 0, 0);
    layout->setSpacing(0);
}

void LabeledPropertyWidget::setLabelWidth(int width) {
    label->setFixedWidth(width);
}

/************************************************************************/
/* SimpleAttributeController */
/************************************************************************/
DefaultPropertyController::DefaultPropertyController(AttributeWidget *widget, int _labelSize)
: PropertyWizardController(widget), propWidget(NULL), labelSize(_labelSize)
{

}

DefaultPropertyController::~DefaultPropertyController() {
    CHECK(NULL != propWidget, );
    CHECK(NULL == propWidget->parent(), );
    delete propWidget;
}

QWidget * DefaultPropertyController::createGUI(U2OpStatus &os) {
    CHECK_EXT(AttributeWidgetHints::DEFAULT == widget->getProperty(AttributeWidgetHints::TYPE),
        os.setError("Widget type is not default"), NULL);

    Attribute *attr = widget->getAttribute();
    Workflow::Actor *actor = widget->getActor();
    PropertyDelegate *delegate = actor->getEditor()->getDelegate(widget->getAttributeId());

    if (NULL != delegate) {
        propWidget = delegate->createWizardWidget(os, NULL);
        SAFE_POINT_OP(os, NULL);
    } else if (BaseTypes::URL_DATASETS_TYPE() == attr->getAttributeType()) {
        URLLineEdit *lineEdit = new URLLineEdit("", "", true, false, false, NULL);
        propWidget = new URLWidget(lineEdit);
    } else {
        propWidget = new DefaultPropertyWidget();
    }

    propWidget->setValue(attr->getAttributePureValue());
    QString label = widget->getProperty(AttributeWidgetHints::LABEL);
    LabeledPropertyWidget *result = new LabeledPropertyWidget(label, propWidget, NULL);
    if (labelSize >= 0) {
        result->setLabelWidth(labelSize);
    }
    return result;
}

QVariant DefaultPropertyController::getResult() {
    CHECK(NULL != propWidget, QVariant());
    return propWidget->value();
}

} // U2
