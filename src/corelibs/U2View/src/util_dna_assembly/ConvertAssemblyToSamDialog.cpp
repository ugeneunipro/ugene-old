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

#include "ConvertAssemblyToSamDialog.h"
#include <ui/ui_AssemblyToSamDialog.h>

#include <U2Core/GUrlUtils.h>
#include <U2Core/DocumentUtils.h>

#include <U2Gui/LastUsedDirHelper.h>

#include <QtGui/QFileDialog>
#include <QtGui/QMessageBox>



namespace U2 {

GUrl ConvertAssemblyToSamDialog::dbFileUrl;

ConvertAssemblyToSamDialog::ConvertAssemblyToSamDialog(QWidget* parent, QString dbPath)
: QDialog(parent)
{
    ui = new Ui_AssemblyToSamDialog;
    ui->setupUi(this);

    connect(ui->setDbPathButton, SIGNAL(clicked()), SLOT(sl_onSetDbPathButtonClicked()));
    connect(ui->setSamPathButton, SIGNAL(clicked()), SLOT(sl_onSetSamPathButtonClicked()));
    connect(ui->convertButton, SIGNAL(clicked()), SLOT(accept()));
    connect(ui->cancelButton, SIGNAL(clicked()), SLOT(reject()));

    if ("" != dbPath) {
        ui->dbPathEdit->setText(dbPath);
        ui->dbPathEdit->setReadOnly(true);
        ui->setDbPathButton->setEnabled(false);
        buildSamUrl(dbPath);
        return;
    }

    if (!dbFileUrl.isEmpty()) {
        ui->dbPathEdit->setText(dbFileUrl.getURLString());
        buildSamUrl(dbFileUrl);
    }
}

void ConvertAssemblyToSamDialog::accept() {
    if (ui->dbPathEdit->text().isEmpty()) {
        QMessageBox::information(this, tr("Data base to SAM converter"),
            tr("Data base file url is not set!") );
    } else if (ui->samPathEdit->text().isEmpty()) {
        QMessageBox::information(this, tr("Data base to SAM converter"),
            tr("SAM file url is not set!") );
    } else {
        if (ui->setDbPathButton->isEnabled()) {
            dbFileUrl = getDbFileUrl();
        }
        QDialog::accept();
    }
}

void ConvertAssemblyToSamDialog::reject() {
    if (ui->setDbPathButton->isEnabled() && !ui->dbPathEdit->text().isEmpty()) {
        dbFileUrl = getDbFileUrl();
    }
    QDialog::reject();
}

void ConvertAssemblyToSamDialog::buildSamUrl(const GUrl &dbUrl) {
    GUrl url = GUrlUtils::rollFileName(dbUrl.dirPath() + "/" + dbUrl.baseFileName()+ ".sam", DocumentUtils::getNewDocFileNameExcludesHint());
    ui->samPathEdit->setText(url.getURLString());
}

void ConvertAssemblyToSamDialog::sl_onSetDbPathButtonClicked() {
    LastUsedDirHelper lod;
    QString filter;

    lod.url = QFileDialog::getOpenFileName(this, tr("Open an assembly data base file"), lod.dir, filter);
    if (lod.url.isEmpty()) {
        return;
    }

    ui->dbPathEdit->setText(lod.url);
    buildSamUrl(lod.url);
}

void ConvertAssemblyToSamDialog::sl_onSetSamPathButtonClicked() {
    LastUsedDirHelper lod;
    lod.url = QFileDialog::getSaveFileName(this, tr("Set a result SAM file name"), lod.dir);
    if (!lod.url.isEmpty()) {
        GUrl result = lod.url;
        if (result.lastFileSuffix().isEmpty()) {
            result = QString( "%1.sam" ).arg( result.getURLString() );
        }
        ui->samPathEdit->setText(result.getURLString());
    }
}

const GUrl ConvertAssemblyToSamDialog::getDbFileUrl() {
    return ui->dbPathEdit->text();
}

const GUrl ConvertAssemblyToSamDialog::getSamFileUrl() {
    return ui->samPathEdit->text();
}

} // U2
