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

#include "ButtonSettingsDialog.h"
#include <QtGui/QColorDialog>

namespace U2 {

QColor ButtonSettings::defaultColor = QColor(0, 0, 0);
int ButtonSettings::defaultRadius = 2;

ButtonSettings::ButtonSettings() {

    col = defaultColor;
    radius = defaultRadius;
}

ButtonSettingsDialog::ButtonSettingsDialog(QWidget *parent, const ButtonSettings &buttonSettings)
: QDialog(parent), settings(buttonSettings), changedSettings(buttonSettings) {

    setupUi(this);

    radiusSpinBox->setValue(settings.radius);
    updateColorButton();

    connect(colorButton, SIGNAL(clicked()), SLOT(sl_colorButton()));
}

void ButtonSettingsDialog::updateColorButton() {

    static const QString COLOR_STYLE("QPushButton { background-color : %1;}");
    colorButton->setStyleSheet(COLOR_STYLE.arg(changedSettings.col.name()));
}

void ButtonSettingsDialog::sl_colorButton() {

    QColor newColor = QColorDialog::getColor(changedSettings.col, this);
    if (newColor.isValid()) {
        changedSettings.col = newColor;
        updateColorButton();
    }
}

void ButtonSettingsDialog::accept() {

    changedSettings.radius = radiusSpinBox->value();

    settings = changedSettings;
    QDialog::accept();
}

ButtonSettings ButtonSettingsDialog::getSettings() const {

    return settings;
}

} //namespace
