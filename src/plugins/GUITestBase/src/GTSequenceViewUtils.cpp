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
#include <U2Core/AppContext.h>
#include <U2View/PanView.h>
#include <QClipboard>
#include <QtGui/QApplication>


namespace U2 {


#define GT_CLASS_NAME "GTSequenceViewUtils"
#define GT_METHOD_NAME "getSequenceAsString"

QString GTSequenceViewUtils::getSequenceAsString(U2OpStatus &os)
{
    QMainWindow *mainWindow = AppContext::getMainWindow()->getQMainWindow();
    GT_CHECK_RESULT(mainWindow != NULL, "Main window not found", QString());

    PanViewRenderArea *renderArea =  (mainWindow->findChild<PanViewRenderArea*>());
    GT_CHECK_RESULT(renderArea != NULL, "PanViewRenderArea not found", QString());

    GTMouseDriver::moveTo(os, renderArea->mapToGlobal(renderArea->rect().center()));
    GTMouseDriver::click(os);

    GTUtilsDialog::selectSequenceDialogFiller filler(os);
    GTUtilsDialog::preWaitForDialog(os, &filler);

    GTKeyboardDriver::keyClick(os, 'a', GTKeyboardDriver::key["ctrl"]);
    GTGlobals::sleep(1000);
    GTKeyboardDriver::keyClick(os, 'c', GTKeyboardDriver::key["ctrl"]);

    GTGlobals::sleep(1000);
    return QApplication::clipboard()->text();
}
#undef GT_METHOD_NAME
#undef GT_CLASS_NAME

} // namespace U2
