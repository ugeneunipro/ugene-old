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

#include <QLayout>

#include <U2Core/U2SafePoints.h>

#include <U2Lang/WorkflowUtils.h>

#include "PropertyWidget.h"

namespace U2 {

/************************************************************************/
/* DefaultPropertyWidget */
/************************************************************************/
DefaultPropertyWidget::DefaultPropertyWidget(int maxLength, QWidget *parent)
: PropertyWidget(parent)
{
    lineEdit = new QLineEdit(this);
    if (maxLength >= 0) {
        lineEdit->setMaxLength(maxLength);
    }
    addMainWidget(lineEdit);
}

QVariant DefaultPropertyWidget::value() {
    return lineEdit->text();
}

void DefaultPropertyWidget::setValue(const QVariant &value) {
    lineEdit->setText(value.toString());
}

/************************************************************************/
/* SpinBoxWidget */
/************************************************************************/
SpinBoxWidget::SpinBoxWidget(const QVariantMap &spinProperties, QWidget *parent)
: PropertyWidget(parent)
{
    spinBox = new QSpinBox(this);
    WorkflowUtils::setQObjectProperties(*spinBox, spinProperties);
    addMainWidget(spinBox);

    connect(spinBox, SIGNAL(valueChanged(int)), SIGNAL(valueChanged(int)));
}

QVariant SpinBoxWidget::value() {
    spinBox->interpretText();
    return spinBox->value();
}

void SpinBoxWidget::setValue(const QVariant &value) {
    spinBox->setValue(value.toInt());
}

bool SpinBoxWidget::setProperty(const char *name, const QVariant &value) {
    return spinBox->setProperty(name, value);
}

/************************************************************************/
/* DoubleSpinBoxWidget */
/************************************************************************/
DoubleSpinBoxWidget::DoubleSpinBoxWidget(const QVariantMap &spinProperties, QWidget *parent)
: PropertyWidget(parent)
{
    spinBox = new QDoubleSpinBox(this);
    WorkflowUtils::setQObjectProperties(*spinBox, spinProperties);
    addMainWidget(spinBox);
}

QVariant DoubleSpinBoxWidget::value() {
    spinBox->interpretText();
    return spinBox->value();
}

void DoubleSpinBoxWidget::setValue(const QVariant &value) {
    spinBox->setValue(value.toDouble());
}

/************************************************************************/
/* ComboBoxWidget */
/************************************************************************/
ComboBoxWidget::ComboBoxWidget(const QVariantMap &items, QWidget *parent)
: PropertyWidget(parent)
{
    comboBox = new QComboBox(this);
    addMainWidget(comboBox);

    foreach (const QString &key, items.keys()) {
        comboBox->addItem(key, items[key]);
    }
    connect(comboBox, SIGNAL(activated(const QString &)),
        this, SIGNAL(valueChanged(const QString &)));
}

QVariant ComboBoxWidget::value() {
    return comboBox->itemData(comboBox->currentIndex());
}

void ComboBoxWidget::setValue(const QVariant &value) {
    int idx = comboBox->findData(value);
    comboBox->setCurrentIndex(idx);
}

/************************************************************************/
/* URLWidget */
/************************************************************************/
URLWidget::URLWidget(URLLineEdit *_urlLine, QWidget *parent)
: PropertyWidget(parent), urlLine(_urlLine)
{
    SAFE_POINT(NULL != urlLine, "NULL url line widget", );
    urlLine->setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Preferred);
    connect(urlLine, SIGNAL(si_finished()), SIGNAL(finished()));
    addMainWidget(urlLine);

    QToolButton * toolButton = new QToolButton(this);
    toolButton->setText("...");
    toolButton->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Preferred);
    connect(toolButton, SIGNAL(clicked()), urlLine, SLOT(sl_onBrowse()));
    layout()->addWidget(toolButton);

    if (urlLine->isMulti()) {
        addButton = new QToolButton(this);
        addButton->setVisible(!urlLine->text().isEmpty());
        addButton->setText(tr("add"));
        addButton->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Preferred);
        connect(addButton, SIGNAL(clicked()), urlLine, SLOT(sl_onBrowseWithAdding()));
        connect(urlLine, SIGNAL(textChanged(const QString &)), SLOT(sl_textChanged(const QString &)));
        layout()->addWidget(addButton);
    }
}

void URLWidget::sl_textChanged(const QString &text) {
    if (!urlLine->isMulti()) {
        return;
    }

    addButton->setVisible(!text.isEmpty());
}

QVariant URLWidget::value() {
    return urlLine->text();
}

void URLWidget::setValue(const QVariant &value) {
    urlLine->setText(value.toString());
}

} // U2
