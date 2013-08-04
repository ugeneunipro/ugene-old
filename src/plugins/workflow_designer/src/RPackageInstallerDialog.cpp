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

#include "RPackageInstallerDialog.h"

#include <QtGui/QFileDialog>
#include <QtGui/QPushButton>

namespace U2 {

RPackageInstallerDialog::RPackageInstallerDialog(QWidget *parent) : QDialog(parent) {
    setupUi(this);
    connect(selectFileToolButton, SIGNAL(clicked()), SLOT(sl_selectFileButtonClicked()));
    connect(doNotRemindCheckBox, SIGNAL(clicked(bool)), SLOT(sl_doNotDisturbCheckBoxClicked(bool)));
}

void RPackageInstallerDialog::sl_selectFileButtonClicked() {
    QString newPath = QFileDialog::getOpenFileName(this, "", "", "installRPackages.R", NULL);
    if (!newPath.isEmpty()) {
        filePathLineEdit->setText(newPath);
    }
}

void RPackageInstallerDialog::sl_doNotDisturbCheckBoxClicked(bool state) {
    filePathLineEdit->setEnabled(!state);
    selectFileToolButton->setEnabled(!state);
    buttonBox->button(QDialogButtonBox::Ok)->setEnabled(!state);
}

} // U2
