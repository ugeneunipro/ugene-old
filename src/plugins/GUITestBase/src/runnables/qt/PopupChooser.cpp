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

#include "PopupChooser.h"
#include "api/GTMenu.h"
#include "api/GTMouseDriver.h"

#if (QT_VERSION < 0x050000) //Qt 5
#include <QtGui/QApplication>
#include <QtGui/QMenu>
#else
#include <QtWidgets/QApplication>
#include <QtWidgets/QMenu>
#endif

namespace U2 {

void PopupChooser::run()
{
    GTGlobals::sleep(1000);
    GTMouseDriver::release(os);
    QMenu* activePopupMenu = qobject_cast<QMenu*>(QApplication::activePopupWidget());
    GTMenu::clickMenuItemByName(os, activePopupMenu, namePath, useMethod);
}

void PopupChooserbyText::run()
{
    GTGlobals::sleep(1000);
    GTMouseDriver::release(os);
    QMenu* activePopupMenu = qobject_cast<QMenu*>(QApplication::activePopupWidget());
    GTMenu::clickMenuItemByName(os, activePopupMenu, namePath, useMethod);
}

}

