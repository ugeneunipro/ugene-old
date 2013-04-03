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

#include "../PropertyWidget.h"

#include <U2Lang/BaseTypes.h>
#include <U2Lang/WorkflowUtils.h>

#include "PropertyWizardController.h"

namespace U2 {

/************************************************************************/
/* WizardAttributeController */
/************************************************************************/
PropertyWizardController::PropertyWizardController(WizardController *wc, AttributeWidget *_widget)
: WidgetController(wc), widget(_widget)
{
    actor = WorkflowUtils::actorById(wc->getCurrentActors(), widget->getActorId());
}

PropertyWizardController::~PropertyWizardController() {

}

Attribute * PropertyWizardController::attribute() {
    return actor->getParameter(widget->getAttributeId());
}

void PropertyWizardController::sl_valueChanged(const QVariant &newValue) {
    wc->setWidgetValue(widget, newValue);
}

/************************************************************************/
/* InUrlDatasetsController */
/************************************************************************/
InUrlDatasetsController::InUrlDatasetsController(WizardController *wc, AttributeWidget *widget)
: PropertyWizardController(wc, widget), dsc(NULL)
{

}

InUrlDatasetsController::~InUrlDatasetsController() {
    delete dsc;
}

QWidget * InUrlDatasetsController::createGUI(U2OpStatus & /*os*/) {
    if (NULL != dsc) {
        delete dsc;
    }
    QVariant value = wc->getWidgetValue(widget);
    if (value.canConvert< QList<Dataset> >()) {
        sets = value.value< QList<Dataset> >();
    } else {
        coreLog.error("Can not convert value to dataset list");
        sets.clear();
        sets << Dataset();
    }
    dsc = new DatasetsController(sets);
    connect(dsc, SIGNAL(si_attributeChanged()), SLOT(sl_datasetsChanged()));
    return dsc->getWigdet();
}

void InUrlDatasetsController::sl_datasetsChanged() {
    sl_valueChanged(qVariantFromValue< QList<Dataset> >(sets));
}

/************************************************************************/
/* SimpleAttributeController */
/************************************************************************/
DefaultPropertyController::DefaultPropertyController(WizardController *wc, AttributeWidget *widget, int _labelSize)
: PropertyWizardController(wc, widget), labelSize(_labelSize)
{

}

DefaultPropertyController::~DefaultPropertyController() {

}

QWidget * DefaultPropertyController::createGUI(U2OpStatus &os) {
    CHECK_EXT(AttributeWidgetHints::DEFAULT == widget->getProperty(AttributeWidgetHints::TYPE),
        os.setError("Widget type is not default"), NULL);

    PropertyWidget *propWidget = createPropertyWidget(os);
    CHECK_OP(os, NULL);
    connect(propWidget, SIGNAL(si_valueChanged(const QVariant &)), SLOT(sl_valueChanged(const QVariant &)));
    propWidget->setValue(wc->getWidgetValue(widget));

    QString label = widget->getProperty(AttributeWidgetHints::LABEL);
    if (label.isEmpty()) {
        label = attribute()->getDisplayName();
    }
    LabeledPropertyWidget *result = new LabeledPropertyWidget(label, propWidget, NULL);
    if (labelSize >= 0) {
        result->setLabelWidth(labelSize);
    }

    result->setToolTip("<html>" + attribute()->getDocumentation() + "</html>");
    return result;
}

PropertyWidget * DefaultPropertyController::createPropertyWidget(U2OpStatus &os) {
    PropertyDelegate *delegate = actor->getEditor()->getDelegate(widget->getAttributeId());
    PropertyWidget *result = NULL;

    if (NULL != delegate) {
        result = delegate->createWizardWidget(os, NULL);
        CHECK_OP(os, NULL);
    } else if (BaseTypes::BOOL_TYPE() == attribute()->getAttributeType()) {
        result = ComboBoxWidget::createBooleanWidget();
    } else if (BaseTypes::URL_DATASETS_TYPE() == attribute()->getAttributeType()) {
        URLLineEdit *lineEdit = new URLLineEdit("", "", true, false, false, NULL);
        result = new URLWidget(lineEdit);
    } else {
        result = new DefaultPropertyWidget();
    }

    return result;
}

} // U2
