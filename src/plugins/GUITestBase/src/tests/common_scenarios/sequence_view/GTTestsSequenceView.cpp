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

#include "GTTestsSequenceView.h"
#include "api/GTMouseDriver.h"
#include "api/GTGlobals.h"
#include "api/GTWidget.h"
#include "api/GTAction.h"
#include "api/GTMenu.h"
#include "api/GTFileDialog.h"
#include "GTUtilsProject.h"
#include "GTUtilsAnnotationsTreeView.h"
#include "GTUtilsMdi.h"
#include "GTUtilsSequenceView.h"
#include "runnables/qt/PopupChooser.h"
#include "runnables/ugene/plugins/dna_export/ExportSequencesDialogFiller.h"
#include "runnables/ugene/ugeneui/SequenceReadingModeSelectorDialogFiller.h"
#include <U2View/DetView.h>
#include <U2Core/AppContext.h>
#include <U2View/ADVConstants.h>

namespace U2 {

namespace GUITest_common_scenarios_sequence_view {

GUI_TEST_CLASS_DEFINITION(test_0002) {

    GTFileDialog::openFile(os, dataDir+"samples/PDB/", "1CF7.PDB");
    GTGlobals::sleep(3000);

    QWidget *w0 = GTWidget::findWidget(os, "ADV_single_sequence_widget_0");
    QWidget *w1 = GTWidget::findWidget(os, "ADV_single_sequence_widget_1");
    QWidget *w2 = GTWidget::findWidget(os, "ADV_single_sequence_widget_2");
    QWidget *w3 = GTWidget::findWidget(os, "ADV_single_sequence_widget_3");

    QAbstractButton* b0 = GTAction::button(os,"Show circular view", w0);
    QAbstractButton* b1 = GTAction::button(os,"Show circular view", w1);
    QAbstractButton* b2 = GTAction::button(os,"Show circular view", w2);
    QAbstractButton* b3 = GTAction::button(os,"Show circular view", w3);
    GTGlobals::sleep(3000);

    CHECK_SET_ERR(b0 == NULL, "b0 is not NULL");
    CHECK_SET_ERR(b1 == NULL, "b1 is not NULL");
    CHECK_SET_ERR(b2 != NULL, "b2 is NULL");
    CHECK_SET_ERR(b3 != NULL, "b3 is NULL");
}

GUI_TEST_CLASS_DEFINITION(test_0002_1) {

    GTFileDialog::openFile(os, dataDir+"samples/PDB/", "1CF7.PDB");
    GTGlobals::sleep(3000);

    QWidget *w0 = GTWidget::findWidget(os, "ADV_single_sequence_widget_0");
    QWidget *w1 = GTWidget::findWidget(os, "ADV_single_sequence_widget_1");
    QWidget *w2 = GTWidget::findWidget(os, "ADV_single_sequence_widget_2");
    QWidget *w3 = GTWidget::findWidget(os, "ADV_single_sequence_widget_3");

    QAbstractButton* b0 = GTAction::button(os,"complement_action", w0);
    QAbstractButton* b1 = GTAction::button(os,"complement_action", w1);
    QAbstractButton* b2 = GTAction::button(os,"complement_action", w2);
    QAbstractButton* b3 = GTAction::button(os,"complement_action", w3);
    GTGlobals::sleep(3000);

    CHECK_SET_ERR(b0 == NULL, "b0 is not NULL");
    CHECK_SET_ERR(b1 == NULL, "b1 is not NULL");
    CHECK_SET_ERR(b2 != NULL, "b2 is NULL");
    CHECK_SET_ERR(b3 != NULL, "b3 is NULL");

    }
GUI_TEST_CLASS_DEFINITION(test_0002_2) {

    GTFileDialog::openFile(os, dataDir+"samples/PDB/", "1CF7.PDB");
    GTGlobals::sleep(3000);

    QWidget *w0 = GTWidget::findWidget(os, "ADV_single_sequence_widget_0");
    QWidget *w1 = GTWidget::findWidget(os, "ADV_single_sequence_widget_1");
    QWidget *w2 = GTWidget::findWidget(os, "ADV_single_sequence_widget_2");
    QWidget *w3 = GTWidget::findWidget(os, "ADV_single_sequence_widget_3");

    QAbstractButton* b0 = GTAction::button(os,"translation_action", w0);
    QAbstractButton* b1 = GTAction::button(os,"translation_action", w1);
    QAbstractButton* b2 = GTAction::button(os,"translation_action", w2);
    QAbstractButton* b3 = GTAction::button(os,"translation_action", w3);
    GTGlobals::sleep(3000);

    CHECK_SET_ERR(b0 == NULL, "b0 is not NULL");
    CHECK_SET_ERR(b1 == NULL, "b1 is not NULL");
    CHECK_SET_ERR(b2 != NULL, "b2 is NULL");
    CHECK_SET_ERR(b3 != NULL, "b3 is NULL");

    }

GUI_TEST_CLASS_DEFINITION(test_0003) {

    GTFileDialog::openFile(os, dataDir + "samples/FASTA/", "human_T1.fa");
    GTGlobals::sleep();

    QWidget *toggleViewButton = GTWidget::findWidget(os, "toggleViewButton");
    Runnable *chooser = new PopupChooser(os, QStringList() << "toggleDetailsView");
    GTUtilsDialog::waitForDialog(os, chooser);
    GTWidget::click(os, toggleViewButton);
    GTGlobals::sleep();

    GTGlobals::sleep();
    QAction *showTranslationAction = GTAction::findAction(os, "translation_action");
    QAbstractButton* translation = GTAction::button(os, "translation_action");

    CHECK_SET_ERR(translation -> isEnabled() == false, "button is not disabled");

    GTGlobals::sleep();
    QAction *showComplementAction = GTAction::findAction(os, "complement_action");
    QAbstractButton* complement = GTAction::button(os, "complement_action");

    CHECK_SET_ERR(complement -> isEnabled() == false, "button is not disabled");

    Runnable *chooser1 = new PopupChooser(os, QStringList() << "toggleDetailsView");
    GTUtilsDialog::waitForDialog(os, chooser1);
    GTWidget::click(os, toggleViewButton);
    GTGlobals::sleep();

    GTGlobals::sleep();
    QAction *showTranslationAction1 = GTAction::findAction(os, "translation_action");
    QAbstractButton* translation1 = GTAction::button(os, "translation_action");
    GTGlobals::sleep();
    CHECK_SET_ERR(translation1 -> isEnabled() == true, "button is not enabled");


    GTGlobals::sleep();
    QAction *showComplementAction1 = GTAction::findAction(os, "complement_action");
    QAbstractButton* complement1 = GTAction::button(os, "complement_action");

    GTGlobals::sleep();
    CHECK_SET_ERR(complement1 -> isEnabled() == true, "button is not enabled");
    }
GUI_TEST_CLASS_DEFINITION(test_0003_1) {

    GTFileDialog::openFile(os, dataDir + "samples/FASTA/", "human_T1.fa");
    GTGlobals::sleep();

    QWidget *toggleViewButton = GTWidget::findWidget(os, "toggleViewButton");
    Runnable *chooser = new PopupChooser(os, QStringList() << "toggleAllSequenceViews");
    GTUtilsDialog::waitForDialog(os, chooser);
    GTWidget::click(os, toggleViewButton);
    GTGlobals::sleep();

    GTGlobals::sleep();
    QAction *showTranslationAction = GTAction::findAction(os, "translation_action");
    QAbstractButton* translation = GTAction::button(os, "translation_action");

    CHECK_SET_ERR(translation -> isEnabled() == false, "button is not disabled");

    GTGlobals::sleep();
    QAction *showComplementAction = GTAction::findAction(os, "complement_action");
    QAbstractButton* complement = GTAction::button(os, "complement_action");

    CHECK_SET_ERR(complement -> isEnabled() == false, "button is not disabled");

    Runnable *chooser1 = new PopupChooser(os, QStringList() << "toggleAllSequenceViews");
    GTUtilsDialog::waitForDialog(os, chooser1);
    GTWidget::click(os, toggleViewButton);
    GTGlobals::sleep();

    GTGlobals::sleep();
    QAction *showTranslationAction1 = GTAction::findAction(os, "translation_action");
    QAbstractButton* translation1 = GTAction::button(os, "translation_action");

    GTGlobals::sleep();
    CHECK_SET_ERR(translation1 -> isEnabled() == true, "button is not enabled");

    GTGlobals::sleep();
    QAction *showComplementAction1 = GTAction::findAction(os, "complement_action");
    QAbstractButton* complement1 = GTAction::button(os, "complement_action");

    GTGlobals::sleep();
    CHECK_SET_ERR(complement1 -> isEnabled() == true, "button is not enabled");
    }
GUI_TEST_CLASS_DEFINITION(test_0003_2) {
    GTFileDialog::openFile(os, dataDir + "samples/FASTA/", "human_T1.fa");
    GTGlobals::sleep();

    QWidget *toggleViewButton = GTWidget::findWidget(os, "toggleViewButton");
    Runnable *chooser = new PopupChooser(os, QStringList() << "toggleDetailsView");
    GTUtilsDialog::waitForDialog(os, chooser);
    GTWidget::click(os, toggleViewButton);
    GTGlobals::sleep();

    GTGlobals::sleep();
    QAction *showTranslationAction = GTAction::findAction(os, "translation_action");
    QAbstractButton* translation = GTAction::button(os, "translation_action");

    CHECK_SET_ERR(translation -> isEnabled() == false, "button is not disabled");

    GTGlobals::sleep();
    QAction *showComplementAction = GTAction::findAction(os, "complement_action");
    QAbstractButton* complement = GTAction::button(os, "complement_action");

    CHECK_SET_ERR(complement -> isEnabled() == false, "button is not disabled");

    Runnable *chooser1 = new PopupChooser(os, QStringList() << "toggleDetailsView");
    GTUtilsDialog::waitForDialog(os, chooser1);
    GTWidget::click(os, toggleViewButton);
    GTGlobals::sleep();

    GTGlobals::sleep();
    QAction *showTranslationAction1 = GTAction::findAction(os, "translation_action");
    QAbstractButton* translation1 = GTAction::button(os, "translation_action");

    GTGlobals::sleep();
    CHECK_SET_ERR(translation1 -> isEnabled() == true, "button is not enabled");

    GTGlobals::sleep();
    CHECK_SET_ERR(complement -> isEnabled() == true, "button is not disabled");

    Runnable *chooser2 = new PopupChooser(os, QStringList() << "toggleDetailsView");
    GTUtilsDialog::waitForDialog(os, chooser2);
    GTWidget::click(os, toggleViewButton);
    GTGlobals::sleep();

    GTGlobals::sleep();
    CHECK_SET_ERR(translation -> isEnabled() == false, "button is not disabled");

    GTGlobals::sleep();
    QAction *showComplementAction1 = GTAction::findAction(os, "complement_action");
    QAbstractButton* complement1 = GTAction::button(os, "complement_action");

    GTGlobals::sleep();
    CHECK_SET_ERR(complement1 -> isEnabled() == false, "button is not disabled");


    }
GUI_TEST_CLASS_DEFINITION(test_0004) {

    GTFileDialog::openFile(os, testDir + "_common_data/fasta/", "fa1.fa");
    GTGlobals::sleep(1000);

    GTUtilsSequenceView::selectSequenceRegion(os, 1, 3);

    GTUtilsDialog::waitForDialog(os, new PopupChooser(os, QStringList() << "ADV_MENU_EXPORT" << "action_export_selected_sequence_region"));
    GTUtilsDialog::waitForDialog(os, new ExportSelectedRegionFiller(os, testDir + "_common_data/scenarios/sandbox/", "exp.fasta", GTGlobals::UseMouse));
    GTMouseDriver::click(os);
    GTMouseDriver::click(os, Qt::RightButton);
}

GUI_TEST_CLASS_DEFINITION(test_0004_1) { //CHANGES: keyboard used instead mouse

    GTFileDialog::openFile(os, testDir + "_common_data/fasta/", "fa1.fa");
    GTGlobals::sleep(1000);

    GTUtilsSequenceView::selectSequenceRegion(os, 1, 3);

    GTUtilsDialog::waitForDialog(os, new PopupChooser(os, QStringList() << "ADV_MENU_EXPORT" << "action_export_selected_sequence_region"));
    GTUtilsDialog::waitForDialog(os, new ExportSelectedRegionFiller(os, testDir + "_common_data/scenarios/sandbox/", "exp.fasta", GTGlobals::UseKey));
    GTMouseDriver::click(os);
    GTMouseDriver::click(os, Qt::RightButton);
}

GUI_TEST_CLASS_DEFINITION(test_0005) {

    class AllActionsPopupChooser : public PopupChooser {
    public:
        AllActionsPopupChooser(U2OpStatus &_os) : PopupChooser(_os, QStringList()) {}
        void run() {
            GTGlobals::sleep(100);
            QMenu* activePopupMenu = qobject_cast<QMenu*>(QApplication::activePopupWidget());

            int i = 0;
            QList<QAction*> act = activePopupMenu->actions();
            foreach (QAction* a, act) {
                QString s = a->objectName();
                if (i == 1) {
                    GTMenu::clickMenuItem(os, activePopupMenu, s, GTGlobals::UseMouse);
                }
                GTGlobals::sleep(100);
                i++;
            }
        }
    };

    GTUtilsDialog::waitForDialog(os, new SequenceReadingModeSelectorDialogFiller(os));
    GTUtilsProject::openFiles(os, testDir + "_common_data/fasta/multy_fa.fa");
    GTUtilsDocument::checkDocument(os, "multy_fa.fa");

    QWidget* toggleAutoAnnotationsButton = GTWidget::findWidget(os, "toggleAutoAnnotationsButton");
    CHECK_SET_ERR(toggleAutoAnnotationsButton != NULL, "toggleAutoAnnotationsButton is NULL");
    CHECK_SET_ERR(toggleAutoAnnotationsButton->isEnabled() == false, "toggleAutoAnnotationsButton is enabled, expected disabled");


    GTUtilsProject::openFiles(os, dataDir + "samples/FASTA/human_T1.fa");
    GTUtilsDocument::checkDocument(os, "human_T1.fa");

    toggleAutoAnnotationsButton = GTWidget::findWidget(os, "toggleAutoAnnotationsButton");
    CHECK_SET_ERR(toggleAutoAnnotationsButton != NULL, "toggleAutoAnnotationsButton is NULL");
    CHECK_SET_ERR(toggleAutoAnnotationsButton->isEnabled() == true, "toggleAutoAnnotationsButton is disabled, expected enabled");

    GTUtilsDialog::waitForDialog(os, new AllActionsPopupChooser(os));
    GTWidget::click(os, toggleAutoAnnotationsButton);
    GTGlobals::sleep(1000);
}
GUI_TEST_CLASS_DEFINITION(test_0006) {

//1. Enable Auto-annotations
    GTGlobals::sleep();
    GTGlobals::sleep();
    GTFileDialog::openFile(os, dataDir + "samples/FASTA", "human_T1.fa");

    QWidget* toggleAutoAnnotationsButton = GTWidget::findWidget(os, "toggleAutoAnnotationsButton");
//  !!! dirty fastfix of test, very temporary
    QToolBar* tb = qobject_cast<QToolBar*>(toggleAutoAnnotationsButton->parent());
    QToolButton* extensionButton = tb->findChild<QToolButton*>("qt_toolbar_ext_button");
//

    GTUtilsDialog::waitForDialog(os, new PopupChooser(os, QStringList() << "Restriction Sites"));
    GTWidget::click(os, extensionButton);
    GTGlobals::sleep(1000);
    GTWidget::click(os, toggleAutoAnnotationsButton);
    GTGlobals::sleep();

    GTUtilsDialog::waitForDialog(os, new PopupChooser(os, QStringList() << "ORFs"));
    GTWidget::click(os, extensionButton);
    GTGlobals::sleep(1000);
    GTWidget::click(os, toggleAutoAnnotationsButton);
    GTGlobals::sleep();

    //2. Open any PBD file from samples
    //Expected state: there is no auto-annotations for opened sequences

    GTFileDialog::openFile(os, dataDir + "samples/PDB", "1CF7.PDB");

    GTGlobals::sleep();
    GTUtilsAnnotationsTreeView::findItem(os, "enzymes", false);
    GTUtilsAnnotationsTreeView::findItem(os, "orfs", false);
    GTGlobals::sleep();

    }

GUI_TEST_CLASS_DEFINITION(test_0006_1) {
    //1. Enable Auto-annotations
    GTGlobals::sleep();
    GTGlobals::sleep();
    GTFileDialog::openFile(os, dataDir + "samples/FASTA", "human_T1.fa");

    QWidget* toggleAutoAnnotationsButton = GTWidget::findWidget(os, "toggleAutoAnnotationsButton");
//  !!! dirty fastfix of test, very temporary
    QToolBar* tb = qobject_cast<QToolBar*>(toggleAutoAnnotationsButton->parent());
    QToolButton* extensionButton = tb->findChild<QToolButton*>("qt_toolbar_ext_button");
//

    GTUtilsDialog::waitForDialog(os, new PopupChooser(os, QStringList() << "Restriction Sites"));
    GTWidget::click(os, extensionButton);
    GTGlobals::sleep(1000);
    GTWidget::click(os, toggleAutoAnnotationsButton);
    GTGlobals::sleep();

    GTUtilsDialog::waitForDialog(os, new PopupChooser(os, QStringList() << "ORFs"));
    GTWidget::click(os, extensionButton);
    GTGlobals::sleep(1000);
    GTWidget::click(os, toggleAutoAnnotationsButton);
    GTGlobals::sleep();

    //1. Disable Auto-annotations
    GTUtilsDialog::waitForDialog(os, new PopupChooser(os, QStringList() << "Restriction Sites"));
    GTWidget::click(os, extensionButton);
    GTGlobals::sleep(1000);
    GTWidget::click(os, toggleAutoAnnotationsButton);
    GTGlobals::sleep();

    GTUtilsDialog::waitForDialog(os, new PopupChooser(os, QStringList() << "ORFs"));
    GTWidget::click(os, extensionButton);
    GTGlobals::sleep(1000);
    GTWidget::click(os, toggleAutoAnnotationsButton);
    GTGlobals::sleep();

    //3. Enable Auto-annotations
    GTUtilsDialog::waitForDialog(os, new PopupChooser(os, QStringList() << "Restriction Sites"));
    GTWidget::click(os, extensionButton);
    GTGlobals::sleep(1000);
    GTWidget::click(os, toggleAutoAnnotationsButton);
    GTGlobals::sleep();

    GTUtilsDialog::waitForDialog(os, new PopupChooser(os, QStringList() << "ORFs"));
    GTWidget::click(os, extensionButton);
    GTGlobals::sleep(1000);
    GTWidget::click(os, toggleAutoAnnotationsButton);
    GTGlobals::sleep();

    //2. Open any PBD file from samples
    //Expected state: there is no auto-annotations for opened sequences
    GTFileDialog::openFile(os, dataDir + "samples/PDB", "1CF7.PDB");
    GTGlobals::sleep();

    //Expected state: there is no auto-annotations for opened sequences
    GTGlobals::sleep();
    GTUtilsAnnotationsTreeView::findItem(os, "enzymes", false);
    GTUtilsAnnotationsTreeView::findItem(os, "orfs", false);
    GTGlobals::sleep();

    }

GUI_TEST_CLASS_DEFINITION(test_0006_2) {
    //2. Open any PBD file from samples
    GTGlobals::sleep();
    GTGlobals::sleep();
    GTFileDialog::openFile(os, dataDir + "samples/PDB", "1CF7.PDB");

    //1. Enable Auto-annotations
    QWidget* toggleAutoAnnotationsButton = GTWidget::findWidget(os, "toggleAutoAnnotationsButton");
//  !!! dirty fastfix of test, very temporary
    QToolBar* tb = qobject_cast<QToolBar*>(toggleAutoAnnotationsButton->parent());
    QToolButton* extensionButton = tb->findChild<QToolButton*>("qt_toolbar_ext_button");
//

    GTUtilsDialog::waitForDialog(os, new PopupChooser(os, QStringList() << "Restriction Sites"));
    GTWidget::click(os, extensionButton);
    GTGlobals::sleep(1000);
    GTWidget::click(os, toggleAutoAnnotationsButton);
    GTGlobals::sleep();

    GTUtilsDialog::waitForDialog(os, new PopupChooser(os, QStringList() << "ORFs"));
    GTWidget::click(os, extensionButton);
    GTGlobals::sleep(1000);
    GTWidget::click(os, toggleAutoAnnotationsButton);
    GTGlobals::sleep();

    //Expected state: there is no auto-annotations for opened sequences
    GTGlobals::sleep();
    GTUtilsAnnotationsTreeView::findItem(os, "enzymes", false);
    GTUtilsAnnotationsTreeView::findItem(os, "orfs", false);
    GTGlobals::sleep();

    }
} // namespace GUITest_common_scenarios_sequence_view

} // namespace U2
