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

#include "TreeSettingsDialog.h"
#include <U2Core/global.h>

namespace U2 {

int TreeSettings::default_width_coef = 1;
int TreeSettings::default_height_coef = 1;

TreeSettings::TreeSettings() {
    width_coef = default_width_coef;
    height_coef = default_height_coef;
    type = PHYLOGRAM;
}

TreeSettingsDialog::TreeSettingsDialog(QWidget *parent, const TreeSettings &treeSettings, bool isRectLayout)
: QDialog(parent), settings(treeSettings), changedSettings(treeSettings) {

    setupUi(this);

    heightSlider->setValue(settings.height_coef);
    widthlSlider->setValue(settings.width_coef);

    heightSlider->setEnabled(isRectLayout);

    treeViewCombo->addItem(treePhylogramText());
    treeViewCombo->addItem(treeCladogramText());

    switch ( settings.type )
    {
    case TreeSettings::PHYLOGRAM:
        treeViewCombo->setCurrentIndex(treeViewCombo->findText(treePhylogramText()));
        break;
    case TreeSettings::CLADOGRAM:
        treeViewCombo->setCurrentIndex(treeViewCombo->findText(treeCladogramText()));
        break;
    default:
        assert(false && "Unexpected tree type value.");
        break;
    }
    
}

void TreeSettingsDialog::accept() {
    changedSettings.height_coef = heightSlider->value();
    changedSettings.width_coef = widthlSlider->value();

    if (treeViewCombo->currentText() == treePhylogramText())
    {
        changedSettings.type = TreeSettings::PHYLOGRAM;
    } else if (treeViewCombo->currentText() == treeCladogramText()) {
        changedSettings.type = TreeSettings::CLADOGRAM;
    } else {
        assert(false && "Unexpected tree type value");
    }

    settings = changedSettings;
    QDialog::accept();
}

TreeSettings TreeSettingsDialog::getSettings() const {

    return settings;
}

} //namespace
