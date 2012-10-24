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

#ifndef _U2_PROPERTYWIZARDCONTROLLER_H_
#define _U2_PROPERTYWIZARDCONTROLLER_H_

#include <QLabel>

#include <U2Core/U2OpStatus.h>

#include <U2Designer/DatasetsController.h>

#include <U2Lang/ConfigurationEditor.h>
#include <U2Lang/WizardWidget.h>

namespace U2 {

/************************************************************************/
/* WizardAttributeController */
/************************************************************************/
class PropertyWizardController {
public:
    PropertyWizardController(AttributeWidget *widget);
    virtual ~PropertyWizardController();

    virtual QWidget * createGUI(U2OpStatus &os) = 0;
    virtual QVariant getResult() = 0;

    void assignPropertyValue();

protected:
    AttributeWidget *widget;
};

/************************************************************************/
/* InUrlDatasetsController */
/************************************************************************/
class InUrlDatasetsController : public PropertyWizardController {
public:
    InUrlDatasetsController(AttributeWidget *widget);
    virtual ~InUrlDatasetsController();

    virtual QWidget * createGUI(U2OpStatus &os);
    virtual QVariant getResult();

private:
    DatasetsController *dsc;
    QList<Dataset> sets;
};

/************************************************************************/
/* DefaultPropertyController */
/************************************************************************/
class LabeledPropertyWidget : public QWidget {
public:
    LabeledPropertyWidget(const QString &labelText, PropertyWidget *widget, QWidget *parent = NULL);

    void setLabelWidth(int width);

private:
    QLabel *label;
};

class DefaultPropertyController : public PropertyWizardController {
public:
    DefaultPropertyController(AttributeWidget *widget, int labelSize);
    virtual ~DefaultPropertyController();

    virtual QWidget * createGUI(U2OpStatus & os);
    virtual QVariant getResult();

private:
    PropertyWidget *propWidget;
    int labelSize;
};

} // U2

#endif // _U2_PROPERTYWIZARDCONTROLLER_H_
