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

#include "GTUtilsMdi.h"
#include <api/GTMouseDriver.h>
#include <U2Gui/MainWindow.h>
#include <U2Core/AppContext.h>
#include <QtGui/QMainWindow>
#include <QtGui/QMdiSubWindow>
#include <QtGui/QMenu>
#include <QtGui/QApplication>

namespace U2 {

#define GT_CLASS_NAME "GTUtilsMdi"

#define GT_METHOD_NAME "click"
void GTUtilsMdi::click(U2OpStatus &os, GTGlobals::WindowAction action) {

	MainWindow* mw = AppContext::getMainWindow();
	GT_CHECK(mw != NULL, "MainWindow == NULL");

	QMainWindow* mainWindow = mw->getQMainWindow();
	GT_CHECK(mainWindow != NULL, "QMainWindow == NULL");

	GTMenuBar::clickCornerMenu(os, mainWindow->menuBar(), action);
}
#undef GT_METHOD_NAME

#define GT_METHOD_NAME "findWindow"
QWidget* GTUtilsMdi::findWindow(U2OpStatus &os, const QString& windowName, const GTGlobals::FindOptions& options) {

	GT_CHECK_RESULT(windowName.isEmpty() == false, "windowname is empty", NULL);

	MainWindow* mw = AppContext::getMainWindow();
	GT_CHECK_RESULT(mw != NULL, "MainWindow == NULL", NULL);

	QList<MWMDIWindow*> mdiWindows = mw->getMDIManager()->getWindows();
	foreach (MWMDIWindow* w, mdiWindows) {
		QString mdiTitle = w->windowTitle();
		if (mdiTitle == windowName) {
			return w;
		}
	}

	if (options.failIfNull) {
		GT_CHECK_RESULT(false, "Widget " + windowName + " not found", NULL);
	}

	return NULL;
}
#undef GT_METHOD_NAME

#define GT_METHOD_NAME "activeWindow"
QWidget* GTUtilsMdi::activeWindow(U2OpStatus &os, const GTGlobals::FindOptions& options) {

	MainWindow* mw = AppContext::getMainWindow();
	GT_CHECK_RESULT(mw != NULL, "MainWindow == NULL", NULL);

	QWidget* w = mw->getMDIManager()->getActiveWindow();
	if (options.failIfNull) {
		GT_CHECK_RESULT(w != NULL, "Active window is not found", NULL);
	}
	return w;
}
#undef GT_METHOD_NAME

#define GT_METHOD_NAME "getMdiItemPosition"
QPoint GTUtilsMdi::getMdiItemPosition(U2OpStatus &os, const QString& windowName){
	QWidget* w = findWindow(os, windowName);
	GT_CHECK_RESULT(w != NULL, "MDI window not found", QPoint());
	const QRect r = w->rect();
	return w->mapToGlobal(r.center());
}
#undef GT_METHOD_NAME

#define GT_METHOD_NAME "selectRandomRegion"
void GTUtilsMdi::selectRandomRegion(U2OpStatus &os, const QString& windowName){
	QWidget* w = findWindow(os, windowName);
	GT_CHECK(w != NULL, "MDI window not found");
	const QRect r = w->rect();
	QPoint p = QPoint((r.topLeft().x() + r.bottomLeft().x())/2 + 5, r.center().y()/2);
	GTMouseDriver::moveTo(os, w->mapToGlobal(p));
	GTMouseDriver::press(os);
	GTMouseDriver::moveTo(os, w->mapToGlobal(r.center()));
	GTMouseDriver::release(os);
}
#undef GT_METHOD_NAME

#undef GT_CLASS_NAME

}
