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

#include "GTDoubleSpinBox.h"
#include "GTWidget.h"

#include "GTMouseDriver.h"
#include "GTKeyboardDriver.h"

namespace U2 {

#define GT_CLASS_NAME "GTSpinBox"

#define GT_METHOD_NAME "setValue"
void GTDoubleSpinbox::setValue(U2OpStatus& os, QDoubleSpinBox *spinBox, double v, GTGlobals::UseMethod useMethod) {
    GT_CHECK(spinBox != NULL, "spinBox is NULL");
    if(spinBox->value() == v){
        return;
    }

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
            while (QString().setNum(spinBox->value()) != QString().setNum(v)) {
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
            while (QString().setNum(spinBox->value()) != QString().setNum(v)) {
                GTKeyboardDriver::keyClick(os, key);
                GTGlobals::sleep(100);
            }
            break;

        case GTGlobals::UseKeyBoard:
            QString s = QString::number(v);
            GTWidget::setFocus(os, spinBox);

            GTGlobals::sleep(100);
            GTKeyboardDriver::keyClick(os, GTKeyboardDriver::key["up"]);
            GTGlobals::sleep(100);
            GTKeyboardDriver::keyClick(os, GTKeyboardDriver::key["down"]);
            GTKeyboardDriver::keySequence(os,s);

        }
    }

    double currIndex = spinBox->value();
    GT_CHECK(QString().setNum(currIndex) == QString().setNum(v), "Expected: " + QString().setNum(v) + " Found: " + QString().setNum(currIndex));
}
#undef GT_METHOD_NAME

#undef GT_CLASS_NAME

}
