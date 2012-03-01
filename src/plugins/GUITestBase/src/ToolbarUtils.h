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

#ifndef _U2_GUI_TOOLBAR_UTILS_H_
#define _U2_GUI_TOOLBAR_UTILS_H_

#include <U2Core/U2OpStatus.h>
#include <QtGui/QToolBar>

namespace U2 {

class ToolbarUtils {
public:
   
    static QToolBar* getToolbar(U2OpStatus &os, const QString &toolbarSysName);

    static void clickToolbarAction(U2OpStatus &os,const QString &actionName, const QToolBar *toolbar);

    static void hoverToolbarAction(U2OpStatus &os,const QString &actionName, const QToolBar *toolbar);


    //static void? clickButtonOnToolbar(QToolBar* tb, QString buttonSysName

private:

    static QAction* getToolbarAction(U2OpStatus &os, const QString &actionName, const QToolBar *toolbar);




};

} // namespace

#endif
