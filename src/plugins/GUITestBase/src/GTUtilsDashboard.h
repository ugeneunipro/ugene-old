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


#ifndef GTUTILSDASHBOARD_H
#define GTUTILSDASHBOARD_H

#include "api/GTGlobals.h"

class QWebView;
class QWebElement;

namespace U2 {

class GTUtilsDashboard
{
public:
    enum Tabs{Overview, Input, ExternalTools};
    static QWebView* getDashboard(U2OpStatus &os);
    static QWebElement findElement(U2OpStatus &os, QString text, QString tag = "*", bool exactMatch = false);
    static void checkElement(U2OpStatus &os, QString text, QString tag = "*", bool exists = true, bool exactMatch = false);
    static void click(U2OpStatus &os, QWebElement el, Qt::MouseButton button = Qt::LeftButton);
    static void selectElementText(U2OpStatus &os, QWebElement el);
    static QWebElement findTreeElement(U2OpStatus &os, QString text);
    static QWebElement findContextMenuElement(U2OpStatus &os, QString text);

    static void openTab(U2OpStatus &os, Tabs tab);

    static void traceAllWebElements(U2OpStatus &os);
private:
    static QMap<QString, Tabs> initTabMap();
    static const QMap<QString, Tabs> tabMap;
};

}

#endif // GTUTILSDASHBOARD_H
