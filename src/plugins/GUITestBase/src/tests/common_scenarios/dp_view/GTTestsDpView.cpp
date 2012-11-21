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
#include "GTTestsDpView.h"
#include "api/GTMouseDriver.h"
#include "api/GTKeyboardDriver.h"
#include "api/GTKeyboardUtils.h"
#include "api/GTWidget.h"
#include "api/GTFileDialog.h"
#include "api/GTMenu.h"
#include "api/GTTreeWidget.h"
#include "api/GTGlobals.h"
#include "api/GTMouseDriver.h"
#include "GTUtilsApp.h"
#include "GTUtilsDocument.h"
#include "GTUtilsProjectTreeView.h"
#include "GTUtilsAnnotationsTreeView.h"
#include "GTUtilsSequenceView.h"
#include "GTUtilsDialog.h"
#include "runnables/qt/PopupChooser.h"
#include "runnables/ugene/plugins/dotplot/BuildDotPlotDialogFiller.h"
#include "runnables/ugene/plugins/dotplot/DotPlotDialogFiller.h"
#include "runnables/qt/MessageBoxFiller.h"

namespace U2 {

void EscClicker::run()
{
    GTGlobals::sleep(1000);
    GTKeyboardDriver::keyClick(os,GTKeyboardDriver::key["esc"]);
}
namespace GUITest_Common_scenarios_dp_view {

GUI_TEST_CLASS_DEFINITION(test_0011){
//1. Use menu {Tools->Build Dot plot}.
//Expected state: dialog "Build dotplot from sequences" has appear.
//2. Fill the next fields in dialog:
//    {File with first sequence} _common_data/scenarios/dp_view/dp1.fa
//    {File with second sequence} _common_data/scenarios/dp_view/dp2.fa
//3. Click Next button
//Expected state: dialog "Dotplot" has appear.
//4. Fill the next fields in dialog:
//    {Minimum repeat length} 8bp
//    {repeats identity} 80%
//5. Click OK button
    QMenu *menu;
    menu=GTMenu::showMainMenu(os, MWMENU_TOOLS);

    GTUtilsDialog::waitForDialog(os, new DotPlotFiller(os, 8, 80,false,false));
    Runnable *filler2 = new BuildDotPlotFiller(os, testDir + "_common_data/scenarios/dp_view/dp1.fa", testDir + "_common_data/scenarios/dp_view/dp2.fa");
    GTUtilsDialog::waitForDialog(os, filler2);

    GTMenu::clickMenuItem(os, menu, QStringList() << "Build dotplot");
//Expected state: Dot plot view has appear. There is 1 line at view.
//6. Use context menu on dot plot view {Dotplot->Remove}
//Expected state: save "Dotplot" has appear.
//7. Click No button
    GTUtilsDialog::waitForDialog(os, new PopupChooser(os, QStringList() << "Dotplot"<<"Remove"));
    GTUtilsDialog::waitForDialog(os, new MessageBoxDialogFiller(os, QMessageBox::No));
    GTMenu::showContextMenu(os,GTWidget::findWidget(os,"dotplot widget"));

//Expected state: Dot plot view has closed.
    GTGlobals::FindOptions options;
    options.failIfNull = false;
    QWidget *w=GTWidget::findWidget(os,"dotplot widget",NULL, options);

    CHECK_SET_ERR(w==NULL, "Dotplot not deleted");

}

GUI_TEST_CLASS_DEFINITION(test_0013) {

    QMenu *menu;
    menu=GTMenu::showMainMenu(os, MWMENU_TOOLS);

    GTUtilsDialog::waitForDialog(os, new DotPlotFiller(os, 100));
    Runnable *filler2 = new BuildDotPlotFiller(os, testDir + "_common_data/scenarios/dp_view/NC_014267.gb", "secondparametrTest",true,true,false,10);
    GTUtilsDialog::waitForDialog(os, filler2);

    GTMenu::clickMenuItem(os, menu, QStringList() << "Build dotplot");
    GTGlobals::sleep();

}

GUI_TEST_CLASS_DEFINITION(test_0014) {
//1. Use menu {Tools->Build Dot plot}.
//Expected state: dialog "Build dotplot from sequences" has appeared.

//2. Fill the following fields in the dialog:
//    {File with first sequence} _common_data/scenarios/dp_view/dp1.fa
//    {Compare sequence againist itself} set checked
//3. Click Next button
//Expected state: dialog "Dotplot" has appeared.

//4. Fill the following fields in the dialog:
//    {Minimum repeat length} 4bp

//5. Click OK button
    QMenu *menu;
    menu=GTMenu::showMainMenu(os, MWMENU_TOOLS);

    GTUtilsDialog::waitForDialog(os, new DotPlotFiller(os, 4));
    Runnable *filler2 = new BuildDotPlotFiller(os, testDir + "_common_data/scenarios/dp_view/dp1.fa","",false,true);
    GTUtilsDialog::waitForDialog(os, filler2);

    GTMenu::clickMenuItem(os, menu, QStringList() << "Build dotplot");
    GTUtilsProjectTreeView::openView(os);

//6. Call dotplot context menu
//7. Alt-Tab or activate another view

//8. Return to dotplot view and call context menu again

//9. Repeat operation 3-4 times
//Expected state: menu repaints correctly, UGENE not crashed
    for(int i=0;i<4;i++){
        GTUtilsDialog::waitForDialog(os, new EscClicker(os));

        GTWidget::click(os, GTWidget::findWidget(os, GTUtilsProjectTreeView::widgetName));
        QWidget* dpWidget = GTWidget::findWidget(os, "dotplot widget");
        CHECK_SET_ERR(dpWidget != NULL, "no dpWidget");

        GTMenu::showContextMenu(os, dpWidget);
    }
}

GUI_TEST_CLASS_DEFINITION(test_0020) {
//1. Use menu {Tools->Build Dot plot}.
//Expected state: dialog "Build dotplot from sequences" has appeared.
//2. Fill the following fields in the dialog:
//    {File with first sequence} trunk\data\samples\PDB\1CF7.PDB
//    {Compare sequence against itself} checked
//3. Click Next button
//Expected state: dialog "Dotplot" has appeared.
//4. Press the "1k" button
//Expected state: minimum repeat length changed to 2bp
//5. Click OK button
//Expected state: Dotplot view has appeared.
    QMenu *menu;
    menu=GTMenu::showMainMenu(os, MWMENU_TOOLS);

    GTUtilsDialog::waitForDialog(os, new DotPlotFiller(os,100,0, false,true));
    Runnable *filler2 = new BuildDotPlotFiller(os, dataDir + "samples/PDB/1CF7.PDB","",false,true);
    GTUtilsDialog::waitForDialog(os, filler2);

    GTMenu::clickMenuItem(os, menu, QStringList() << "Build dotplot");
    GTUtilsProjectTreeView::openView(os);
//6. Click on the Dotplot view
    QPoint mouse_pos;
    QRect ground_widget;

    mouse_pos = QCursor::pos();
    QWidget *ground;
    ground=GTWidget::findWidget(os,"dotplot widget");
    ground_widget = ground->geometry();
    ground_widget = QRect(ground->mapToGlobal(ground_widget.topLeft()), ground->mapToGlobal(ground_widget.bottomRight()));

    GTMouseDriver::moveTo(os, ground_widget.center());
    GTMouseDriver::click(os);
//Expected state: Dotplot view has been selected, UGENE didn't crash
}
} // namespace GUITest_Assembly_browser_
} // namespace U2


