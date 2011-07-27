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

#include "RangeSelector.h"

#include <QtGui/QHBoxLayout>
#include <QtGui/QVBoxLayout>
#include <QtGui/QToolButton>
#include <QtGui/QPushButton>
#include <QtGui/QLabel>
#include <QtGui/QIntValidator>
#include <math.h>

namespace U2 {

RangeSelector::RangeSelector(QWidget* p, int s, int e) 
: QWidget(p), rangeStart(s), rangeEnd(e), startEdit(NULL), endEdit(NULL), dialog(NULL), autoclose(false) 
{
    init();
    
    QToolButton* goButton = new QToolButton(this);
    goButton->setText(tr("Ok"));
    connect(goButton, SIGNAL(clicked(bool)), SLOT(sl_onGoButtonClicked(bool)));

    layout()->addWidget(goButton);
}

void RangeSelector::init() {
    int w = qMax(((int)log10((double)rangeEnd))*10, 70);

    startEdit = new QLineEdit(this);
    startEdit->setValidator(new QIntValidator(1, len, startEdit));
    if (dialog == NULL) {
        startEdit->setFixedWidth(w);
    } else {
        startEdit->setMinimumWidth(w);
    }

    startEdit->setAlignment(Qt::AlignRight);
    startEdit->setText(QString::number(rangeStart));
    connect(startEdit, SIGNAL(returnPressed()), SLOT(sl_onReturnPressed()));

    endEdit = new QLineEdit(this);
    endEdit->setValidator(new QIntValidator(1, len, startEdit));
    if (dialog == NULL) {
        endEdit->setFixedWidth(w);
    } else {
        endEdit->setMinimumWidth(w);
    }

    endEdit->setAlignment(Qt::AlignRight);
    endEdit->setText(QString::number(rangeEnd));
    connect(endEdit, SIGNAL(returnPressed()), SLOT(sl_onReturnPressed()));

    QToolButton* minButton = new QToolButton(this);
    minButton->setText(tr("Min"));
    connect(minButton, SIGNAL(clicked(bool)), SLOT(sl_onMinButtonClicked(bool)));

    QToolButton* maxButton = new QToolButton(this);
    maxButton->setText(tr("Max"));
    connect(maxButton, SIGNAL(clicked(bool)), SLOT(sl_onMaxButtonClicked(bool)));

    if (dialog != NULL) {
        minButton->setShortcut(QKeySequence(Qt::ALT | Qt::Key_Z));
        maxButton->setShortcut(QKeySequence(Qt::ALT | Qt::Key_X));
    }

    QHBoxLayout* l = new QHBoxLayout(this);
    if (dialog == NULL) {
        l->setContentsMargins(5, 0, 5, 0);
        l->setSizeConstraint(QLayout::SetFixedSize);
    } else {
        l->setMargin(0);
    }

    setLayout(l);

    QLabel* rangeLabel = new QLabel(tr("Range:"), this);
    rangeLabel->setSizePolicy(QSizePolicy::Maximum, QSizePolicy::Preferred);

    startEdit->setObjectName("start_edit_line");
    endEdit->setObjectName("end_edit_line");
    minButton->setObjectName("min_val_button");
    maxButton->setObjectName("max_val_button");
    this->setObjectName("range_selector");

    l->addWidget(rangeLabel);
    l->addWidget(minButton);
    l->addWidget(startEdit);
    l->addWidget(new QLabel(tr("-"), this));
    l->addWidget(endEdit);
    l->addWidget(maxButton);
}

RangeSelector::RangeSelector(QDialog* d, int s, int e, int _len, bool a) 
: QWidget(d), rangeStart(s), rangeEnd(e), len(_len), startEdit(NULL), endEdit(NULL), dialog(d), autoclose(a)
{
    init();

    QPushButton* okButton = new QPushButton(this);
    okButton->setText(tr("Ok"));
    okButton->setDefault(true);
    okButton->setObjectName("ok_button");
    connect(okButton, SIGNAL(clicked(bool)), SLOT(sl_onGoButtonClicked(bool)));

    QPushButton* cancelButton = new QPushButton(this);
    cancelButton->setText(tr("Cancel"));
    cancelButton->setObjectName("cancel_button");
    connect(cancelButton, SIGNAL(clicked()), d, SLOT(reject()));

    QHBoxLayout* l3 = new QHBoxLayout();
    l3->setMargin(0);
    l3->addStretch();
    l3->addWidget(okButton);
    l3->addWidget(cancelButton);

    assert(dialog!=NULL);
    QVBoxLayout* l2 = new QVBoxLayout();
    l2->addWidget(this);
    l2->addStretch();
    l2->addLayout(l3);

    dialog->setObjectName("range_selection_dialog");
    dialog->setLayout(l2);
    dialog->resize(l2->minimumSize());
}


RangeSelector::~RangeSelector(){ 
//    printf("DELETED\n");
}


void RangeSelector::sl_onGoButtonClicked(bool checked) {
    Q_UNUSED(checked);
    exec();
}

void RangeSelector::sl_onReturnPressed() {
    exec();
}

void RangeSelector::exec() {
    bool ok = false;
    int v1 = startEdit->text().toInt(&ok);
    if (!ok || v1 < 1 || v1 > len) {
        return;
    }
    int v2 = endEdit->text().toInt(&ok);
    if (!ok || v2 < v1 || v2 > len) {
        return;
    }

    emit si_rangeChanged(v1, v2);

    if (dialog!=NULL && autoclose) {
        dialog->accept();
    }
}

void RangeSelector::sl_onMinButtonClicked(bool checked) {
    Q_UNUSED(checked);
    startEdit->setText(QString::number(1));
}

void RangeSelector::sl_onMaxButtonClicked(bool checked) {
    Q_UNUSED(checked);
    endEdit->setText(QString::number(len));
}


int RangeSelector::getStart() const {
    bool ok = false;
    int v = startEdit->text().toInt(&ok);
    assert(ok);
    return v;
}

int RangeSelector::getEnd() const {
    bool ok = false;
    int v = endEdit->text().toInt(&ok);
    assert(ok);
    return v;
}

} //namespace
