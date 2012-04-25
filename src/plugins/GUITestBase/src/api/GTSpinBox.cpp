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

#include "GTSpinBox.h"
#include "GTWidget.h"

#include "GTMouseDriver.h"
#include "GTKeyboardDriver.h"

namespace U2 {

#define GT_CLASS_NAME "GTSpinBox"

#define GT_METHOD_NAME "setValue"
void GTSpinBox::setValue(U2OpStatus& os, QSpinBox *spinBox, int v) {
    GT_CHECK(spinBox != NULL, "spinBox is NULL");

    spinBox->setValue(v);
    // TODO: set value through GUI

    int currIndex = spinBox->value();
    GT_CHECK(currIndex == v, "Can't set index");
}
#undef GT_METHOD_NAME

#undef GT_CLASS_NAME

}
