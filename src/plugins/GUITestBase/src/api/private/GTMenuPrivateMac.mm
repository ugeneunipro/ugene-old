/**
 * UGENE - Integrated Bioinformatics Tools.
 * Copyright (C) 2008-2015 UniPro <ugene@unipro.ru>
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

#include "GTMenuPrivateMac.h"

namespace U2 {

#ifdef __OBJC__

#define GT_CLASS_NAME "GTMenuPrivateMac"

namespace {

inline NSString *qStringToNSString(const QString &qstr) {
    CFStringRef cfStringRef = CFStringCreateWithCharacters(0, reinterpret_cast<const UniChar *>(qstr.unicode()), qstr.length());
    return [const_cast<NSString *>(reinterpret_cast<const NSString *>(cfStringRef)) autorelease];
}
}

#define GT_METHOD_NAME "clickMainMenuItem"
void GTMenuPrivateMac::clickMainMenuItem(U2OpStatus &os, const QStringList &itemPath) {
    NSMenu *menu = [NSApp mainMenu];
    foreach (const QString &itemTitle, itemPath) {
        GT_CHECK(NULL != menu, "Menu is NULL");
        menu = clickMenuItem(os, menu, itemTitle);
        GTGlobals::sleep();
    }
}
#undef GT_METHOD_NAME

#define GT_METHOD_NAME "clickMenuItem"
NSMenu * GTMenuPrivateMac::clickMenuItem(U2OpStatus &os, NSMenu *menu, const QString &itemTitle) {
    NSMenuItem *item = [menu itemWithTitle:qStringToNSString(itemTitle)];
    GT_CHECK_RESULT(NULL != item, "Menu item is NULL", NULL);
    NSMenu *submenu = [item submenu];
    [menu performActionForItemAtIndex:[menu indexOfItem:item]];
    return submenu;
}
#undef GT_METHOD_NAME

#undef GT_CLASS_NAME

}   // namespace U2

#endif // __OBJC__
