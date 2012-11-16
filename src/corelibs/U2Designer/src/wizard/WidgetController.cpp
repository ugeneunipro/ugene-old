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

#include "WidgetController.h"

namespace U2 {

WidgetController::WidgetController(WizardController *_wc)
: wc(_wc)
{

}

WidgetController::~WidgetController() {

}

/************************************************************************/
/* LabeledPropertyWidget */
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

} // U2
