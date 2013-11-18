/**
 * UGENE - Integrated Bioinformatics Tools.
 * Copyright (C) 2008-2013 UniPro <ugene@unipro.ru>
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

#include "api/GTGlobals.h"
#include <QtGui/QAbstractButton>

namespace U2 {

class GTWidget {
public:
    // fails if widget is NULL, not visible or not enabled; if p isNull, clicks on the center of widget
    static void click(U2OpStatus &os, QWidget *w, Qt::MouseButton mouseButton = Qt::LeftButton, QPoint p = QPoint());

    // fails if widget is NULL, GTWidget::click fails or widget hasn't got focus
    static void setFocus(U2OpStatus &os, QWidget *w);

    // finds widget with the given object name using given FindOptions. Parent widget is QMainWindow, if not set
    static QWidget *findWidget(U2OpStatus &os, const QString &widgetName, QWidget *parentWidget = NULL, const GTGlobals::FindOptions& = GTGlobals::FindOptions());

    static QAbstractButton *findButtonByText(U2OpStatus &os, const QString &text, QWidget *parentWidget = NULL, const GTGlobals::FindOptions& = GTGlobals::FindOptions());
    //this method writes info about all widgets to opStatus
    static void getAllWidgetsInfo(U2OpStatus &os, QWidget* parent=NULL);
};

} //namespace

#endif
