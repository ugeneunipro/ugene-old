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

#include <QWebElement>

#include "GTUtilsMdi.h"
#include "GTUtilsProject.h"
#include "GTUtilsProjectTreeView.h"
#include "GTutilsStartPage.h"

#include "api/GTFileDialog.h"
#include "api/GTMenu.h"
#include "api/GTKeyboardDriver.h"
#include "api/GTWebView.h"

#include "runnables/qt/DefaultDialogFiller.h"
#include "runnables/qt/PopupChooser.h"
#include "runnables/ugene/plugins/workflow_designer/StartupDialogFiller.h"

#include "GTTestsStartPage.h"

namespace U2 {

namespace GUITest_common_scenarios_start_page{

GUI_TEST_CLASS_DEFINITION(test_0001){
//    Start UGENE
//    Press "Open file(s)" button on start page
    GTUtilsDialog::waitForDialog(os, new GTFileDialogUtils(os, dataDir + "samples/CLUSTALW/COI.aln"));
    GTutilsStartPage::clickButton(os, GTutilsStartPage::OpenFile);
//    Expected state: File dialog is opened.
    GTGlobals::sleep(500);
}

GUI_TEST_CLASS_DEFINITION(test_0002){
//    Start UGENE
//    Press "Create workflow button" button on start page
    GTUtilsDialog::waitForDialog(os, new StartupDialogFiller(os));
    GTutilsStartPage::clickButton(os, GTutilsStartPage::CreateWorkflow);
    GTGlobals::sleep(500);
//    Expected state: WD opened.
    QString windowName = GTUtilsMdi::activeWindowTitle(os);
    CHECK_SET_ERR(windowName == "Workflow Designer - New workflow", "unexpected window title: " + windowName)
}

GUI_TEST_CLASS_DEFINITION(test_0003){
//    Start UGENE
//    Press "Create sequence" button
    GTUtilsDialog::waitForDialog(os, new DefaultDialogFiller(os, "CreateDocumentFromTextDialog",QDialogButtonBox::Cancel));
    GTutilsStartPage::clickButton(os, GTutilsStartPage::CreateSequence);
//    Expected: Create document from text dialog appeared
    GTGlobals::sleep(500);
}

GUI_TEST_CLASS_DEFINITION(test_0005){
//    Start UGENE
//    Open any file.
    GTFileDialog::openFile(os, dataDir + "samples/CLUSTALW/COI.aln");
//    Go to Start page.
    GTutilsStartPage::openStartPage(os);
//    Expected state: File added to "Resent files" list
//    Remove file from project
    GTUtilsProjectTreeView::click(os, "COI.aln");
    GTKeyboardDriver::keyClick(os, GTKeyboardDriver::key["delete"]);
//    Go to Start page
    GTutilsStartPage::openStartPage(os);
//    Click file name in "Resent files" list
    GTGlobals::sleep();
    GTutilsStartPage::clickResentDocument(os, "COI.aln");
//    Expected state: file is opened
    QString name = GTUtilsMdi::activeWindowTitle(os);
    CHECK_SET_ERR(name == "COI [m] COI", "unexpected window title " + name);
    GTUtilsProjectTreeView::checkItem(os, "COI.aln");
}

GUI_TEST_CLASS_DEFINITION(test_0006){
//    Start UGENE
//    Open any project.
    GTFileDialog::openFile(os, testDir+"_common_data/scenarios/project/", "proj1.uprj");
//    Go to Start page.
//    Expected state: project is added to "Resent files" list
//    Close project
    GTUtilsProject::closeProject(os);
    GTGlobals::sleep(500);
    GTWebView::traceAllWebElements(os, GTutilsStartPage::getStartPage(os));
    GTGlobals::sleep(500);
//    Go to Start page
    GTutilsStartPage::openStartPage(os);
//    Click project name in "Resent projects" list
    GTGlobals::sleep();
    GTutilsStartPage::clickResentProject(os, "proj1.uprj");
//    Expected state: project is opened
    GTUtilsDocument::checkDocument(os, "1CF7.PDB");

    QString expectedTitle;
#ifdef Q_OS_MAC
    expectedTitle = "proj1 UGENE";
#else
    expectedTitle = "proj1 UGENE - [Start Page]";
#endif
    GTUtilsApp::checkUGENETitle(os, expectedTitle);
}

GUI_TEST_CLASS_DEFINITION(test_0008){
//    Start UGENE
    GTGlobals::sleep();
    QString title = GTUtilsMdi::activeWindowTitle(os);
    CHECK_SET_ERR(title == "Start Page", "unexpected window title: " + title);
//    Use main menu: Help->Open start page
    GTUtilsDialog::waitForDialog(os, new PopupChooser(os, QStringList()<<"welcome_page"));
    GTMenu::showMainMenu(os, MWMENU_HELP);
    GTGlobals::sleep(500);
//    Expected state: nothing happens
    title = GTUtilsMdi::activeWindowTitle(os);
    CHECK_SET_ERR(title == "Start Page", "unexpected window title: " + title);
//    Close Start page
    GTUtilsMdi::click(os, GTGlobals::Close);
    QWidget* window = GTUtilsMdi::activeWindow(os, GTGlobals::FindOptions(false));
    CHECK_SET_ERR(window == NULL, "start page was not closed")
//    Repeat step 2
    GTUtilsDialog::waitForDialog(os, new PopupChooser(os, QStringList()<<"welcome_page"));
    GTMenu::showMainMenu(os, MWMENU_HELP);
    GTGlobals::sleep(500);
//    Expected state: Start page is opened
    title = GTUtilsMdi::activeWindowTitle(os);
    CHECK_SET_ERR(title == "Start Page", "unexpected window title: " + title);
}

}

}
