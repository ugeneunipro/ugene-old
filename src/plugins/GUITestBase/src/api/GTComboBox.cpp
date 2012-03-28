/**
 * UGENE - Integrated Bioinformatics Tools.
 * Copyright (C) 2008-2012 UniPro <ugene@unipro.ru>
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

#include "GTComboBox.h"
#include "GTWidget.h"

#include "GTMouseDriver.h"
#include "GTKeyboardDriver.h"

namespace U2 {

void GTComboBox::setCurrentIndex(U2OpStatus& os, QComboBox *comboBox, int index) {

    CHECK_SET_ERR(comboBox != NULL, "QComboBox* == NULL");

    int comboCount = comboBox->count();
    CHECK_SET_ERR(index>=0 && index<comboCount, "invalid index");

    int currIndex = comboBox->currentIndex();
    QString directionKey = index > currIndex ? "down" : "up";

    GTWidget::setFocus(os, comboBox);
    int pressCount = qAbs(index-currIndex);
    for (int i=0; i<pressCount; i++) {
        GTKeyboardDriver::keyClick(os, GTKeyboardDriver::key[directionKey]);
        GTGlobals::sleep(100);
    }
    GTKeyboardDriver::keyClick(os, GTKeyboardDriver::key["enter"]);
    GTGlobals::sleep(500);
}

}
