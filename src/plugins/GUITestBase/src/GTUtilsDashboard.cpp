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

#include <QWebElement>
#include <QWebFrame>
#include <QWebView>

#include "api/GTMouseDriver.h"
#include "api/GTWidget.h"

#include "GTUtilsDashboard.h"

namespace U2 {

#define GT_CLASS_NAME "GTUtilsDashboard"
QMap<QString, GTUtilsDashboard::Tabs> GTUtilsDashboard::initTabMap(){
    QMap<QString, GTUtilsDashboard::Tabs> result;
    result.insert("Overview", GTUtilsDashboard::Overview);
    result.insert("Input", GTUtilsDashboard::Input);
    result.insert("External Tools", GTUtilsDashboard::ExternalTools);
    return result;
}

const QMap<QString, GTUtilsDashboard::Tabs> GTUtilsDashboard::tabMap = initTabMap();

QWebView* GTUtilsDashboard::getDashboard(U2OpStatus &os){
    return GTWidget::findExactWidget<QWebView*>(os, "Dashboard");
}

#define GT_METHOD_NAME "findElement"
QWebElement GTUtilsDashboard::findElement(U2OpStatus &os, QString text, QString tag){
    QWebView* dashboard = getDashboard(os);
    QWebFrame* frame = dashboard->page()->mainFrame();
    foreach (QWebElement el, frame->findAllElements(tag)) {
        QString s = el.toPlainText();
        int width = el.geometry().width();

        if (s == text && width != 0){
            return el;
        }
    }
    GT_CHECK_RESULT(false, "element with text " + text + " and tag " + tag + " not found", QWebElement());
}
#undef GT_METHOD_NAME

void GTUtilsDashboard::click(U2OpStatus &os, QWebElement el, Qt::MouseButton button){
    QWebView* dashboard = getDashboard(os);

    GTMouseDriver::moveTo(os, dashboard->mapToGlobal(el.geometry().center()));
    GTMouseDriver::click(os, button);
}
#define GT_METHOD_NAME "openTab"
void GTUtilsDashboard::openTab(U2OpStatus &os, Tabs tab){
    QWebElement el = findElement(os, tabMap.key(tab), "A");
    click(os, el);
}

#undef GT_METHOD_NAME

#undef GT_CLASS_NAME
}
