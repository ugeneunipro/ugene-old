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

#ifndef _U2_GT_TABWIDGET_H_
#define _U2_GT_TABWIDGET_H_

#include "GTGlobals.h"

#if (QT_VERSION < 0x050000) //Qt 5
#include <QtGui/QTabWidget>
#else
#include <QtWidgets/QTabWidget>
#endif

namespace U2 {

class GTTabWidget {
public:
    // fails if the tabwidget is NULL, index is not in a tabwidget's range
    // or a tabwidget's index differs from a given index in the end of method's execution
    static void setCurrentIndex(U2OpStatus& os, QTabWidget *tabWidget, int index);
    static QTabBar* getTabBar(U2OpStatus &os, QTabWidget* tabWidget);
    static void clickTab(U2OpStatus &os, QTabWidget* tabWidget, int idx, Qt::MouseButton button = Qt::LeftButton);
    static void clickTab(U2OpStatus &os, QTabWidget* tabWidget, QString tabName, Qt::MouseButton button = Qt::LeftButton);
    static QString getTabName(U2OpStatus &os, QTabWidget* tabWidget, int idx);


};

}
#endif // _U2_GT_TABWIDGET_H_
