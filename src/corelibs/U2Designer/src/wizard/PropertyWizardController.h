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

#ifndef _U2_PROPERTYWIZARDCONTROLLER_H_
#define _U2_PROPERTYWIZARDCONTROLLER_H_

#include <U2Core/U2OpStatus.h>

#include <U2Designer/DatasetsController.h>
#include <U2Designer/WizardController.h>

#include <U2Lang/ConfigurationEditor.h>
#include <U2Lang/WizardWidget.h>

#include "WidgetController.h"

namespace U2 {

/************************************************************************/
/* WizardAttributeController */
/************************************************************************/
class PropertyWizardController : public WidgetController {
    Q_OBJECT
public:
    PropertyWizardController(WizardController *wc, AttributeWidget *widget);
    virtual ~PropertyWizardController();

    virtual QWidget * createGUI(U2OpStatus &os) = 0;

protected:
    Actor *actor;
    AttributeWidget *widget;

protected:
    Attribute * attribute();

protected slots:
    void sl_valueChanged(const QVariant &newValue);
};

/************************************************************************/
/* InUrlDatasetsController */
/************************************************************************/
class InUrlDatasetsController : public PropertyWizardController {
    Q_OBJECT
public:
    InUrlDatasetsController(WizardController *wc, AttributeWidget *widget);
    virtual ~InUrlDatasetsController();

    virtual QWidget * createGUI(U2OpStatus &os);

private:
    DatasetsController *dsc;
    QList<Dataset> sets;

private slots:
    void sl_datasetsChanged();
};

/************************************************************************/
/* DefaultPropertyController */
/************************************************************************/
class DefaultPropertyController : public PropertyWizardController {
public:
    DefaultPropertyController(WizardController *wc, AttributeWidget *widget, int labelSize);
    virtual ~DefaultPropertyController();

    virtual QWidget * createGUI(U2OpStatus & os);

private:
    int labelSize;

private:
    PropertyWidget * createPropertyWidget(U2OpStatus &os);
};

} // U2

#endif // _U2_PROPERTYWIZARDCONTROLLER_H_
