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

#include <QWebView>
#include <QWebElement>

#include "api/GTWebView.h"
#include "api/GTWidget.h"

#include "GTUtilsMdi.h"

#include "GTUtilsStartPage.h"

namespace U2 {

void GTUtilsStartPage::openStartPage(U2OpStatus &os){
    GTUtilsMdi::activateWindow(os, "Start Page");
}

QWebView* GTUtilsStartPage::getStartPage(U2OpStatus &os){
    QWidget* startPageWidget = GTWidget::findWidget(os, "Start Page");
    QWebView* result = GTWidget::findExactWidget<QWebView*>(os, "webView", startPageWidget);
    return result;
}

QWebElement GTUtilsStartPage::getButton(U2OpStatus &os, Buttons button){
    QWebElement result;
    switch (button) {
    case OpenFile:
        result = GTWebView::findElement(os, getStartPage(os), "Open File(s)", "DIV");
        break;
    case CreateSequence:
        result = GTWebView::findElement(os, getStartPage(os), "Create Sequence", "DIV");
        break;
    case CreateWorkflow:
        result = GTWebView::findElement(os, getStartPage(os), "Create Workflow", "DIV");
        break;
    case QuickStartGuide:
        //You do not need this button
    default:
        break;
    }

    return result;
}

void GTUtilsStartPage::clickButton(U2OpStatus &os, Buttons b){
    QWebElement but = getButton(os, b);
    QWebView* start = getStartPage(os);
    GTWebView::click(os, start, but);
}

void GTUtilsStartPage::clickResentDocument(U2OpStatus &os, QString text){
    QWebElement el = GTWebView::findElement(os, getStartPage(os), "- " + text, "A");
    GTWebView::click(os, getStartPage(os), el);
}

void GTUtilsStartPage::clickResentProject(U2OpStatus &os, QString text){
    clickResentDocument(os, text);
}

}
