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

#include <cctype>
#include "GTKeyboardDriver.h"
#include "QtUtils.h"

#ifdef _WIN32
    #include <windows.h>
    #define VIRTUAL_TO_SCAN_CODE 0
#elif defined __linux__
    #define XK_LATIN1      // for latin symbol
    #define XK_MISCELLANY  // for action keys
    #include <X11/keysymdef.h>
    #include <X11/extensions/XTest.h>
#elif defined __APPLE__ & __MACH__
    //...
#endif

#define ADD_KEY(name, code) insert(name, code)

namespace U2 {

#ifdef _WIN32

void GTKeyboardDriver::keyPress(U2::U2OpStatus &os, char key)
{
    if (isalpha(key) && islower(key)) {
        key = toupper(key);
    }
    keyPress(os, (int)key);
}

void GTKeyboardDriver::keyPress(U2::U2OpStatus &os, char key, int modifiers)
{
    if (isalpha(key) && islower(key)) {
        key = toupper(key);
    }
    keyPress(os, (int)key, modifiers);
}

void GTKeyboardDriver::keyRelease(U2OpStatus &os, char key)
{
    if (isalpha(key) && islower(key)) {
        key = toupper(key);
    }
    keyRelease(os, (int)key);
}

void GTKeyboardDriver::keyRelease(U2OpStatus &os, char key, int modifiers)
{
    if (isalpha(key) && islower(key)) {
        key = toupper(key);
    }
    keyRelease(os, (int)key, modifiers);
}
 
void GTKeyboardDriver::keyClick(U2OpStatus &os, char key)
{
    CHECK_SET_ERR(key != 0, " Error: key = 0 in GTKeyboardDriver::keyClick()");

    keyPress(os, key);
    keyRelease(os, key);
}

void GTKeyboardDriver::keyClick(U2OpStatus &os, char key, int modifiers)
{
    CHECK_SET_ERR(key != 0, " Error: key = 0 in GTKeyboardDriver::keyClick()");
    CHECK_SET_ERR(modifiers != 0, " Error: modifiers = 0 in GTKeyboardDriver::keyClick()");

    keyPress(os, key, modifiers);
    keyRelease(os, key, modifiers);
}

void GTKeyboardDriver::keyPress(U2::U2OpStatus &os, int key)
{
    CHECK_SET_ERR(key != 0, " Error: key = 0 in GTKeyboardDriver::keyPress()");

    INPUT event;
    event.type = INPUT_KEYBOARD;
    event.ki.wVk = key;
    event.ki.wScan = MapVirtualKey(key, VIRTUAL_TO_SCAN_CODE);
    event.ki.dwFlags = 0;
    event.ki.time = 0;
    event.ki.dwExtraInfo = 0;

    SendInput(1, &event, sizeof(event));
}

void GTKeyboardDriver::keyPress(U2::U2OpStatus &os, int key, int modifiers)
{
    CHECK_SET_ERR(key != 0, " Error: key = 0 in GTKeyboardDriver::keyPress()");
    CHECK_SET_ERR(modifiers != 0, " Error: modifiers = 0 in GTKeyboardDriver::keyPress()");

    INPUT event;
    event.type = INPUT_KEYBOARD;
    event.ki.wVk = modifiers;
    event.ki.wScan = MapVirtualKey(modifiers, VIRTUAL_TO_SCAN_CODE);
    event.ki.dwFlags = 0;
    event.ki.time = 0;
    event.ki.dwExtraInfo = 0;

    SendInput(1, &event, sizeof(event));

    event.ki.wVk = key;
    event.ki.wScan = MapVirtualKey(key, VIRTUAL_TO_SCAN_CODE);

    SendInput(1, &event, sizeof(event));
}

void GTKeyboardDriver::keyRelease(U2::U2OpStatus &os, int key)
{
    CHECK_SET_ERR(key != 0, " Error: key = 0 in GTKeyboardDriver::keyRelease()");

    INPUT event;
    event.type = INPUT_KEYBOARD;
    event.ki.wVk = key;
    event.ki.wScan = MapVirtualKey(key, VIRTUAL_TO_SCAN_CODE);
    event.ki.dwFlags = KEYEVENTF_KEYUP;
    event.ki.time = 0;
    event.ki.dwExtraInfo = 0;

    SendInput(1, &event, sizeof(event));
}

void GTKeyboardDriver::keyRelease(U2::U2OpStatus &os, int key, int modifiers)
{
    CHECK_SET_ERR(key != 0, " Error: key = 0 in GTKeyboardDriver::keyRelease()");
    CHECK_SET_ERR(modifiers != 0, " Error: modifiers = 0 in GTKeyboardDriver::keyRelease()");

    INPUT event;
    event.type = INPUT_KEYBOARD;
    event.ki.wVk = modifiers;
    event.ki.wScan = MapVirtualKey(modifiers, VIRTUAL_TO_SCAN_CODE);
    event.ki.dwFlags = KEYEVENTF_KEYUP;
    event.ki.time = 0;
    event.ki.dwExtraInfo = 0;

    SendInput(1, &event, sizeof(event));

    event.ki.wVk = key;
    event.ki.wScan = MapVirtualKey(key, VIRTUAL_TO_SCAN_CODE);

    SendInput(1, &event, sizeof(event));
}

GTKeyboardDriver::keys::keys()
{
    ADD_KEY("back", VK_BACK);
    ADD_KEY("tab", VK_TAB);
    ADD_KEY("clear", VK_CLEAR);
    ADD_KEY("enter", VK_RETURN);
    ADD_KEY("shift", VK_SHIFT);
    ADD_KEY("rshift", VK_RSHIFT);
    ADD_KEY("lshift", VK_LSHIFT);
    ADD_KEY("ctrl", VK_CONTROL);
    ADD_KEY("rctrl", VK_RCONTROL);
    ADD_KEY("lctrl", VK_LCONTROL);
    ADD_KEY("alt", VK_MENU);
    ADD_KEY("pause", VK_PAUSE);
    ADD_KEY("esc", VK_ESCAPE);
    ADD_KEY("space", VK_SPACE);
    ADD_KEY("left", VK_LEFT);
    ADD_KEY("up", VK_UP);
    ADD_KEY("right", VK_RIGHT);
    ADD_KEY("down", VK_DOWN);
    ADD_KEY("insert", VK_INSERT);
    ADD_KEY("delete", VK_DELETE);
    ADD_KEY("help", VK_HELP);
    ADD_KEY("f1", VK_F1);
    ADD_KEY("f2", VK_F2);
    ADD_KEY("f3", VK_F3);
    ADD_KEY("f4", VK_F4);
    ADD_KEY("f5", VK_F5);
    ADD_KEY("f6", VK_F6);
    ADD_KEY("f7", VK_F7);
    ADD_KEY("f8", VK_F8);
    ADD_KEY("f9", VK_F9);
    ADD_KEY("f10", VK_F10);
    ADD_KEY("f12", VK_F12);

// feel free to add other keys
// macro VK_* defined in WinUser.h
}

#elif defined __linux__

void GTKeyboardDriver::keyPress(U2::U2OpStatus &os, int key)
{
    CHECK_SET_ERR (key != 0, " Error: key = 0 in GTKeyboardDriver::keyPress()");

    Display *display = XOpenDisplay(NULL);
    CHECK_SET_ERR (display != 0, "Error: display is NULL in keyPress()");

    XTestFakeKeyEvent(display, XKeysymToKeycode(display, key), 1, 0);
    XFlush(display);

    XCloseDisplay(display);
}

void GTKeyboardDriver::keyPress(U2::U2OpStatus &os, int key, int modifiers)
{
    CHECK_SET_ERR(key != 0, " Error: key = 0 in GTKeyboardDriver::keyPress()");
    CHECK_SET_ERR(modifiers != 0, " Error: modifiers = 0 in GTKeyboardDriver::keyPress()");

    Display *display = XOpenDisplay(NULL);
    CHECK_SET_ERR (display != 0, "Error: display is NULL in keyPress()");

    XTestFakeKeyEvent(display, XKeysymToKeycode(display, modifiers), 1, 0);
    XTestFakeKeyEvent(display, XKeysymToKeycode(display, key), 1, 0);
    XFlush(display);

    XCloseDisplay(display);
}

void GTKeyboardDriver::keyRelease(U2::U2OpStatus &os, int key)
{
    CHECK_SET_ERR(key != 0, " Error: key = 0 in GTKeyboardDriver::keyRelease()");

    Display *display = XOpenDisplay(NULL);
    CHECK_SET_ERR (display != 0, "Error: display is NULL in keyRelease()");

    XTestFakeKeyEvent(display, XKeysymToKeycode(display, key), 0, 0);
    XFlush(display);

    XCloseDisplay(display);
}

void GTKeyboardDriver::keyRelease(U2::U2OpStatus &os, int key, int modifiers)
{
    CHECK_SET_ERR(key != 0, " Error: key = 0 in GTKeyboardDriver::keyRelease()");
    CHECK_SET_ERR(modifiers != 0, " Error: modifiers = 0 in GTKeyboardDriver::keyRelease()");

    Display *display = XOpenDisplay(NULL);
    CHECK_SET_ERR (display != 0, "Error: display is NULL in keyRelease()");

    XTestFakeKeyEvent(display, XKeysymToKeycode(display, key), 0, 0);
    XTestFakeKeyEvent(display, XKeysymToKeycode(display, modifiers), 0, 0);
    XFlush(display);

    XCloseDisplay(display);
}

GTKeyboardDriver::keys::keys()
{
    ADD_KEY("back", XK_BackSpace);
    ADD_KEY("tab", XK_Tab);
    ADD_KEY("clear", XK_Clear);
    ADD_KEY("enter", XK_Return);
    ADD_KEY("shift", XK_Shift_L);
    ADD_KEY("rshift", XK_Shift_R);
    ADD_KEY("lshift", XK_Shift_L);
    ADD_KEY("ctrl", XK_Control_L);
    ADD_KEY("rctrl", XK_Control_R);
    ADD_KEY("lctrl", XK_Control_L);
    ADD_KEY("menu", XK_Menu);//alt?
    ADD_KEY("pause", XK_Pause);
    ADD_KEY("esc", XK_Escape);
    ADD_KEY("space", XK_space);
    ADD_KEY("left", XK_Left);
    ADD_KEY("up", XK_Up);
    ADD_KEY("right", XK_Right);
    ADD_KEY("down", XK_Down);
    ADD_KEY("insert", XK_Insert);
    ADD_KEY("delete", XK_Delete);
    ADD_KEY("help", XK_Help);
    ADD_KEY("f1", XK_F1);
    ADD_KEY("f2", XK_F2);
    ADD_KEY("f3", XK_F3);
    ADD_KEY("f4", XK_F4);
    ADD_KEY("f5", XK_F5);
    ADD_KEY("f6", XK_F6);
    ADD_KEY("f7", XK_F7);
    ADD_KEY("f8", XK_F8);
    ADD_KEY("f9", XK_F9);
    ADD_KEY("f10", XK_F10);
    ADD_KEY("f12", XK_F12);

// feel free to add other keys
// macro XK_* defined in X11/keysymdef.h
}

#elif defined __APPLE__ & __MACH__
void GTKeyboardDriver::keyPress(U2::U2OpStatus &os, int key)
{
}

void GTKeyboardDriver::keyPress(U2::U2OpStatus &os, int key, int modifiers)
{
}

void GTKeyboardDriver::keyRelease(U2::U2OpStatus &os, int key)
{
}

void GTKeyboardDriver::keyRelease(U2::U2OpStatus &os, int key, int modifiers)
{
}

GTKeyboardDriver::keys::keys()
{
}
#endif

void GTKeyboardDriver::keyClick(U2::U2OpStatus &os, int key)
{
    CHECK_SET_ERR(key != 0, " Error: key = 0 in GTKeyboardDriver::keyClick()");

    keyPress(os, key);
    keyRelease(os, key);
}

void GTKeyboardDriver::keyClick(U2::U2OpStatus &os, int key, int modifiers)
{
    CHECK_SET_ERR(key != 0, " Error: key = 0 in GTKeyboardDriver::keyClick()");
    CHECK_SET_ERR(modifiers != 0, " Error: modifiers = 0 in GTKeyboardDriver::keyClick()");

    keyPress(os, key, modifiers);
    keyRelease(os, key, modifiers);
}

void GTKeyboardDriver::keySequence(U2::U2OpStatus &os, QString str)
{
    foreach(QChar ch, str) {
        if(isalpha(ch.toAscii()) && !islower(ch.toAscii())) {
            keyClick(os, ch.toAscii(), key["shift"]);
        } else {
            keyClick(os, ch.toAscii());
        }
    }
}

void GTKeyboardDriver::keySequence(U2::U2OpStatus &os, QString str, int modifiers)
{
    keyPress(os, modifiers);

    foreach(QChar ch, str) {
        if(isalpha(ch.toAscii()) && !islower(ch.toAscii())) {
            keyClick(os, ch.toAscii(), key["shift"]);
        } else {
            keyClick(os, ch.toAscii());
        }
    }

    keyRelease(os, modifiers);
}

/******************************************************************************/
int GTKeyboardDriver::keys::operator [] (QString str) const
{
    if (!contains(str.toLower())) {
        return 0;
    }
    return value(str.toLower());
}

GTKeyboardDriver::keys GTKeyboardDriver::key;

} //namespace
