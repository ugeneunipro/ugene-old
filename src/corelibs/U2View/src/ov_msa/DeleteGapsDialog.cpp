/**
 * UGENE - Integrated Bioinformatics Tools.
 * Copyright (C) 2008-2016 UniPro <ugene@unipro.ru>
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

#include "DeleteGapsDialog.h"
#include <U2Gui/HelpButton.h>
#if (QT_VERSION < 0x050000) //Qt 5
#include <QtGui/QPushButton>
#else
#include <QtWidgets/QPushButton>
#endif


namespace U2 {

DeleteGapsDialog::DeleteGapsDialog(QWidget* parent, int rowNum): QDialog(parent) {
    setupUi(this);
    new HelpButton(this, buttonBox, "17467637");
    buttonBox->button(QDialogButtonBox::Ok)->setText(tr("Remove"));
    buttonBox->button(QDialogButtonBox::Cancel)->setText(tr("Cancel"));

    allRadioButton->setChecked(true);
    absoluteSpinBox->setMinimum(1);
    absoluteSpinBox->setMaximum(rowNum);

    QPushButton *deleteButton = buttonBox->button(QDialogButtonBox::Ok);
    QPushButton *cancelButton = buttonBox->button(QDialogButtonBox::Cancel);

    connect(absoluteRadioButton, SIGNAL(clicked()), SLOT(sl_onRadioButtonClicked()));
    connect(relativeRadioButton, SIGNAL(clicked()), SLOT(sl_onRadioButtonClicked()));
    connect(allRadioButton, SIGNAL(clicked()), SLOT(sl_onRadioButtonClicked()));
    connect(deleteButton, SIGNAL(clicked()), SLOT(sl_onOkClicked()));
    connect(cancelButton, SIGNAL(clicked()), SLOT(sl_onCancelClicked()));

    sl_onRadioButtonClicked();

}

void DeleteGapsDialog::sl_onRadioButtonClicked() {
    absoluteSpinBox->setEnabled(absoluteRadioButton->isChecked());
    relativeSpinBox->setEnabled(relativeRadioButton->isChecked());

    if (absoluteRadioButton->isChecked()) {
        absoluteSpinBox->setFocus();
    }
    if (relativeRadioButton->isChecked()) {
        relativeSpinBox->setFocus();
    }
}

void DeleteGapsDialog::sl_onOkClicked() {
    deleteMode = allRadioButton->isChecked() ? DeleteAll : (relativeRadioButton->isChecked() ? DeleteByRelativeVal : DeleteByAbsoluteVal);

    switch(deleteMode) {
        case DeleteByAbsoluteVal: value = absoluteSpinBox->value();
            break;
        case DeleteByRelativeVal: value = relativeSpinBox->value();
            break;
        default: value = 0;
    }

    accept();
}

void DeleteGapsDialog::sl_onCancelClicked() {
    reject();
}

}
