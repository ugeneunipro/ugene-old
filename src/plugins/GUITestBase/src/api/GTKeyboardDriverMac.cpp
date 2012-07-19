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

#include "GTKeyboardDriver.h"
#include "api/GTGlobals.h"

#ifdef  Q_OS_MAC
#include <ApplicationServices/ApplicationServices.h>
#include <Carbon/Carbon.h>
#endif

namespace U2 {

#ifdef  Q_OS_MAC

int asciiToVirtual(int);

#define GT_CLASS_NAME "GTKeyboardDriverMac"
#define GT_METHOD_NAME "keyPress_char"
void GTKeyboardDriver::keyPress(U2::U2OpStatus &os, char key, int modifiers)
{
    GT_CHECK(key != 0, "key = 0");

    bool isChanged = false;
    switch(key) {
    case '_':
        key = asciiToVirtual('-');
        isChanged = true;
        break;
    case '<':
        key = asciiToVirtual(',');
        isChanged = true;
        break;
    case '>':
        key = asciiToVirtual('.');
        isChanged = true;
        break;
    case '(':
        key = asciiToVirtual('9');
        isChanged = true;
        break;
    case ')':
        key = asciiToVirtual('0');
        isChanged = true;
        break;
    }

    if (isChanged) {
        CGEventRef event = CGEventCreateKeyboardEvent(NULL, GTKeyboardDriver::key["shift"], true);
        GT_CHECK(event != NULL, "Can't create event");

        CGEventPost(kCGSessionEventTap, event);
        CFRelease(event);
    } else {
        key = asciiToVirtual(key);
    }

    GTGlobals::sleep(10);
    keyPress(os, (int)key, modifiers);
}

#define GT_METHOD_NAME "keyPress_int"
void GTKeyboardDriver::keyPress(U2::U2OpStatus &os, int key, int modifiers)
{
    if (modifiers) {
        CGEventRef event = CGEventCreateKeyboardEvent(NULL, modifiers, true);
        GT_CHECK(event != NULL, "Can't create event");

        CGEventPost(kCGSessionEventTap, event);
        CFRelease(event);
        GTGlobals::sleep(10);
    }

    CGEventRef event = CGEventCreateKeyboardEvent(NULL, key, true);
    GT_CHECK(event != NULL, "Can't create event");

    CGEventPost(kCGSessionEventTap, event);
    CFRelease(event);
    GTGlobals::sleep(10);
}
#undef GT_METHOD_NAME

#define GT_METHOD_NAME "keyRelease_char"
void GTKeyboardDriver::keyRelease(U2::U2OpStatus &os, char key, int modifiers)
{
    GT_CHECK(key != 0, "key = 0");

    bool isChanged = false;
    switch(key) {
    case '_':
        key = asciiToVirtual('-');
        isChanged = true;
        break;
    case '<':
        key = asciiToVirtual(',');
        isChanged = true;
        break;
    case '>':
        key = asciiToVirtual('.');
        isChanged = true;
        break;
    case '(':
        key = asciiToVirtual('9');
        isChanged = true;
        break;
    case ')':
        key = asciiToVirtual('0');
        isChanged = true;
        break;
    }

    if (!isChanged) {
        key = asciiToVirtual(key);
    } else {
        CGEventRef event = CGEventCreateKeyboardEvent(NULL, GTKeyboardDriver::key["shift"], false);
        GT_CHECK(event != NULL, "Can't create event");

        CGEventPost(kCGSessionEventTap, event);
        CFRelease(event);
    }

    GTGlobals::sleep(10);
    keyRelease(os, (int) key, modifiers);
}

#define GT_METHOD_NAME "keyRelease_int"
void GTKeyboardDriver::keyRelease(U2::U2OpStatus &os, int key, int modifiers)
{
    CGEventRef event = CGEventCreateKeyboardEvent(NULL, key, false);
    GT_CHECK(event != NULL, "Can't create event");

    CGEventPost(kCGSessionEventTap, event);
    CFRelease(event);
    GTGlobals::sleep(10);

    if (modifiers) {
        CGEventRef event = CGEventCreateKeyboardEvent(NULL, modifiers, false);
        GT_CHECK(event != NULL, "Can't create event");

        CGEventPost(kCGSessionEventTap, event);
        CFRelease(event);
    }
    GTGlobals::sleep(10);
}
#undef GT_METHOD_NAME

GTKeyboardDriver::keys::keys()
{
    ADD_KEY("cmd", kVK_Command);
    ADD_KEY("tab", kVK_Tab);
    ADD_KEY("enter", kVK_Return);
    ADD_KEY("shift", kVK_Shift);
    ADD_KEY("ctrl", kVK_Control);
    ADD_KEY("alt", kVK_Option);
    ADD_KEY("esc", kVK_Escape);
    ADD_KEY("space", kVK_Space);
    ADD_KEY("left", kVK_LeftArrow);
    ADD_KEY("up", kVK_UpArrow);
    ADD_KEY("right", kVK_RightArrow);
    ADD_KEY("down", kVK_DownArrow);
    ADD_KEY("delete", kVK_Delete);
    ADD_KEY("help", kVK_Help);
    ADD_KEY("f1", kVK_F1);
    ADD_KEY("f2", kVK_F2);
    ADD_KEY("f3", kVK_F3);
    ADD_KEY("f4", kVK_F4);
    ADD_KEY("f5", kVK_F5);
    ADD_KEY("f6", kVK_F6);
    ADD_KEY("f7", kVK_F7);
    ADD_KEY("f8", kVK_F8);
    ADD_KEY("f9", kVK_F9);
    ADD_KEY("f10", kVK_F10);
    ADD_KEY("f12", kVK_F12);
    //ADD_KEY("pageUp", VK_PRIOR);
    //ADD_KEY("pageDown", VK_NEXT);

// feel free to add other keys
// macro kVK_* defined in Carbon.framework/Frameworks/HIToolbox.framework/Headers/Events.h
}

#define GT_METHOD_NAME "keyClick"
void GTKeyboardDriver::keyClick(U2::U2OpStatus &os, char key, int modifiers)
{
    GT_CHECK(key != 0, "key = 0");

    keyPress(os, key, modifiers);
    GTGlobals::sleep(10);
    keyRelease(os, key, modifiers);
    GTGlobals::sleep(10);
}
#undef GT_METHOD_NAME

#undef GT_CLASS_NAME

int asciiToVirtual(int key)
{
    if (isalpha(key)) {
        key = tolower(key);
    }

    switch(key) {
    case ' ':
        key = kVK_Space;
        break;
    case '0':
        key = kVK_ANSI_0;
        break;
    case '1':
        key = kVK_ANSI_1;
        break;
    case '2':
        key = kVK_ANSI_2;
        break;
    case '3':
        key = kVK_ANSI_3;
        break;
    case '4':
        key = kVK_ANSI_4;
        break;
    case '5':
        key = kVK_ANSI_5;
        break;
    case '6':
        key = kVK_ANSI_6;
        break;
    case '7':
        key = kVK_ANSI_7;
        break;
    case '8':
        key = kVK_ANSI_8;
        break;
    case '9':
        key = kVK_ANSI_9;
        break;
    case 'a':
        key = kVK_ANSI_A;
        break;
    case 'b':
        key = kVK_ANSI_B;
        break;
    case 'c':
        key = kVK_ANSI_C;
        break;
    case 'd':
        key = kVK_ANSI_D;
        break;
    case 'e':
        key = kVK_ANSI_E;
        break;
    case 'f':
        key = kVK_ANSI_F;
        break;
    case 'g':
        key = kVK_ANSI_G;
        break;
    case 'h':
        key = kVK_ANSI_H;
        break;
    case 'i':
        key = kVK_ANSI_I;
        break;
    case 'j':
        key = kVK_ANSI_J;
        break;
    case 'k':
        key = kVK_ANSI_K;
        break;
    case 'l':
        key = kVK_ANSI_L;
        break;
    case 'm':
        key = kVK_ANSI_M;
        break;
    case 'n':
        key = kVK_ANSI_N;
        break;
    case 'o':
        key = kVK_ANSI_O;
        break;
    case 'p':
        key = kVK_ANSI_P;
        break;
    case 'q':
        key = kVK_ANSI_Q;
        break;
    case 'r':
        key = kVK_ANSI_R;
        break;
    case 's':
        key = kVK_ANSI_S;
        break;
    case 't':
        key = kVK_ANSI_T;
        break;
    case 'u':
        key = kVK_ANSI_U;
        break;
    case 'v':
        key = kVK_ANSI_V;
        break;
    case 'w':
        key = kVK_ANSI_W;
        break;
    case 'x':
        key = kVK_ANSI_X;
        break;
    case 'y':
        key = kVK_ANSI_Y;
        break;
    case 'z':
        key = kVK_ANSI_Z;
        break;
    case '=':
        key = kVK_ANSI_Equal;
        break;
    case '-':
        key = kVK_ANSI_Minus;
        break;
    case ']':
        key = kVK_ANSI_RightBracket;
        break;
    case '[':
        key = kVK_ANSI_LeftBracket;
        break;
    case '\'':
        key = kVK_ANSI_Quote;
        break;
    case ';':
        key = kVK_ANSI_Semicolon;
        break;
    case '\\':
        key = kVK_ANSI_Backslash;
        break;
    case ',':
        key = kVK_ANSI_Comma;
        break;
    case '/':
        key = kVK_ANSI_Slash;
        break;
    case '.':
        key = kVK_ANSI_Period;
        break;
    }

    return key;
}

#endif
} //namespace
