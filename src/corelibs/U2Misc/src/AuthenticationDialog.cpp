#include <QtGui/QMessageBox>

#include <ui/ui_AuthentificationDialog.h>
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
