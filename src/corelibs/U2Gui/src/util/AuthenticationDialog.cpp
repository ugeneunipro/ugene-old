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

#include <QtGui/QMessageBox>

#include "ui/ui_AuthentificationDialog.h"
#include "AuthenticationDialog.h"


namespace U2 {

AuthDialog::AuthDialog( QWidget* parent )
: QDialog(parent)        
{
    ui = new Ui_AuthentificationDialog();
    ui->setupUi(this);
    connect(ui->OKButton, SIGNAL(clicked()), SLOT(sl_onOkButtonClicked()));

}

QString AuthDialog::getUserName() const
{
    return ui->nameEdit->text();
}

QString AuthDialog::getPasswd() const
{
    return ui->passEdit->text();
}

bool AuthDialog::rememberAuthData() const
{
    return ui->rememberBox->isChecked();
}

void AuthDialog::sl_onOkButtonClicked()
{
    if (ui->nameEdit->text().isEmpty() ) {
        QMessageBox::critical( this, tr( "Error!" ), tr("User name is not set.") );
        return; 
    } else  if (ui->passEdit->text().isEmpty()) {
        QMessageBox::critical( this, tr("Error!"), tr("Password is not set.") );
        return; 
    } else {
        accept();
    }
}

AuthDialog::~AuthDialog() 
{
    delete ui;
}

} // namespace
