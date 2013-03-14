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

#include <QtGui/QHBoxLayout>

#include "ConfigurationEditor.h"

namespace U2 {

PropertyWidget::PropertyWidget(QWidget *parent)
    : QWidget(parent)
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

PropertyDelegate::PropertyDelegate(QObject *parent)
: QItemDelegate(parent)
{

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

} // U2
