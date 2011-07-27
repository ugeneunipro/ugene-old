/**
 * UGENE - Integrated Bioinformatics Tools.
 * Copyright (C) 2008-2011 UniPro <ugene@unipro.ru>
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

#include "PositionSelector.h"

#include <QtGui/QHBoxLayout>
#include <QtGui/QVBoxLayout>
#include <QtGui/QPushButton>
#include <QtGui/QToolButton>
#include <QtGui/QIntValidator>
#include <QtGui/QLabel>
#include <math.h>

namespace U2 {

PositionSelector::PositionSelector(QWidget* p, int s, int e) 
: QWidget(p), rangeStart(s), rangeEnd(e), posEdit(NULL), autoclose(false), dialog(NULL) 
{
    init();

    QToolButton* goButton = new QToolButton(this);
    goButton->setText(tr("Go!"));
    goButton->setToolTip(tr("Go to position"));
    connect(goButton, SIGNAL(clicked(bool)), SLOT(sl_onButtonClicked(bool)));

    layout()->addWidget(goButton);
}

void PositionSelector::init() {
    int w = qMax(((int)log10((double)rangeEnd))*10, 70);
    posEdit = new QLineEdit(this);
    posEdit->setValidator(new QIntValidator(rangeStart, rangeEnd, posEdit));
    if (dialog == NULL) {
        posEdit->setFixedWidth(w);
    } else {
        posEdit->setMinimumWidth(qMax(120, w));
    }
    posEdit->setAlignment(Qt::AlignRight);
    posEdit->setToolTip("Enter position here");
    connect(posEdit, SIGNAL(returnPressed()), SLOT(sl_onReturnPressed()));

    QHBoxLayout* l = new QHBoxLayout(this);
    if (dialog == NULL) {
        l->setContentsMargins(5, 0, 5, 0);
        l->setSizeConstraint(QLayout::SetFixedSize);
    } else {
        l->setMargin(0);
    }

    setLayout(l);
    
    if (dialog!=NULL) {
        QLabel* posLabel = new QLabel(tr("Position"), this);
        posLabel->setSizePolicy(QSizePolicy::Maximum, QSizePolicy::Preferred);
        l->addWidget(posLabel);
    }

    l->addWidget(posEdit);
}

PositionSelector::PositionSelector(QDialog* d, int s, int e, bool _a) 
: QWidget(d), rangeStart(s), rangeEnd(e), posEdit(NULL), autoclose(_a), dialog(d) 
{
    init();

    QPushButton* okButton = new QPushButton(this);
    okButton->setText(tr("Go!"));
    okButton->setDefault(true);
    connect(okButton, SIGNAL(clicked(bool)), SLOT(sl_onButtonClicked(bool)));

    QPushButton* cancelButton = new QPushButton(this);
    cancelButton->setText(tr("Cancel"));
    connect(cancelButton, SIGNAL(clicked()), d, SLOT(reject()));

    QHBoxLayout* l3 = new QHBoxLayout();
    l3->setMargin(0);
    l3->addStretch();
    l3->addWidget(cancelButton);
    l3->addWidget(okButton);

    assert(dialog!=NULL);
    QVBoxLayout* l2 = new QVBoxLayout();
    l2->addWidget(this);
    l2->addStretch();
    l2->addLayout(l3);

    dialog->setLayout(l2);
    dialog->resize(l2->minimumSize());

    //todo: add checkbox to handle 'autoclose' state
}

PositionSelector::~PositionSelector(){ 
}


void PositionSelector::sl_onButtonClicked(bool checked) {
    Q_UNUSED(checked);
    exec();
}

void PositionSelector::sl_onReturnPressed(){
    exec();
}

void PositionSelector::exec() {
    bool ok = false;
    QString text = posEdit->text();
    int v = text.toInt(&ok);
    if (!ok || v < rangeStart || v > rangeEnd) {
        return;
    }

    emit si_positionChanged(v);
    
    if (dialog!=NULL && autoclose) {
        dialog->accept();
    }
}


} //namespace
