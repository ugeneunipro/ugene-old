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

#include "GTTestsToggleView.h"
#include "api/GTGlobals.h"
#include "api/GTKeyboardDriver.h"
#include "api/GTMouseDriver.h"
#include "api/GTMenu.h"
#include "api/GTFileDialog.h"
#include "api/GTTreeWidget.h"
#include "GTUtilsProject.h"
#include "GTUtilsDocument.h"
#include "GTUtilsLog.h"
#include "GTUtilsApp.h"
#include "GTUtilsToolTip.h"
#include "GTUtilsDialog.h"
#include "GTUtilsMdi.h"
#include "GTUtilsAnnotationsTreeView.h"
#include "GTUtilsProjectTreeView.h"
#include "GTUtilsTaskTreeView.h"
#include "GTUtilsSequenceView.h"
#include "GTUtilsMdi.h"

#include <U2Core/DocumentModel.h>
#include <U2View/AnnotatedDNAViewFactory.h>
#include <U2View/MSAEditorFactory.h>
#include <U2View/ADVConstants.h>
#include <QClipboard>

namespace U2{

namespace GUITest_common_scenarios_toggle_view {

GUI_TEST_CLASS_DEFINITION(test_0005) {

    GTUtilsDialog::SequenceReadingModeSelectorDialogFiller dialog(os);
    GTUtilsDialog::preWaitForDialog(os, &dialog, GUIDialogWaiter::Modal);
    GTUtilsProject::openFiles(os, testDir + "_common_data/scenarios/project/multiple.fa");
    GTGlobals::sleep(1000);

    QWidget *toggleViewButton = GTWidget::findWidget(os, "toggleViewButton");
    GTGlobals::sleep(1000);
    CHECK_SET_ERR(toggleViewButton != NULL, "Toggle View Button is NULL"); 
    GTUtilsDialog::PopupChooser chooser(os, QStringList() << "toggleOverview");
    GTUtilsDialog::preWaitForDialog(os, &chooser, GUIDialogWaiter::Popup);
    GTWidget::click(os, toggleViewButton);
    GTGlobals::sleep(1000);

    QWidget *toggleViewButton1 = GTWidget::findWidget(os, "toggleViewButton");
    GTGlobals::sleep(1000);
    CHECK_SET_ERR(toggleViewButton1 != NULL, "toggleViewButton is NULL"); 
    GTUtilsDialog::preWaitForDialog(os, &chooser, GUIDialogWaiter::Popup);
    GTWidget::click(os, toggleViewButton1);
    GTGlobals::sleep(1000);


}
GUI_TEST_CLASS_DEFINITION(test_0006) {

    GTUtilsDialog::SequenceReadingModeSelectorDialogFiller dialog(os);
    GTUtilsDialog::preWaitForDialog(os, &dialog, GUIDialogWaiter::Modal);
    GTUtilsProject::openFiles(os, testDir + "_common_data/scenarios/project/multiple.fa");
    GTGlobals::sleep(1000);

    QWidget *toggleViewButton = GTWidget::findWidget(os, "toggleViewButton");
    GTGlobals::sleep(1000);
    CHECK_SET_ERR(toggleViewButton != NULL, "Toggle View Button is NULL"); 
    GTUtilsDialog::PopupChooser chooser(os, QStringList() << "toggleDetailsView");
    GTUtilsDialog::preWaitForDialog(os, &chooser, GUIDialogWaiter::Popup);
    GTWidget::click(os, toggleViewButton);
    GTGlobals::sleep(1000);

    QWidget *toggleViewButton1 = GTWidget::findWidget(os, "toggleViewButton");
    GTGlobals::sleep(1000);
    CHECK_SET_ERR(toggleViewButton1 != NULL, "toggleViewButton is NULL"); 
    GTUtilsDialog::preWaitForDialog(os, &chooser, GUIDialogWaiter::Popup);
    GTWidget::click(os, toggleViewButton1);
    GTGlobals::sleep(1000);

    }
GUI_TEST_CLASS_DEFINITION(test_0007) {
    GTUtilsDialog::SequenceReadingModeSelectorDialogFiller dialog(os);
    GTUtilsDialog::preWaitForDialog(os, &dialog, GUIDialogWaiter::Modal);
    GTUtilsProject::openFiles(os, testDir + "_common_data/scenarios/project/multiple.fa");
    GTGlobals::sleep(1000);

    QWidget *toggleViewButton = GTWidget::findWidget(os, "toggleViewButton");
    GTGlobals::sleep(1000);
    CHECK_SET_ERR(toggleViewButton != NULL, "Toggle View Button is NULL"); 
    GTUtilsDialog::PopupChooser chooser(os, QStringList() << "toggleAllSequenceViews");
    GTUtilsDialog::preWaitForDialog(os, &chooser, GUIDialogWaiter::Popup);
    GTWidget::click(os, toggleViewButton);
    GTGlobals::sleep(1000);

    QWidget *toggleViewButton1 = GTWidget::findWidget(os, "toggleViewButton");
    GTGlobals::sleep(1000);
    CHECK_SET_ERR(toggleViewButton1 != NULL, "toggleViewButton is NULL"); 
    GTUtilsDialog::preWaitForDialog(os, &chooser, GUIDialogWaiter::Popup);
    GTWidget::click(os, toggleViewButton1);
    GTGlobals::sleep(1000);

    }

} // namespace
} // namespace U2
