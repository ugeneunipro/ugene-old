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

#include <U2Core/AppContext.h>
#include <U2Core/CredentialsStorage.h>
#include <U2Core/Settings.h>

#include "util/AuthenticationWidget.h"
#include "EditConnectionDialog.h"
#include "ui_EditConnectionDialog.h"

namespace U2 {

EditConnectionDialog::EditConnectionDialog(QWidget *parent, const U2DbiId& dbiUrl, const QString &name) :
    QDialog(parent),
    ui(new Ui::EditConnectionDialog)
{
    ui->setupUi(this);
    adjustSize();
    init(dbiUrl, name);
}

EditConnectionDialog::~EditConnectionDialog() {
    delete ui;
}

QString EditConnectionDialog::getName() const {
    if (!ui->leName->text().isEmpty()) {
        return ui->leName->text();
    } else {
        return getDbUrl();
    }
}

U2DbiId EditConnectionDialog::getDbUrl() const {
    return U2DbiUtils::createDbiUrl(ui->leHost->text(),
                                    ui->lePort->text().toInt(),
                                    ui->leDatabase->text());
}


void EditConnectionDialog::accept() {
    if (!checkFields()) {
        return;
    }

    if (!ui->authenticationWidget->getLogin().isEmpty()) {
        saveCredentials();
    } else {
        removeCredentials();
    }

    QDialog::accept();
}

void EditConnectionDialog::init(const U2DbiId& dbUrl, const QString& name) {
    initTabOrder();

    ui->leName->setText(name);

    if (dbUrl.isNull()) {
        return;
    }

    QString host;
    int port = -1;
    QString dbName;

    U2DbiUtils::parseDbiUrl(dbUrl, host,port, dbName);
    ui->leHost->setText(host);
    if (port > 0) {
        ui->lePort->setText(QString::number(port));
    }
    ui->leDatabase->setText(dbName);

    Credentials credentials = AppContext::getCredentialsStorage()->getEntry(dbUrl);
    if (credentials.isValid()) {
        ui->authenticationWidget->setLogin(credentials.login);
        ui->authenticationWidget->setPassword(credentials.password);
        ui->authenticationWidget->setRemembered(AppContext::getCredentialsStorage()->isRemembered(dbUrl));
    }
}

void EditConnectionDialog::initTabOrder() {
    setTabOrder(ui->leName, ui->leHost);
    setTabOrder(ui->leHost, ui->lePort);
    setTabOrder(ui->lePort, ui->leDatabase);
    setTabOrder(ui->leDatabase, ui->authenticationWidget->leLogin);
    setTabOrder(ui->authenticationWidget->leLogin, ui->authenticationWidget->lePassword);
    setTabOrder(ui->authenticationWidget->lePassword, ui->authenticationWidget->cbRemember);
    setTabOrder(ui->authenticationWidget->cbRemember, ui->buttonBox);
}

void EditConnectionDialog::saveCredentials() const {
    const Credentials credentials(ui->authenticationWidget->getLogin(), ui->authenticationWidget->getPassword());
    const bool remember = ui->authenticationWidget->isRemembered();
    AppContext::getCredentialsStorage()->addEntry(getDbUrl(), credentials, remember);
}

void EditConnectionDialog::removeCredentials() const {
    AppContext::getCredentialsStorage()->removeEntry(getDbUrl());
}

bool EditConnectionDialog::checkFields() {
    if (ui->leHost->text().isEmpty()) {
        QMessageBox::critical(this, tr("Error"), tr("Host is not set"));
        ui->leHost->setFocus();
        return false;
    }

    if (ui->leDatabase->text().isEmpty()) {
        QMessageBox::critical(this, tr("Error"), tr("Database is not set"));
        ui->leDatabase->setFocus();
        return false;
    }

    return true;
}

}   // namespace U2
