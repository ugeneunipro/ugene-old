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

#include "api/GTMenu.h"
#include "QtUtils.h"
#include <U2Core/AppContext.h>

namespace U2 {

QPoint GTMenu::getMenuPos(U2::U2OpStatus &os, const QString &menuName)
{
    QMainWindow *mainWindow = AppContext::getMainWindow()->getQMainWindow();
    QList<QAction*>actions = mainWindow->findChildren<QAction*>();

    QAction *action = NULL;
    foreach(QAction *a, actions) {
        if(a->text() == menuName) {
            action = a;
            break;
        }
    }

    CHECK_SET_ERR_RESULT(action != NULL && action->isVisible(),
                             QString("Error: action \"%1\" not found or not visible in getMenuPos()").arg(menuName),
                             BAD_POINT);

    QPoint pos = mainWindow->menuBar()->actionGeometry(action).center();
    QPoint gPos = mainWindow->menuBar()->mapToGlobal(pos);

    return gPos;
}

QPoint GTMenu::getActionPos(U2::U2OpStatus &os, const QString &menuName, const QString &actionName)
{
    QMainWindow *mainWindow = AppContext::getMainWindow()->getQMainWindow();
    QList<QAction*>actions = mainWindow->findChildren<QAction*>();

    QAction *action = NULL;
    foreach(QAction *a, actions) {
        if(a->text() == menuName) {
            action = a;
            break;
        }
    }

    CHECK_SET_ERR_RESULT(action != NULL && action->isVisible(),
                             QString("Error: menu \"%1\" not found or not visible in getMenuPos()").arg(menuName),
                             BAD_POINT);

    QMenu *menu = action->menu();
    actions = menu->actions();
    QAction *act = NULL;

    foreach(QAction *a, actions) {
        qDebug() << a->text();
        if (a->text() == actionName) {
            act = a;
            break;
        }
    }

    CHECK_SET_ERR_RESULT(act != NULL && act->isVisible(),
                         QString("Error: action \"%1\" not found or not visible in getActionPos()").arg(actionName),
                         BAD_POINT);

    QRect menuPos = mainWindow->menuBar()->actionGeometry(action);
    QPoint actionPos = menu->actionGeometry(act).center();
    QPoint gPos = mainWindow->menuBar()->mapToGlobal(QPoint(menuPos.x() + actionPos.x(), menuPos.bottom() + actionPos.y()));

    return gPos;
    }

QPoint GTMenu::getContextMenuActionPos(U2::U2OpStatus &os, const QString &actionName)
{
    QMenu *menu = static_cast<QMenu*>(QApplication::activePopupWidget());
    CHECK_SET_ERR_RESULT(menu != NULL && menu->isVisible(),
                         "Error: context menu not found or not visible in getContextMenuActionPos()",
                         BAD_POINT);

    QList<QAction*> actions = menu->actions();
    QAction *action = NULL;

    foreach(QAction *a, actions) {
        if(a->text() == actionName) {
            action = a;
            break;
        }
    }

    CHECK_SET_ERR_RESULT(action != NULL && action->isVisible(),
                         "Error: action not found or not visible in getContextMenuActionPos()",
                         BAD_POINT);

    QPoint pos = menu->actionGeometry(action).center();
    QPoint gPos = menu->mapToGlobal(pos);

    return gPos;
}

}
