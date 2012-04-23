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

#include "GTSequenceViewUtils.h"
#include "api/GTGlobals.h"
#include "api/GTMouseDriver.h"
#include "api/GTKeyboardDriver.h"
#include <QtGui/QMainWindow>
#include "U2Gui/MainWindow.h"
#include "GTUtilsDialog.h"
#include "GTUtilsProjectTreeView.h"
#include <U2Core/AppContext.h>
#include <U2View/DetView.h>
#include <QClipboard>
#include <QtGui/QApplication>

namespace U2 {

#define GT_CLASS_NAME "GTSequenceViewUtils"
#define GT_METHOD_NAME "getSequenceAsString"

QString GTSequenceViewUtils::getSequenceAsString(U2OpStatus &os)
{
    MainWindow* mw = AppContext::getMainWindow();
    GT_CHECK_RESULT(mw != NULL, "MainWindow == NULL", NULL);

    MWMDIWindow *mdiWindow = mw->getMDIManager()->getActiveWindow();
    GT_CHECK_RESULT(mdiWindow != NULL, "MDI window == NULL", NULL);

    GTMouseDriver::moveTo(os, mdiWindow->mapToGlobal(mdiWindow->rect().center()));
    GTMouseDriver::click(os);

    GTUtilsDialog::selectSequenceRegionDialogFiller filler(os);
    GTUtilsDialog::preWaitForDialog(os, &filler);

    GTKeyboardDriver::keyClick(os, 'a', GTKeyboardDriver::key["ctrl"]);
    GTGlobals::sleep(1000);
    GTMouseDriver::moveTo(os, mdiWindow->mapToGlobal(mdiWindow->rect().center()));
    GTMouseDriver::click(os);
    GTKeyboardDriver::keyClick(os, 'c', GTKeyboardDriver::key["ctrl"]);
    GTGlobals::sleep(1000);

    return QApplication::clipboard()->text();
}
#undef GT_METHOD_NAME

#define GT_METHOD_NAME "getSequenceAsString"

QString GTSequenceViewUtils::getLeftOfSequenceAsString(U2OpStatus &os, int length)
{
    MainWindow* mw = AppContext::getMainWindow();
    GT_CHECK_RESULT(mw != NULL, "MainWindow == NULL", NULL);

    MWMDIWindow *mdiWindow = mw->getMDIManager()->getActiveWindow();
    GT_CHECK_RESULT(mdiWindow != NULL, "MDI window == NULL", NULL);

    GTMouseDriver::moveTo(os, mdiWindow->mapToGlobal(mdiWindow->rect().center()));
    GTMouseDriver::click(os);

    GTUtilsDialog::selectSequenceRegionDialogFiller filler(os, 1, length);
    GTUtilsDialog::preWaitForDialog(os, &filler);

    GTKeyboardDriver::keyClick(os, 'a', GTKeyboardDriver::key["ctrl"]);
    GTGlobals::sleep(1000);
    GTGlobals::sleep(500); // don't touch!!!
    GTKeyboardDriver::keyClick(os, 'c', GTKeyboardDriver::key["ctrl"]);
    GTGlobals::sleep(500);

    return QApplication::clipboard()->text();
}
#undef GT_METHOD_NAME

#define GT_METHOD_NAME "checkSequence"

void GTSequenceViewUtils::checkSequence(U2OpStatus &os, const QString &expectedSequence)
{
    QString actualSequence = getSequenceAsString(os);

    GT_CHECK(expectedSequence == actualSequence, "Actual sequence does not match with expected sequence");
}
#undef GT_METHOD_NAME

#define GT_METHOD_NAME "openSequenceView"
void GTSequenceViewUtils::openSequenceView(U2OpStatus &os, const QString &sequenceName){
	QPoint itemPos = GTUtilsProjectTreeView::getItemCenter(os, sequenceName);
	GTMouseDriver::moveTo(os, itemPos);
	GTMouseDriver::click(os, Qt::RightButton);

	GTUtilsDialog::PopupChooser chooser(os, QStringList() << "submenu_open_view" << "action_open_view", GTGlobals::UseMouse);
	GTUtilsDialog::preWaitForDialog(os, &chooser, GUIDialogWaiter::Popup);
	GTGlobals::sleep(1000);
}
#undef GT_METHOD_NAME

#undef GT_CLASS_NAME

} // namespace U2
