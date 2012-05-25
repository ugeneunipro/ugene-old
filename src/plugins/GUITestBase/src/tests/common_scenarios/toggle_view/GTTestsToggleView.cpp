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
#include "GTUtilsDialogRunnables.h"
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

    GTUtilsDialogRunnables::SequenceReadingModeSelectorDialogFiller dialog(os);
    GTUtilsDialog::preWaitForDialog(os, &dialog, GUIDialogWaiter::Modal);
    GTUtilsProject::openFiles(os, testDir + "_common_data/scenarios/project/multiple.fa");
    GTGlobals::sleep(1000);

    QWidget *overViewSe1 = GTWidget::findWidget(os, "overview_se1");
    QWidget *overViewSe2 = GTWidget::findWidget(os, "overview_se2");
    QWidget *toggleViewButton = GTWidget::findWidget(os, "toggleViewButton");
    QWidget *toggleViewButtonSe1 = GTWidget::findWidget(os, "toggle_view_button_se1");
    QWidget *toggleViewButtonSe2 = GTWidget::findWidget(os, "toggle_view_button_se2");

    GTGlobals::sleep(1000);
    GTUtilsDialogRunnables::PopupChooser chooser0(os, QStringList() << "toggleOverview");
    GTUtilsDialog::preWaitForDialog(os, &chooser0, GUIDialogWaiter::Popup);
    GTWidget::click(os, toggleViewButton);
    GTGlobals::sleep(1000);
    GTGlobals::sleep(10000);

    CHECK_SET_ERR(overViewSe1->isHidden() && overViewSe2->isHidden(),
        "panoramical views for both sequences has not been closed");

    GTGlobals::sleep(1000);
    GTUtilsDialogRunnables::PopupChooser chooser1(os, QStringList() << "toggleOverview");
    GTUtilsDialog::preWaitForDialog(os, &chooser1, GUIDialogWaiter::Popup);
    GTWidget::click(os, toggleViewButton);
    GTGlobals::sleep(1000);
    GTGlobals::sleep(10000);

    CHECK_SET_ERR(!overViewSe1->isHidden() && !overViewSe2->isHidden(), 
        "panoramical view for both sequences has not been shown");

}

GUI_TEST_CLASS_DEFINITION(test_0006) {

    GTUtilsDialogRunnables::SequenceReadingModeSelectorDialogFiller dialog(os);
    GTUtilsDialog::preWaitForDialog(os, &dialog, GUIDialogWaiter::Modal);
    GTUtilsProject::openFiles(os, testDir + "_common_data/scenarios/project/multiple.fa");
    GTGlobals::sleep(1000);

    QWidget *detailsViewSe1 = GTWidget::findWidget(os, "det_view_se1");
    QWidget *detailsViewSe2 = GTWidget::findWidget(os, "det_view_se2");
    QWidget *toolBarSe1 = GTWidget::findWidget(os, "tool_bar_se1");
    QWidget *toolBarSe2 = GTWidget::findWidget(os, "tool_bar_se2");
    QWidget *toggleViewButton = GTWidget::findWidget(os, "toggleViewButton");
    QWidget *toggleViewButtonSe2 = GTWidget::findWidget(os, "toggle_view_button_se2");
    QWidget *toggleViewButtonSe1 = GTWidget::findWidget(os, "toggle_view_button_se1");
    GTGlobals::sleep(1000);

    GTUtilsDialogRunnables::PopupChooser chooser0(os, QStringList() << "toggleDetailsView");
    GTUtilsDialog::preWaitForDialog(os, &chooser0, GUIDialogWaiter::Popup);
    GTWidget::click(os, toggleViewButton);
    GTGlobals::sleep(1000);
    GTGlobals::sleep(10000);

    CHECK_SET_ERR(detailsViewSe1->isHidden() && detailsViewSe2->isHidden(),
        "details views for both sequences has not been closed");
    CHECK_SET_ERR(!toolBarSe1->isHidden() && !toolBarSe2->isHidden(), 
        "toolbars for both sequences has not been shown");

    GTUtilsDialogRunnables::PopupChooser chooser1(os, QStringList() << "toggleDetailsView");
    GTUtilsDialog::preWaitForDialog(os, &chooser1, GUIDialogWaiter::Popup);
    GTWidget::click(os, toggleViewButton);
    GTGlobals::sleep(1000);
    GTGlobals::sleep(10000);

    CHECK_SET_ERR(!detailsViewSe1->isHidden() && !detailsViewSe2->isHidden(), 
        "details view for both sequences has not been shown");

}

