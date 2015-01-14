/**
 * UGENE - Integrated Bioinformatics Tools.
 * Copyright (C) 2008-2015 UniPro <ugene@unipro.ru>
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

#include <math.h>

#include <QtCore/qglobal.h>
#if (QT_VERSION < 0x050000) //Qt 5
#include <QtGui/QHBoxLayout>
#include <QtGui/QLabel>
#include <QtGui/QPushButton>
#include <QtGui/QToolButton>
#include <QtGui/QVBoxLayout>
#else
#include <QtWidgets/QHBoxLayout>
#include <QtWidgets/QLabel>
#include <QtWidgets/QPushButton>
#include <QtWidgets/QToolButton>
#include <QtWidgets/QVBoxLayout>
#endif

#include <U2Gui/U2LongLongValidator.h>

#include "PositionSelector.h"

namespace U2 {

PositionSelector::PositionSelector(QWidget* p, qint64 s, qint64 e, bool fixedSize)
: QWidget(p), rangeStart(s), rangeEnd(e), posEdit(NULL), autoclose(false), dialog(NULL) 
{
    init(fixedSize);

    QToolButton* goButton = new QToolButton(this);
    goButton->setText(tr("Go!"));
    goButton->setToolTip(tr("Go to position"));
    goButton->setObjectName("Go!");
    connect(goButton, SIGNAL(clicked(bool)), SLOT(sl_onButtonClicked(bool)));

    layout()->addWidget(goButton);
}

void PositionSelector::init(bool fixedSize) {
    int w = qMax(((int)log10((double)rangeEnd))*10, 70);
    posEdit = new QLineEdit(this);
    posEdit->setObjectName("go_to_pos_line_edit");
    posEdit->setValidator(new U2LongLongValidator(rangeStart, rangeEnd, posEdit));
    if (fixedSize) {
        posEdit->setFixedWidth(w);
    } else {
        posEdit->setMinimumWidth(qMax(120, w));
    }
    posEdit->setAlignment(Qt::AlignRight);
    posEdit->setToolTip("Enter position here");
    connect(posEdit, SIGNAL(returnPressed()), SLOT(sl_onReturnPressed()));

    QHBoxLayout* l = new QHBoxLayout(this);
    if (fixedSize) {
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

PositionSelector::PositionSelector(QDialog* d, qint64 s, qint64 e, bool _a)
: QWidget(d), rangeStart(s), rangeEnd(e), posEdit(NULL), autoclose(_a), dialog(d) 
{
    init(true);

    QPushButton* okButton = new QPushButton(this);
    okButton->setText(tr("Go!"));
    okButton->setObjectName("okButton");
    okButton->setDefault(true);
    connect(okButton, SIGNAL(clicked(bool)), SLOT(sl_onButtonClicked(bool)));

    QPushButton* cancelButton = new QPushButton(this);
    cancelButton->setText(tr("Cancel"));
    cancelButton->setObjectName("cancelButton");
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

    dialog->setLayout(l2);
    dialog->resize(l2->minimumSize());

    //todo: add checkbox to handle 'autoclose' state
}

PositionSelector::~PositionSelector(){ 
}

void PositionSelector::updateRange(qint64 _rangeStart, qint64 _rangeEnd) {
    rangeStart = _rangeStart;
    rangeEnd = _rangeEnd;

    posEdit->setValidator(new U2LongLongValidator(_rangeStart, _rangeEnd, posEdit));

    int width = qMax(((int)log10((double)_rangeEnd))*10, 70);
    if (posEdit->maximumWidth() == posEdit->minimumWidth()) {
        posEdit->setFixedWidth(width);
    } else {
        posEdit->setMinimumWidth(qMax(120, width));
    }

    const QValidator *oldValidator = posEdit->validator();
    posEdit->setValidator(new U2LongLongValidator(_rangeStart, _rangeEnd, posEdit));
    // force the validation
    const QString position = posEdit->text();
    posEdit->clear();
    posEdit->insert(position);

    delete oldValidator;
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
    QString text = posEdit->text().remove(' ').remove(',');
    qint64 v = text.toLongLong(&ok);
    if (!ok || v < rangeStart || v > rangeEnd) {
        return;
    }

    emit si_positionChanged(v);
    
    if (dialog!=NULL && autoclose) {
        dialog->accept();
    }
}


} //namespace
