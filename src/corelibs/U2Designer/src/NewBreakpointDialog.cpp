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

#include "ui/ui_NewBreakpointDialog.h"
#include "NewBreakpointDialog.h"

namespace U2 {

NewBreakpointDialog::NewBreakpointDialog(const QStringList &elementsNames, QWidget *parent,
    Qt::WindowFlags f) : QDialog(parent, f), ui(new Ui::NewBreakpointDialog())
{
    ui->setupUi(this);
    ui->elementCombo->addItems(elementsNames);
    connect(ui->okButton, SIGNAL(clicked()), SLOT(sl_announceNewBreakpoint()));
}

NewBreakpointDialog::~NewBreakpointDialog() {
    delete ui;
}

void NewBreakpointDialog::sl_announceNewBreakpoint() {
    emit si_newBreakpointCreated(ui->elementCombo->currentText());
}

} // namespace U2