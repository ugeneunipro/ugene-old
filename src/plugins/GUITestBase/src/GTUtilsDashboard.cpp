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
namespace {
bool compare(QString s1, QString s2, bool exactMatch){
    if(exactMatch){
        return s1==s2;
    }else{
        return s1.contains(s2);
    }
}
}

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
QWebElement GTUtilsDashboard::findElement(U2OpStatus &os, QString text, QString tag, bool exactMatch){
    QWebView* dashboard = getDashboard(os);
    QWebFrame* frame = dashboard->page()->mainFrame();
    foreach (QWebElement el, frame->findAllElements(tag)) {
        QString s = el.toPlainText();
        int width = el.geometry().width();

        if (compare(s, text, exactMatch) && width != 0){
            return el;
        }
    }
    GT_CHECK_RESULT(false, "element with text " + text + " and tag " + tag + " not found", QWebElement());
}
#undef GT_METHOD_NAME

#define GT_METHOD_NAME "checkElement"
void GTUtilsDashboard::checkElement(U2OpStatus &os, QString text, QString tag, bool exists, bool exactMatch){
    const bool found = doesElementExist(os, text, tag, exactMatch);
    if (exists) {
        GT_CHECK(found, "element with text " + text + " and tag " + tag + " not found");
    } else {
        GT_CHECK(!found, "element with text " + text + " and tag " + tag + " unexpectedly found");
    }
}
#undef GT_METHOD_NAME

#define GT_METHOD_NAME "doesElementExist"
bool GTUtilsDashboard::doesElementExist(U2OpStatus &os, const QString &text, const QString &tag, bool exactMatch) {
    QWebView* dashboard = getDashboard(os);
    QWebFrame* frame = dashboard->page()->mainFrame();

    foreach (QWebElement el, frame->findAllElements(tag)) {
        QString s = el.toPlainText();
        int width = el.geometry().width();

        if (compare(s, text, exactMatch) && width != 0) {
            return true;
        }
    }
    return false;
}
#undef GT_METHOD_NAME

QWebElement GTUtilsDashboard::findTreeElement(U2OpStatus &os, QString text){
    return findElement(os, text, "SPAN");
}

QWebElement GTUtilsDashboard::findContextMenuElement(U2OpStatus &os, QString text){
    return findElement(os, text, "LI");
}

bool GTUtilsDashboard::areThereProblems(U2OpStatus &os) {
    openTab(os, Overview);
    return doesElementExist(os, "Problems", "DIV", true);
}

void GTUtilsDashboard::click(U2OpStatus &os, QWebElement el, Qt::MouseButton button){
    QWebView* dashboard = getDashboard(os);

    GTMouseDriver::moveTo(os, dashboard->mapToGlobal(el.geometry().center()));
    GTMouseDriver::click(os, button);
}

void GTUtilsDashboard::selectElementText(U2OpStatus &os, QWebElement el){
    QWebView* dashboard = getDashboard(os);
    GTMouseDriver::moveTo(os, dashboard->mapToGlobal(el.geometry().topLeft()) + QPoint(5,5));
    GTMouseDriver::press(os);
    GTMouseDriver::moveTo(os, dashboard->mapToGlobal(el.geometry().bottomRight()) - QPoint(5,5));
    GTMouseDriver::release(os);
}

#define GT_METHOD_NAME "openTab"
void GTUtilsDashboard::openTab(U2OpStatus &os, Tabs tab){
    QWebElement el = findElement(os, tabMap.key(tab), "A");
    click(os, el);
}

#undef GT_METHOD_NAME

void GTUtilsDashboard::traceAllWebElements(U2OpStatus &os){
    QWebView* dashboard = GTWidget::findExactWidget<QWebView*>(os, "Dashboard");
    QWebFrame* frame = dashboard->page()->mainFrame();
    QWebElement result;
    foreach (QWebElement el, frame->findAllElements("*")) {
        QString s = el.toPlainText();
        QString tagName = el.tagName();
        QString localName = el.localName();
        QString rect = QString("%1").arg(el.geometry().width());

        if(rect != "0"){
            uiLog.trace("tag: " + tagName + " name: " + localName + " text: " + s + " width: " + rect);
        }
        if (s == "Input"){
            result = el;
        }
    }

}

#undef GT_CLASS_NAME
}

//    QWebView* dashboard = GTWidget::findExactWidget<QWebView*>(os, "Dashboard");
//    QWebFrame* frame = dashboard->page()->mainFrame();
//    int num = frame->findAllElements("*").count();
//    QWebElement result;
//    foreach (QWebElement el, frame->findAllElements("*")) {
//        QString s = el.toPlainText();
//        QString tagName = el.tagName();
//        QString localName = el.localName();
//        QString rect = QString("%1").arg(el.geometry().width());

//        if(rect != "0"){
//            uiLog.trace("tag: " + tagName + " name: " + localName + " text: " + s + " width: " + rect);
//        }
//        if (s == "Input"){
//            result = el;
//        }
//    }
