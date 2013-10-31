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
#include "ADVGraphModel.h"

#include <QtGui/QSpinBox>
#include <QtGui/QHBoxLayout>
#include <QtGui/QVBoxLayout>
#include <QtGui/QPushButton>
#include <QtGui/QLabel>
#include <QtGui/QCheckBox>
#include <QtGui/QMessageBox>

#include "GraphLabelsSelectDialog.h"

#define BACKGROUND_COLOR "QPushButton { background-color : %1;}"

namespace U2 {

GraphLabelsSelectDialog::GraphLabelsSelectDialog(int maxWindowSize, QWidget* parent )
:QDialog(parent) {
    stepSpinBox = new QSpinBox(this);
    stepSpinBox->setRange(maxWindowSize/50, maxWindowSize);
    usedIntervalsCheck = new QCheckBox(this);
    QVBoxLayout* mainLayout = new QVBoxLayout();
    QHBoxLayout* buttonsLayout = new QHBoxLayout();
    QHBoxLayout* spinLayout = new QHBoxLayout();
    buttonsLayout->addStretch(10);

    QPushButton* cancelButton = new QPushButton(tr("Cancel"), this);
    QPushButton* okButton = new QPushButton(tr("OK"), this);
    QLabel* spinLabel = new QLabel(tr("Window size"), this);
    buttonsLayout->addWidget(okButton);
    buttonsLayout->addWidget(cancelButton);
    spinLayout->addWidget(spinLabel);
    spinLayout->addWidget(stepSpinBox);
    usedIntervalsCheck->setText(tr("Mark labels only in selected area"));

    mainLayout->addLayout(spinLayout);
    mainLayout->addWidget(usedIntervalsCheck);
    mainLayout->addLayout(buttonsLayout);

    setLayout(mainLayout);
    setWindowTitle(tr("Graph Settings"));
    setWindowIcon(QIcon(":core/images/graphs.png"));

    setSizePolicy(QSizePolicy::Minimum, QSizePolicy::Minimum);
    setMinimumWidth(200);

    connect(cancelButton, SIGNAL(clicked()), SLOT(sl_onCancelClicked()));
    connect(okButton, SIGNAL(clicked()), SLOT(sl_onOkClicked()));

    okButton->setDefault(true);
}
int GraphLabelsSelectDialog::getWindowSize() {
    return stepSpinBox->value();
}
bool GraphLabelsSelectDialog::isUsedIntervals() {
    CHECK(Qt::Checked != usedIntervalsCheck->checkState(), true);
    return false;
}

void GraphLabelsSelectDialog::sl_onCancelClicked() {
    reject();
}

void GraphLabelsSelectDialog::sl_onOkClicked() {
    accept();
    return;
}

} // namespace 
