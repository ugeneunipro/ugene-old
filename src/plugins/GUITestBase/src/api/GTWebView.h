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



#ifndef GTWEBVIEW_H
#define GTWEBVIEW_H

#include "GTGlobals.h"

class QWebView;
class QWebElement;

namespace U2 {
using namespace HI;

class GTWebView{
public:
    static QWebElement findElement(U2OpStatus &os, QWebView *view, const QString &text, const QString &tag = "*", bool exactMatch = false);
    static void checkElement(U2OpStatus &os, QWebView *view, QString text, QString tag = "*", bool exists = true, bool exactMatch = false);
    static bool doesElementExist(U2OpStatus &os, QWebView *view, const QString &text, const QString &tag = "*", bool exactMatch = false);

    static void click(U2OpStatus &os, QWebView *view, QWebElement el, Qt::MouseButton button = Qt::LeftButton);
    static void selectElementText(U2OpStatus &os, QWebView *view, QWebElement el);
    static QWebElement findTreeElement(U2OpStatus &os, QWebView *view, QString text);
    static QWebElement findContextMenuElement(U2OpStatus &os, QWebView *view, QString text);

    static void traceAllWebElements(U2OpStatus &os, QWebView *view);

};

}

#endif // GTWEBVIEW_H
