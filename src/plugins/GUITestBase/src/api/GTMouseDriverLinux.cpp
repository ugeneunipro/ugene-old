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

#include "GTMouseDriver.h"
#include "GTGlobals.h"

#ifdef __linux__
    #include <X11/extensions/XTest.h>
#endif

namespace U2 {

#ifdef __linux__

void GTMouseDriver::moveTo(U2::U2OpStatus &os, const int x, const int y)
{
    Display *display = XOpenDisplay(NULL);
    CHECK_SET_ERR (display != 0, "Error: display is NULL in GTMouseDriver::moveTo()");

    int horres = XDisplayWidth(display, 0);
    int vertres = XDisplayHeight(display, 0);

    QRect screen(0, 0, horres-1, vertres-1);
    CHECK_SET_ERR(screen.contains(QPoint(x, y)), "Invalid coordinates for GTMouseDriver::moveTo()");

    Window root, child;
    int root_x, root_y, pos_x, pos_y;
    unsigned mask;
    XQueryPointer(display, RootWindow(display, DefaultScreen(display)),
                  &root, &child, &root_x, &root_y,
                  &pos_x, &pos_y, &mask);

    const int delay = 10; //msec
    int x0 = pos_x;
    int y0 = pos_y;
    int x1 = x;
    int y1 = y;

    if (x0 == x1) {
        while(y0 != y1) {
            if (y0 < y1) {
                ++y0;
            } else {
                --y0;
            }

            XTestFakeMotionEvent(display, -1, x1, y0, delay);
            XFlush(display);
        }
    } else if (y0 == y1) {
        while(x0 != x1) {
            if (x0 < x1) {
                ++x0;
            } else {
                --x0;
            }
            XTestFakeMotionEvent(display, -1, x0, y1, delay);
            XFlush(display);
        }
    } else {
        // moved by the shortest way
        // equation of the line by two points y = (-(x0 * y1 - x1 * y0) - x*(y0 - y1)) / (x1 - x0)
        int diff_x = x1 - x0;
        int diff_y = y0 - y1;
        int diff_xy = -(x0 * y1 - x1 * y0);
        int current_x = x0, current_y;

        while (current_x != x1) {
            if (x1 > x0) {
                ++current_x;
            } else {
                -- current_x;
            }

            current_y = (diff_xy - current_x * diff_y) / diff_x;
            XTestFakeMotionEvent(display, -1, current_x, current_y, delay);
            XFlush(display);
        }
    }

    XCloseDisplay(display);
}

void GTMouseDriver::press(U2::U2OpStatus &os, Qt::MouseButton button)
{
    Display *display = XOpenDisplay(NULL);
    CHECK_SET_ERR (display != 0, "Error: display is NULL in GTMouseDriver::press()");

    //1 = Left, 2 = Middle, 3 = Right
    unsigned int btn = button == Qt::LeftButton ? 1 :
                       button == Qt::RightButton ? 3 :
                       button == Qt::MidButton ? 2 : 0;
    CHECK_SET_ERR (btn != 0, "Error: button is 0 in GTMouseDriver::press()");

    XTestFakeButtonEvent(display, btn, True, 0);
    XFlush(display);

    XCloseDisplay(display);
}

void GTMouseDriver::release(U2::U2OpStatus &os, Qt::MouseButton button)
{
    // TODO: check if this key has been already pressed
    Display *display = XOpenDisplay(NULL);
    CHECK_SET_ERR (display != 0, "Error: display is NULL in GTMouseDriver::press()");

    unsigned int btn = button == Qt::LeftButton ? 1 :
                       button == Qt::RightButton ? 3 :
                       button == Qt::MidButton ? 2 : 0;
    CHECK_SET_ERR (btn != 0, "Error: button is 0 in GTMouseDriver::release()");

    XTestFakeButtonEvent(display, btn, False, 0);
    XFlush(display);

    XCloseDisplay(display);
}

void GTMouseDriver::scroll(U2OpStatus &os, int value)
{
    Display *display = XOpenDisplay(NULL);
    CHECK_SET_ERR (display != 0, "Error: display is NULL in GTMouseDriver::press()");

    unsigned button =  value > 0 ? Button4 : Button5; //Button4 - scroll up, Button5 - scroll down
    value = value > 0 ? value : -value;

    for (int i = 0; i < value; i++) {
        XTestFakeButtonEvent(display, button, True, 0);
        XTestFakeButtonEvent(display, button, False, 0);
    }

    XFlush(display);
    XCloseDisplay(display);
}

#endif
} // namespace
