/**
 * UGENE - Integrated Bioinformatics Tools.
 * Copyright (C) 2008-2014 UniPro <ugene@unipro.ru>
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

#include <QPushButton>
#include <QValidator>

#include <U2Core/L10n.h>
#include <U2Core/U2SafePoints.h>

#include "AddPrimerDialog.h"

namespace U2 {

AddPrimerDialog::AddPrimerDialog(QWidget *parent)
: QDialog(parent)
{
    setupUi(this);
    primerEdit->setValidator(new QRegExpValidator(QRegExp("[acgtACGT]+")));
    connect(primerEdit, SIGNAL(textEdited(const QString &)), SLOT(sl_onPrimerChanged(const QString &)));

    connect(primerEdit, SIGNAL(textChanged(const QString &)), SLOT(sl_validate()));
    connect(nameEdit, SIGNAL(textChanged(const QString &)), SLOT(sl_validate()));

    sl_validate();
}

Primer AddPrimerDialog::getPrimer() const {
    Primer result;
    result.name = nameEdit->text();
    result.sequence = primerEdit->text();
    return result;
}

void AddPrimerDialog::sl_onPrimerChanged(const QString &primerSequence) {
    primerEdit->setText(primerSequence.toUpper());
}

void AddPrimerDialog::sl_validate() {
    CHECK_EXT(!primerEdit->text().isEmpty(), validate(false), );
    CHECK_EXT(!nameEdit->text().isEmpty(), validate(false), );
    validate(true);
}

void AddPrimerDialog::validate(bool isValid) {
    QPushButton *okButton = buttonBox->button(QDialogButtonBox::Ok);
    SAFE_POINT(NULL != okButton, L10N::nullPointerError("OK button"), );
    okButton->setEnabled(isValid);
}

} // U2
