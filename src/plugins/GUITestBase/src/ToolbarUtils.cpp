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

#include "ToolbarUtils.h"
#include "QtUtils.h"
#include "QToolBar"
#include "U2Gui/MainWindow.h"
#include "QtUtils.h"
#include "QAction"
#include <U2Gui/GUIUtils.h>


namespace U2 {

    QToolBar* ToolbarUtils::getToolbar(U2OpStatus &os, const QString &toolbarSysName) {
        QString typeCheck = "QToolBar";
        QToolBar *tb = static_cast<QToolBar*>(QtUtils::findWidgetByName(os, toolbarSysName));
        if (0 != typeCheck.compare(tb->metaObject()->className())) { //the found widget is not a qtoolbar
            return NULL;
        }
        else {
            return tb;
        }
    }

    void ToolbarUtils::clickToolbarAction(U2OpStatus &os,const QString &actionName, const QToolBar *toolbar) {
        QAction *action = ToolbarUtils::getToolbarAction(os, actionName, toolbar);

        CHECK_SET_ERR(action != NULL, "No such action " + actionName + " in the toolbar " + toolbar->objectName());

        action->activate(QAction::Trigger);
    }

    void ToolbarUtils::hoverToolbarAction(U2OpStatus &os,const QString &actionName, const QToolBar *toolbar) {
        QAction *action = ToolbarUtils::getToolbarAction(os, actionName, toolbar);

        CHECK_SET_ERR(action != NULL, "No such action " + actionName + " in the toolbar " + toolbar->objectName());

        action->activate(QAction::Hover);
    }

    QAction* ToolbarUtils::getToolbarAction(U2OpStatus &os, const QString &actionName, const QToolBar *toolbar) {
        QAction *action = GUIUtils::findAction(toolbar->actions(), actionName);
        return action;
    }
    

}
