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

#ifndef _U2_GT_GLOBALS_H_
#define _U2_GT_GLOBALS_H_

#include <U2Core/U2OpStatus.h>
#include <U2Core/U2SafePoints.h>

#include <QtGui/QAction>

namespace U2 {

#define CHECK_SET_ERR(condition, errorMessage) \
    CHECK_SET_ERR_RESULT(condition, errorMessage, )

#define CHECK_SET_ERR_RESULT(condition, errorMessage, result) \
    CHECK_EXT(condition, if (!os.hasError()) {os.setError(errorMessage);}, result)

class GTGlobals {
public:
    enum UseMethod {UseMouse, UseKey};
    enum WindowAction {Minimize, Maximize, Close, WindowActionCount};

    static void sleep(int msec);
    static void sendEvent(QObject *obj, QEvent *e);

    static void expandTopLevelMenu(U2OpStatus &os, const QString &menuName, const QString &parentMenu);
    static QAction* getMenuAction(U2OpStatus &os, const QString &actionName, const QString &menuName);
    static void clickMenuAction(U2OpStatus &os, const QString &actionName, const QString &menuName);
    static QWidget *findWidgetByName(U2OpStatus &os, const QString &widgetName, QWidget *parentWidget = NULL, bool errorIfNull = true);
};

} //namespace

#endif
