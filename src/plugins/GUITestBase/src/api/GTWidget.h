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

#ifndef _U2_GUI_GTWIDGET_H_
#define _U2_GUI_GTWIDGET_H_

#include <U2Core/U2OpStatus.h>
#include <U2Core/AppContext.h>
#include <U2Gui/MainWindow.h>
#include <QtGui/QWidget>


namespace U2 {

class GTWidget {
public:

    //finders
    static QWidget* findWidgetByName(U2OpStatus &os, const QString &widgetName, QWidget *parentWidget, bool recursive = true);

    //mouse positioning - this or GTMouse?
    static void moveMouseToWidget(U2OpStatus &os, QWidget *widget);
    static void moveMouseOutOfWidget(U2OpStatus &os, QWidget *widget);

    //interactions
    static void click(U2OpStatus &os, QWidget *widget);
    
    
protected:
    

};

} //namespace

#endif
