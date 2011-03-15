/**
 * UGENE - Integrated Bioinformatics Tools.
 * Copyright (C) 2008-2011 UniPro <ugene@unipro.ru>
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

#include "BranchSettingsDialog.h"
#include <QtGui/QColorDialog>

namespace U2 {

BranchSettingsDialog::BranchSettingsDialog(QWidget *parent, const BranchSettings &branchSettings)
: QDialog(parent), settings(branchSettings), changedSettings(branchSettings) {

    setupUi(this);

    thicknessSpinBox->setValue(settings.branchThickness);
    updateColorButton();

    connect(colorButton, SIGNAL(clicked()), SLOT(sl_colorButton()));
}

void BranchSettingsDialog::updateColorButton() {

    static const QString COLOR_STYLE("QPushButton { background-color : %1;}");
    colorButton->setStyleSheet(COLOR_STYLE.arg(changedSettings.branchColor.name()));
}

void BranchSettingsDialog::sl_colorButton() {

    QColor newColor = QColorDialog::getColor(changedSettings.branchColor, this);
    if (newColor.isValid()) {
        changedSettings.branchColor = newColor;
        updateColorButton();
    }
}

void BranchSettingsDialog::accept() {

    changedSettings.branchThickness = thicknessSpinBox->value();

    settings = changedSettings;
    QDialog::accept();
}

BranchSettings BranchSettingsDialog::getSettings() const {

    return settings;
}

} //namespace