GUI_TEST_CLASS_DEFINITION(test_0007) {


    GTUtilsDialogRunnables::SequenceReadingModeSelectorDialogFiller dialog(os);
    GTUtilsDialog::preWaitForDialog(os, &dialog, GUIDialogWaiter::Modal);
    GTUtilsProject::openFiles(os, testDir + "_common_data/scenarios/project/multiple.fa");
    GTGlobals::sleep(1000);

    QWidget *overViewSe1 = GTWidget::findWidget(os, "overview_se1");
    QWidget *overViewSe2 = GTWidget::findWidget(os, "overview_se2");
    QWidget *DetailsViewSe1 = GTWidget::findWidget(os, "det_view_se1");
    QWidget *DetailsViewSe2 = GTWidget::findWidget(os, "det_view_se2");
    QWidget *zoomViewSe1 = GTWidget::findWidget(os, "pan_view_se1");
    QWidget *zoomViewSe2 = GTWidget::findWidget(os, "pan_view_se2");
    QWidget *toolBarSe1 = GTWidget::findWidget(os, "tool_bar_se1");
    QWidget *toolBarSe2 = GTWidget::findWidget(os, "tool_bar_se2");
    QWidget *toggleViewButton = GTWidget::findWidget(os, "toggleViewButton");
    QWidget *toggleViewButtonSe1 = GTWidget::findWidget(os, "toggle_view_button_se1");
    QWidget *toggleViewButtonSe2 = GTWidget::findWidget(os, "toggle_view_button_se2");
    GTGlobals::sleep(1000);

    GTUtilsDialogRunnables::PopupChooser chooser0(os, QStringList() << "toggleAllSequenceViews");
    GTUtilsDialog::preWaitForDialog(os, &chooser0, GUIDialogWaiter::Popup);
    GTWidget::click(os, toggleViewButton);
    GTGlobals::sleep(1000);
    GTGlobals::sleep(10000);

    CHECK_SET_ERR(overViewSe1->isHidden() && overViewSe2->isHidden(),
        "panoramical views for both sequences has not been closed");
    CHECK_SET_ERR(DetailsViewSe1->isHidden() && DetailsViewSe2->isHidden(),
        "details views for both sequences has not been closed");
    CHECK_SET_ERR(zoomViewSe1->isHidden() && zoomViewSe2->isHidden(),
        "zoom views for both sequences has not been closed");
    CHECK_SET_ERR(!toolBarSe1->isHidden() && !toolBarSe2->isHidden(), 
        "toolbars view for both sequences has not been shown");

    GTUtilsDialogRunnables::PopupChooser chooser1(os, QStringList() << "toggleAllSequenceViews");
    GTUtilsDialog::preWaitForDialog(os, &chooser1, GUIDialogWaiter::Popup);
    GTWidget::click(os, toggleViewButton);
    GTGlobals::sleep(1000);
    GTGlobals::sleep(10000);

    CHECK_SET_ERR(!overViewSe1->isHidden() && !overViewSe2->isHidden(), 
        "panoramical view for both sequences has not been shown");
    CHECK_SET_ERR(!DetailsViewSe1->isHidden() && !DetailsViewSe2->isHidden(), 
        "details view for both sequences has not been shown");
    CHECK_SET_ERR(!zoomViewSe1->isHidden() && !zoomViewSe2->isHidden(), 
        "zoom view for both sequences has not been shown");

}

GUI_TEST_CLASS_DEFINITION(test_0008)
{
    GTUtilsDialogRunnables::SequenceReadingModeSelectorDialogFiller dialog(os);
    GTUtilsDialog::preWaitForDialog(os, &dialog, GUIDialogWaiter::Modal, 10000);

    GTFileDialog::openFile(os, testDir + "_common_data/scenarios/project/", "multiple.fa");
    GTGlobals::sleep(1000);
    GTGlobals::sleep(1000);

    QWidget *mainWindow = AppContext::getMainWindow()->getQMainWindow();
    QToolBar *toolBarSe2 = mainWindow->findChild<QToolBar*>("tool_bar_se2");
    CHECK_SET_ERR(mainWindow != NULL, "Main Window not found");
    CHECK_SET_ERR(toolBarSe2 != NULL, "Tool bar not found");

    QAction *zoomAction = mainWindow->findChild<QAction*>("zoom_to_range_se2");
    CHECK_SET_ERR(zoomAction != NULL, "Zoom to range action not found");
    QWidget *zoomButton = toolBarSe2->widgetForAction(zoomAction);
    CHECK_SET_ERR(zoomButton != NULL, "Zoom button not found");

    QAction *zoomInSe2 = mainWindow->findChild<QAction*>("action_zoom_in_se2");
    QAction *zoomOutSe2 = mainWindow->findChild<QAction*>("action_zoom_out_se2");
    CHECK_SET_ERR(zoomInSe2 != NULL, "Zoom In action not found");
    CHECK_SET_ERR(zoomOutSe2 != NULL, "Zoom Out action not found");

    QWidget *zoomInButton = toolBarSe2->widgetForAction(zoomInSe2);
    QWidget *zoomOutButton = toolBarSe2->widgetForAction(zoomInSe2);

    CHECK_SET_ERR(zoomInButton != NULL, "Zoom In button not found");
    CHECK_SET_ERR(zoomOutButton != NULL, "Zoom Out button not found");

    GTUtilsDialogRunnables::ZoomToRangeDialogFiller filler(os, 20, 50);
    GTUtilsDialog::preWaitForDialog(os, &filler);
    GTWidget::click(os, zoomButton);
    GTGlobals::sleep(1000);
    GTGlobals::sleep(1000);

    //  TODO: Expected state: sequence overview showh 20..50 sequence region
    CHECK_SET_ERR(zoomInButton->isEnabled(), "Zoom In button is not enabled");
    CHECK_SET_ERR(zoomOutButton->isEnabled(), "Zoom Out button is not enabled");

    QWidget *toggleViewButtonSe2 = GTWidget::findWidget(os, "toggle_view_button_se2");
    CHECK_SET_ERR(toggleViewButtonSe2 != NULL, "Toggle View button button not found");

    GTUtilsDialogRunnables::PopupChooser chooser0(os, QStringList() << "show_hide_zoom_view");
    GTUtilsDialog::preWaitForDialog(os, &chooser0, GUIDialogWaiter::Popup);
    GTWidget::click(os, toggleViewButtonSe2);
    GTGlobals::sleep(1000);
    GTGlobals::sleep(1000);

    QWidget *zoomViewSe2 = GTWidget::findWidget(os, "pan_view_se2");
    CHECK_SET_ERR(zoomViewSe2 != NULL, "Zoom View widget not found");
    CHECK_SET_ERR(zoomViewSe2->isHidden(), "panoramical view for se2 sequence has been not closed");
    CHECK_SET_ERR(!zoomInButton->isEnabled(), "Zoom In button is enabled");
    CHECK_SET_ERR(!zoomOutButton->isEnabled(), "Zoom Out button is enabled");

    GTUtilsDialog::preWaitForDialog(os, &chooser0, GUIDialogWaiter::Popup);
    GTWidget::click(os, toggleViewButtonSe2);
    GTGlobals::sleep(1000);
    GTGlobals::sleep(1000);

    CHECK_SET_ERR(!zoomViewSe2->isHidden(), "panoramical view for se2 sequence has been not appeared");
    CHECK_SET_ERR(zoomInButton->isEnabled(), "Zoom In button is not enabled");
    CHECK_SET_ERR(zoomOutButton->isEnabled(), "Zoom Out button is not enabled");
}

