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

#include "GTUtilsApp.h"
#include <U2Core/AppContext.h>
#include <U2Gui/MainWindow.h>
#if (QT_VERSION < 0x050000) //Qt 5
#include <QtGui/QMainWindow>
#else
#include <QtWidgets/QMainWindow>
#endif

namespace U2 {

#define GT_CLASS_NAME "GTUtilsApp"

#define GT_METHOD_NAME "checkUGENETitle"
void GTUtilsApp::checkUGENETitle(U2OpStatus &os, const QString &title) {

    QMainWindow* w = AppContext::getMainWindow()->getQMainWindow();
    GT_CHECK(w, "main window is NULL");
    QString ugeneTitle = w->windowTitle();
    GT_CHECK(ugeneTitle == title, "UGENE title is <" + ugeneTitle + ">, not <" + title + ">");
}

#undef GT_METHOD_NAME

#undef GT_CLASS_NAME

#define GT_CLASS_NAME "GTUtilsApp"

#define GT_METHOD_NAME "checkUGENETitleContains"
void GTUtilsApp::checkUGENETitleContains(U2OpStatus &os, const QString& string) {
    QMainWindow* w = AppContext::getMainWindow()->getQMainWindow();
    GT_CHECK(w, "main window is NULL");
    QString ugeneTitle = w->windowTitle();
    GT_CHECK(ugeneTitle.contains(string), "UGENE title is <" + ugeneTitle + ">, and it not contains <" + string + ">");
}
#undef GT_METHOD_NAME

#undef GT_CLASS_NAME

}
