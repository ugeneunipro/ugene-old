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

#include "GTSpinBox.h"
#include "GTWidget.h"

#include "GTMouseDriver.h"
#include "GTKeyboardDriver.h"

namespace U2 {

#define GT_CLASS_NAME "GTSpinBox"

#define GT_METHOD_NAME "setValue"
void GTSpinBox::setValue(U2OpStatus& os, QSpinBox *spinBox, int v, GTGlobals::UseMethod useMethod) {
    GT_CHECK(spinBox != NULL, "spinBox is NULL");
    GT_CHECK(v <= spinBox->maximum(), QString("value for this spinbox cannot be more then %1").arg(spinBox->maximum()));
    GT_CHECK(v >= spinBox->minimum(), QString("value for this spinbox cannot be less then %1").arg(spinBox->minimum()));

    QPoint arrowPos;
    QRect spinBoxRect;
    int key;

    GT_CHECK(spinBox->isEnabled(), "SpinBox is not enabled");

    if (spinBox->value() != v) {
        switch(useMethod) {
        case GTGlobals::UseMouse:
            spinBoxRect = spinBox->rect();
            if (v > spinBox->value()) {
                arrowPos = QPoint(spinBoxRect.right() - 5, spinBoxRect.height() / 4); // -5 it's needed that area under cursor was clickable
            } else {
                arrowPos = QPoint(spinBoxRect.right() - 5, spinBoxRect.height() * 3 / 4);
            }

            GTMouseDriver::moveTo(os, spinBox->mapToGlobal(arrowPos));
            while (spinBox->value() != v) {
                GTMouseDriver::click(os);
                GTGlobals::sleep(100);
            }
            break;

        case GTGlobals::UseKey:
            if (v > spinBox->value()) {
                key = GTKeyboardDriver::key["up"];
            } else {
                key = GTKeyboardDriver::key["down"];
            }

            GTWidget::setFocus(os, spinBox);
            while (spinBox->value() != v) {
                GTKeyboardDriver::keyClick(os, key);
                GTGlobals::sleep(100);
            }
            break;

        case GTGlobals::UseKeyBoard:
            QString s = QString::number(v);
            GTWidget::setFocus(os, spinBox);
            GTGlobals::sleep(100);
            GTKeyboardDriver::keyClick(os, 'a', GTKeyboardDriver::key["ctrl"]);
            GTGlobals::sleep(100);
            GTKeyboardDriver::keyClick(os, GTKeyboardDriver::key["delete"]);
            GTGlobals::sleep(100);
            GTKeyboardDriver::keySequence(os,s);
            GTGlobals::sleep(100);
        }
    }

    int currIndex = spinBox->value();
    GT_CHECK(currIndex == v, "Can't set index");
}
#undef GT_METHOD_NAME

#undef GT_CLASS_NAME

}
