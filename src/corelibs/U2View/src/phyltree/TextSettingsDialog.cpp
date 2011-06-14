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

#include "TextSettingsDialog.h"
#include "U2View/TreeViewerUtils.h"
#include <QtGui/QColorDialog>

namespace U2 {

QColor TextSettings::defaultColor = QColor(Qt::gray);
QFont TextSettings::defaultFont = TreeViewerUtils::getFont();

TextSettings::TextSettings() {

    textColor = defaultColor;
    textFont = defaultFont;
}

TextSettingsDialog::TextSettingsDialog(QWidget *parent, const TextSettings &textSettings)
: QDialog(parent), settings(textSettings), changedSettings(textSettings) {

    setupUi(this);

    updateColorButton();
    fontComboBox->setCurrentFont(settings.textFont);
    sizeSpinBox->setValue(settings.textFont.pointSize());

    boldToolButton->setChecked(settings.textFont.bold());
    italicToolButton->setChecked(settings.textFont.italic());
    underlineToolButton->setChecked(settings.textFont.underline());
    overlineToolButton->setChecked(settings.textFont.overline());

    overlineToolButton->setVisible(false);

    connect(colorButton, SIGNAL(clicked()), SLOT(sl_colorButton()));
}

void TextSettingsDialog::updateColorButton() {

    static const QString COLOR_STYLE("QPushButton { background-color : %1;}");
    colorButton->setStyleSheet(COLOR_STYLE.arg(changedSettings.textColor.name()));
}

void TextSettingsDialog::sl_colorButton() {

    QColor newColor = QColorDialog::getColor(changedSettings.textColor, this);
    if (newColor.isValid()) {
        changedSettings.textColor = newColor;
        updateColorButton();
    }
}

void TextSettingsDialog::accept() {

    changedSettings.textFont = fontComboBox->currentFont();
    changedSettings.textFont.setPointSize(sizeSpinBox->value());

    changedSettings.textFont.setBold(boldToolButton->isChecked());
    changedSettings.textFont.setItalic(italicToolButton->isChecked());
    changedSettings.textFont.setUnderline(underlineToolButton->isChecked());
    changedSettings.textFont.setOverline(overlineToolButton->isChecked());

    settings = changedSettings;
    QDialog::accept();
}

TextSettings TextSettingsDialog::getSettings() const {

    return settings;
}

} //namespace
