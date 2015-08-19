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

#include <QMainWindow>
#include <QMenuBar>

#include <U2Core/AppContext.h>

#include <U2Gui/MainWindow.h>

#include "GTMenuPrivate.h"
#ifdef Q_OS_MAC
#include "GTMenuPrivateMac.h"
#endif
#include "GTUtilsDialog.h"
#include "api/GTMouseDriver.h"
#include "api/GTKeyboardDriver.h"
#include "runnables/qt/PopupChooser.h"

namespace U2 {

#define GT_CLASS_NAME "GTMenu"

#define GT_METHOD_NAME "clickMainMenuItem"
void GTMenuPrivate::clickMainMenuItem(U2OpStatus &os, const QStringList &itemPath, U2::GTGlobals::UseMethod method) {
    GT_CHECK(itemPath.count() > 1, QString("Menu item path is too short: { %1 }").arg(itemPath.join(" -> ")));
#ifdef Q_OS_MAC
    Q_UNUSED(method);
    GTMenuPrivateMac::clickMainMenuItem(os, itemPath);
#else
    QStringList cuttedItemPath = itemPath;
    const QString menuName = cuttedItemPath.takeFirst();
    GTUtilsDialog::waitForDialog(os, new PopupChooserbyText(os, cuttedItemPath, method));
    showMainMenu(os, menuName, method);
#endif
}
#undef GT_METHOD_NAME

#define GT_METHOD_NAME "showMainMenu"
void GTMenuPrivate::showMainMenu(U2OpStatus &os, const QString &menuName, GTGlobals::UseMethod m) {
    MainWindow *mw = AppContext::getMainWindow();
    GT_CHECK(mw != NULL, "MainWindow is NULL");
    QMainWindow *mainWindow = mw->getQMainWindow();
    GT_CHECK(mainWindow != NULL, "QMainWindow is NULL");

    QAction *menu = mainWindow->findChild<QAction*>(menuName);

    GT_CHECK(menu != NULL, QString("menu \"%1\" not found").arg(menuName));

    QPoint pos;
    QPoint gPos;
    QString menuText;
    int key = 0;
    int key_pos = 0;

    switch(m) {
    case GTGlobals::UseMouse:
        pos = mainWindow->menuBar()->actionGeometry(menu).center();
        gPos = mainWindow->menuBar()->mapToGlobal(pos);

        GTMouseDriver::moveTo(os, gPos);
        GTMouseDriver::click(os);
        break;

    case GTGlobals::UseKey:
        menuText = menu->text();
        key_pos = menuText.indexOf('&');
        key = (menuText.at(key_pos + 1)).toLatin1();

        GTKeyboardDriver::keyClick(os, key, GTKeyboardDriver::key["alt"]);
        break;

    default:
        break;
    }

    GTGlobals::sleep(1000);
}
#undef GT_METHOD_NAME

#undef GT_CLASS_NAME

}   // namespace U2
