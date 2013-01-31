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

#include <QLayout>
#include <QFileDialog>

#include <U2Core/U2SafePoints.h>

#include <U2Lang/URLContainer.h>
#include <U2Lang/WorkflowUtils.h>
#include <U2Gui/LastUsedDirHelper.h>

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
    connect(lineEdit, SIGNAL(textChanged(const QString &)), SLOT(sl_valueChanged(const QString &)));
}

QVariant DefaultPropertyWidget::value() {
    return lineEdit->text();
}

void DefaultPropertyWidget::setValue(const QVariant &value) {
    lineEdit->setText(value.toString());
}

void DefaultPropertyWidget::sl_valueChanged(const QString &value) {
    emit si_valueChanged(value);
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

    connect(spinBox, SIGNAL(valueChanged(int)), SLOT(sl_valueChanged(int)));
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

void SpinBoxWidget::sl_valueChanged(int value) {
    emit valueChanged(value);
    emit si_valueChanged(value);
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

    connect(spinBox, SIGNAL(valueChanged(double)), SLOT(sl_valueChanged(double)));
}

QVariant DoubleSpinBoxWidget::value() {
    spinBox->interpretText();
    return spinBox->value();
}

void DoubleSpinBoxWidget::setValue(const QVariant &value) {
    spinBox->setValue(value.toDouble());
}

void DoubleSpinBoxWidget::sl_valueChanged(double value) {
    emit si_valueChanged(value);
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
    connect(comboBox, SIGNAL(currentIndexChanged(int)),
        this, SLOT(sl_valueChanged(int)));
}

QVariant ComboBoxWidget::value() {
    return comboBox->itemData(comboBox->currentIndex());
}

void ComboBoxWidget::setValue(const QVariant &value) {
    int idx = comboBox->findData(value);
    comboBox->setCurrentIndex(idx);
}

void ComboBoxWidget::sl_valueChanged(int) {
    emit si_valueChanged(value());
}

ComboBoxWidget * ComboBoxWidget::createBooleanWidget(QWidget *parent) {
    QVariantMap values;
    values[ComboBoxWidget::tr("False")] = false;
    values[ComboBoxWidget::tr("True")] = true;
    return new ComboBoxWidget(values, parent);
}

/************************************************************************/
/* ComboBoxWithUrlWidget */
/************************************************************************/
ComboBoxWithUrlWidget::ComboBoxWithUrlWidget(const QVariantMap &items, QWidget *parent)
: PropertyWidget(parent)
, customIdx(-1)
{
    comboBox = new QComboBox(this);
    addMainWidget(comboBox);

    foreach (const QString &key, items.keys()) {
        comboBox->addItem(key, items[key]);
    }

    QToolButton * toolButton = new QToolButton(this);
    toolButton->setText("...");
    toolButton->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Preferred);
    connect(toolButton, SIGNAL(clicked()), SLOT(sl_browse()));
    layout()->addWidget(toolButton);

    connect(comboBox, SIGNAL(activated(const QString &)),
        this, SIGNAL(valueChanged(const QString &)));
    connect(comboBox, SIGNAL(currentIndexChanged(int)),
        this, SLOT(sl_valueChanged(int)));
}

QVariant ComboBoxWithUrlWidget::value() {
    return comboBox->itemData(comboBox->currentIndex());
}

void ComboBoxWithUrlWidget::setValue(const QVariant &value) {
    int idx = comboBox->findData(value);
    if (idx == -1){
        if (customIdx == -1){
            comboBox->addItem(value.toString(), value);
            customIdx = comboBox->findData(value);
        }else{
            comboBox->setItemText(customIdx, value.toString());
            comboBox->setItemData(customIdx, value);
        }
        comboBox->setCurrentIndex(customIdx);
    }else{
        idx = comboBox->findData(value);
        comboBox->setCurrentIndex(idx);
    }
}

void ComboBoxWithUrlWidget::sl_valueChanged(int) {
    emit si_valueChanged(value());
}

void ComboBoxWithUrlWidget::sl_browse(){
    LastUsedDirHelper lod("UrlCombo");
    QString lastDir = lod.dir;

    QString name;
    lod.url = name = QFileDialog::getOpenFileName(NULL, tr("Select a file"), lastDir);
    if (!name.isEmpty()) {
        setValue(name);
    }
    comboBox->setFocus();
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
    connect(urlLine, SIGNAL(textChanged(const QString &)), SLOT(sl_textChanged(const QString &)));
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
        layout()->addWidget(addButton);
    }
}

void URLWidget::sl_textChanged(const QString &text) {
    emit si_valueChanged(text);
    if (!urlLine->isMulti()) {
        return;
    }

    addButton->setVisible(!text.isEmpty());
}

QVariant URLWidget::value() {
    return urlLine->text();
}

void URLWidget::setValue(const QVariant &value) {
    QString urlString;
    if (value.canConvert< QList<Dataset> >()) {
        QStringList urls;
        foreach (const Dataset &set, value.value< QList<Dataset> >()) {
            foreach (URLContainer *c, set.getUrls()) {
                urls << c->getUrl();
            }
        }
        urlString = urls.join(";");
    } else {
        urlString = value.toString();
    }
    urlLine->setText(urlString);
}

} // U2
