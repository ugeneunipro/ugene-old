
#include <QtCore/QFile>

#include <U2Core/AppContext.h>

#include "LicenseDialog.h"
#include "ui_LicenseDialog.h"


namespace U2 {

LicenseDialog::LicenseDialog(Plugin *_plugin, QWidget *parent) :
    QDialog(parent), ui(new Ui::LicenseDialog), plugin(_plugin)
{
    ui->setupUi(this);
    //Opening license file
    QFile licenseFile(plugin->getLicensePath().getURLString());
    if (!licenseFile.open(QIODevice::ReadOnly | QIODevice::Text)){
        ui->licenseTextBrowser->setText(tr("License file not found."));
    }else{
        ui->licenseTextBrowser->setText(QString(licenseFile.readAll()));
        licenseFile.close();
    }

    connect(ui->acceptButton,SIGNAL(clicked()),SLOT(sl_accept()));
}

LicenseDialog::~LicenseDialog()
{
    delete ui;
}

void LicenseDialog::sl_accept(){
    AppContext::getPluginSupport()->setLicenseAccepted(plugin);
    accept();
}
}//namespace