GUI_TEST_CLASS_DEFINITION(test_0009)
{
    GTUtilsDialogRunnables::SequenceReadingModeSelectorDialogFiller dialog(os);
    GTUtilsDialog::preWaitForDialog(os, &dialog, GUIDialogWaiter::Modal);
    GTUtilsProject::openFiles(os, testDir + "_common_data/scenarios/project/multiple.fa");
    GTGlobals::sleep(1000);


    QWidget *overViewSe1 = GTWidget::findWidget(os, "overview_se1");
    QWidget *overViewSe2 = GTWidget::findWidget(os, "overview_se2");

    QWidget *toggleViewButton = GTWidget::findWidget(os, "toggleViewButton");
    QWidget *toggleViewButtonSe2 = GTWidget::findWidget(os, "toggle_view_button_se2");

    GTUtilsDialogRunnables::PopupChooser chooser0(os, QStringList() << "show_hide_overview");
    GTUtilsDialog::preWaitForDialog(os, &chooser0, GUIDialogWaiter::Popup);
    GTWidget::click(os, toggleViewButtonSe2);
    GTGlobals::sleep(1000);
    GTGlobals::sleep(1000);
    CHECK_SET_ERR(overViewSe2->isHidden(), "panoramical view for se2 sequence has been not closed");

    GTUtilsDialogRunnables::PopupChooser chooser1(os, QStringList() << "toggleOverview");
    GTUtilsDialog::preWaitForDialog(os, &chooser1, GUIDialogWaiter::Popup);
    GTWidget::click(os, toggleViewButton);
    GTGlobals::sleep(1000);
    GTGlobals::sleep(1000);
    CHECK_SET_ERR(overViewSe1->isHidden() && overViewSe2->isHidden(),
                  "panoramical views for both sequences has been not closed");

    GTUtilsDialogRunnables::PopupChooser chooser2(os, QStringList() << "show_hide_overview");
    GTUtilsDialog::preWaitForDialog(os, &chooser2, GUIDialogWaiter::Popup);
    GTWidget::click(os, toggleViewButtonSe2);
    GTGlobals::sleep(1000);
    GTGlobals::sleep(1000);
    CHECK_SET_ERR(!overViewSe2->isHidden(), "panoramical view for se2 sequence has been not shown");

    GTUtilsDialogRunnables::PopupChooser chooser3(os, QStringList() << "toggleOverview");
    GTUtilsDialog::preWaitForDialog(os, &chooser3, GUIDialogWaiter::Popup);
    GTWidget::click(os, toggleViewButton);
    GTGlobals::sleep(1000);
    GTGlobals::sleep(10000);
    CHECK_SET_ERR(overViewSe1->isHidden() && overViewSe2->isHidden(),
                  "panoramical view for both sequences has been not closed");
}


GUI_TEST_CLASS_DEFINITION(test_0010)
{
    GTFileDialog::openFile(os, dataDir + "samples/FASTA/", "human_T1.fa");
    GTGlobals::sleep(1000);

    GTUtilsSequenceView::selectSequenceRegion(os, 60000, 70000);
    GTGlobals::sleep(1000);

}

} // namespace
} // namespace U2
