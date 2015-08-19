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

#ifndef _U2_GT_MENU_PRIVATE_MAC_H_
#define _U2_GT_MENU_PRIVATE_MAC_H_


#ifdef __OBJC__
#import <AppKit/AppKit.h>
#endif // __OBJC__

#include "api/GTGlobals.h"

namespace U2 {

/**
 * It is supposed that Cocoa is used on OSX
 */
class GTMenuPrivateMac {
public:
    static void clickMainMenuItem(U2OpStatus &os, const QStringList &itemPath);

private:
#ifdef __OBJC__
    static NSMenu *clickMenuItem(U2OpStatus &os, NSMenu *menu, const QString &itemTitle);
#endif // __OBJC__
};

}   // namespace U2

#endif // _U2_GT_MENU_PRIVATE_MAC_H_
