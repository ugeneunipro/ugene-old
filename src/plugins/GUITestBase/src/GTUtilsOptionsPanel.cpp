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

#include "GTUtilsOptionsPanel.h"
#include "api/GTMouseDriver.h"
#include "api/GTKeyboardDriver.h"
#include "api/GTKeyboardUtils.h"
#include "api/GTWidget.h"
#include "api/GTTreeWidget.h"
#include "GTUtilsTaskTreeView.h"
#include "GTUtilsApp.h"
#include <U2Core/ProjectModel.h>
#include <U2Core/U2OpStatus.h>
#include <U2Gui/MainWindow.h>
#include <QtGui/QMainWindow>
#include <QtGui/QTreeWidget>
#include <QtGui/QApplication>

namespace U2 {

#define GT_CLASS_NAME "GTUtilsOptionsPanel"


#define GT_METHOD_NAME "runFindPatternWithHotKey"
void GTUtilsOptionsPanel::runFindPatternWithHotKey( const QString& pattern, U2OpStatus& os){
    GTKeyboardDriver::keyClick(os, 'f', GTKeyboardDriver::key["ctrl"]);
    GTGlobals::sleep();

    QWidget *w = QApplication::focusWidget();
    GT_CHECK(w && w->objectName()=="textPattern", "Focus is not on FindPattern widget");

    GTKeyboardDriver::keySequence(os, pattern);
    GTGlobals::sleep(1000);
    GTKeyboardDriver::keyClick(os, GTKeyboardDriver::key["enter"]);
}
#undef GT_METHOD_NAME

#undef GT_CLASS_NAME

}
