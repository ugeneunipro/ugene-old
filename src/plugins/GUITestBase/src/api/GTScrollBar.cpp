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

#include "GTScrollBar.h"


namespace U2 {

void GTScrollBar::pageUp(U2OpStatus &os, QScrollBar *scrollbar, device _device) {
    switch (_device) {
        case GTScrollBar::MOUSE:
            GTMouseDriver::moveTo(os, GTScrollBar::getAreaOverSliderPosition(os, scrollbar));
            GTMouseDriver::click(os);
            break;
            
        case GTScrollBar::KEYBOARD:
            GTMouseDriver::moveTo(os, GTScrollBar::getSliderPosition(os, scrollbar));
            GTMouseDriver::click(os);
            GTKeyboardDriver::keyClick(os, GTKeyboardDriver::key["pageUp"]);
            break;

        default:
            break;
    }
}

void GTScrollBar::pageDown(U2OpStatus &os, QScrollBar *scrollbar, device _device) {
    switch (_device) {
        case GTScrollBar::MOUSE:
            GTMouseDriver::moveTo(os, GTScrollBar::getAreaUnderSliderPosition(os, scrollbar));
            GTMouseDriver::click(os);
            break;

        case GTScrollBar::KEYBOARD:
            GTMouseDriver::moveTo(os, GTScrollBar::getSliderPosition(os, scrollbar));
            GTMouseDriver::click(os);
            GTKeyboardDriver::keyClick(os, GTKeyboardDriver::key["pageDown"]);
            break;
        
        default:
            break;

    }
}

void GTScrollBar::lineUp(U2OpStatus &os, QScrollBar *scrollbar, device _device) {
    switch (_device) {
        case GTScrollBar::MOUSE:
            GTMouseDriver::moveTo(os, GTScrollBar::getUpArrowPosition(os, scrollbar));
            GTMouseDriver::click(os);
            break;

        case GTScrollBar::KEYBOARD:
            GTMouseDriver::moveTo(os, GTScrollBar::getSliderPosition(os, scrollbar));
            GTMouseDriver::click(os);
            GTKeyboardDriver::keyClick(os, GTKeyboardDriver::key["up"]);
            break;

        default:
            break;
    }
}


void GTScrollBar::lineDown(U2OpStatus &os, QScrollBar *scrollbar, device _device) {
    switch (_device) {
        case GTScrollBar::MOUSE:
            GTMouseDriver::moveTo(os, GTScrollBar::getDownArrowPosition(os, scrollbar));
            GTMouseDriver::click(os);
            break;

        case GTScrollBar::KEYBOARD:
            GTMouseDriver::moveTo(os, GTScrollBar::getSliderPosition(os, scrollbar));
            GTMouseDriver::click(os);
            GTKeyboardDriver::keyClick(os, GTKeyboardDriver::key["down"]);
            break;

        default:
            break;
    }
}

void GTScrollBar::moveSliderWithMouseUp(U2OpStatus &os, QScrollBar *scrollbar, int nPix) {
    GTMouseDriver::moveTo(os, GTScrollBar::getSliderPosition(os, scrollbar));
    GTMouseDriver::press(os);
    QPoint newPosition;
    if (Qt::Horizontal == scrollbar->orientation()) {
        newPosition = QPoint(QCursor::pos().x() + nPix, QCursor::pos().y());
    }   
    else {
        newPosition = QPoint(QCursor::pos().x(), QCursor::pos().y() + nPix);
    }
    GTMouseDriver::moveTo(os, newPosition);
    GTMouseDriver::release(os);  
}

void GTScrollBar::moveSliderWithMouseDown(U2OpStatus &os, QScrollBar *scrollbar, int nPix) {
    GTMouseDriver::moveTo(os, GTScrollBar::getSliderPosition(os, scrollbar));
    GTMouseDriver::press(os);
    QPoint newPosition;
    if (Qt::Horizontal == scrollbar->orientation()) {
        newPosition = QPoint(QCursor::pos().x() - nPix , QCursor::pos().y());
    }   
    else {
        newPosition = QPoint(QCursor::pos().x(), QCursor::pos().y() - nPix);
    }
    GTMouseDriver::moveTo(os, newPosition);
    GTMouseDriver::release(os);   
}

void GTScrollBar::moveSliderWithMouseWheelUp(U2OpStatus &os, QScrollBar *scrollbar, int nScrolls) {
    GTMouseDriver::moveTo(os, GTScrollBar::getSliderPosition(os, scrollbar));
    GTMouseDriver::click(os);
    GTMouseDriver::scroll(os, nScrolls);
}

void GTScrollBar::moveSliderWithMouseWheelDown(U2OpStatus &os, QScrollBar *scrollbar, int nScrolls) {
    GTMouseDriver::moveTo(os, GTScrollBar::getSliderPosition(os, scrollbar));
    GTMouseDriver::click(os);
    GTMouseDriver::scroll(os, (-1 * nScrolls)); //since scrolling down means negative value for GTMouseDriver::scroll
}

//todo
QPoint GTScrollBar::getSliderPosition(U2OpStatus &os, QScrollBar *scrollbar) {
    return QPoint();
}

QPoint GTScrollBar::getUpArrowPosition(U2OpStatus &os, QScrollBar *scrollbar) {
    return QPoint();
}

QPoint GTScrollBar::getDownArrowPosition(U2OpStatus &os, QScrollBar *scrollbar) {
    return QPoint();
}

QPoint GTScrollBar::getAreaUnderSliderPosition(U2OpStatus &os, QScrollBar *scrollbar) {
    return QPoint();
}

QPoint GTScrollBar::getAreaOverSliderPosition(U2OpStatus &os, QScrollBar *scrollbar) {
    return QPoint();
}



    
}
// namespace
