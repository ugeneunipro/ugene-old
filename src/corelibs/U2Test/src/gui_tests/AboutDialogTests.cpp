#include "AboutDialogTests.h"

#include <QtGui>
#include <U2Core/ProjectModel.h>
#include <U2Core/Task.h>
#include <U2Gui/MainWindow.h>

namespace U2 {
///////////////////////////////////////////////////////////////////////////////
//util methods for testing AboutDialog
void AboutDialogTests::openDialogByMenu() {
    expandTopLevelMenu(MWMENU_HELP,"mw_menu_bar");
    waitForMenuWithAction("&About");
    clickMenu(ACTION__ABOUT, MWMENU_HELP);
    waitForWidget("AboutDialog", true);
}
void AboutDialogTests::openDialogByShortcut() {
    keyClick("mw_menu_bar", Qt::Key_F1);
    waitForWidget("AboutDialog", true);
}
QPoint AboutDialogTests::getButtonPos(const QString& buttonObjName){
    QDialog* dialog=getActiveDialog();
    QPushButton* button=dialog->findChild<QPushButton*>(buttonObjName);
    return button->pos()+ button->rect().center();
}

///////////////////////////////////////////////////////////////////////////////
//Tests of AboutDialog
//Test 1: Open dialog with shortcut. Check Title and buttons.
void Test1AboutDialog::execute(){
    openDialogByShortcut();
}
void Test1AboutDialog::checkResult(){
    if(!isWidgetExists("AboutDialog")){
        throw TestException(tr("Not found AboutDialog"));
    }
    QWidget* dialog = findWidgetByName("AboutDialog");
    if(dialog->windowTitle() != "About UGENE"){
        throw TestException(tr("Wrong AboutDialog title"));
    }
    if(dialog->findChild<QPushButton*>("close_button") == NULL){
        throw TestException(tr("Not found close button"));
    }
    if(dialog->findChild<QPushButton*>("web_page_button") == NULL){
        throw TestException(tr("Not found web page button"));
    }
}
//Test 2: Open dialog by menu.
void Test2AboutDialog::execute(){
    openDialogByMenu();
}
void Test2AboutDialog::checkResult(){
    if(!isWidgetExists("AboutDialog")){
        throw TestException(tr("Not found AboutDialog"));
    }
}
//Test 3: Close with butoon "Close"
void Test3AboutDialog::execute(){
    openDialogByShortcut();
    if(!isWidgetExists("AboutDialog")){
        throw TestException(tr("Not found AboutDialog"));
    }
    QPoint pos=getButtonPos("close_button");
    moveTo("AboutDialog",pos);
    mouseClick("close_button",Qt::LeftButton);
    waitForWidget("AboutDialog", false);
}
void Test3AboutDialog::checkResult(){
    if(isWidgetExists("AboutDialog")){
        throw TestException(tr("AboutDialog not closed. Close button not work"));
    }
}
//Test 4: Check that tetris game started/stoped by mouse click. Close with shortcut "Escape"
void Test4AboutDialog::execute(){
    openDialogByShortcut();
    if(!isWidgetExists("AboutDialog")){
        throw TestException(tr("Not found AboutDialog"));
    }
    QWidget* dialog=findWidgetByName("AboutDialog");
    moveTo("AboutDialog");
    waitForWidget("about_widget", true);
    mouseClick("about_widget",Qt::LeftButton);
    waitForWidget("tetris_widget", true);
    if(dialog->windowTitle() != "Have fun ;-)"){
        throw TestException(tr("Wrong AboutDialog title"));
    }
    moveTo("tetris_widget");
    mouseClick("tetris_widget",Qt::LeftButton);//"tetris_widget"
    waitForWidget("tetris_widget", false);
    if(dialog->windowTitle() != "About UGENE"){
        throw TestException(tr("Wrong AboutDialog title"));
    }
    keyClick("AboutDialog",Qt::Key_Escape);
    waitForWidget("AboutDialog", false);
}
void Test4AboutDialog::checkResult(){
    if(isWidgetExists("AboutDialog")){
        throw TestException(tr("About Dialog not closed. 'Escape' shortcut not work"));
    }
}
//Test 5: Check that tetris game worked. Close with shortcut "Escape"
void Test5AboutDialog::execute(){
    openDialogByShortcut();
    if(!isWidgetExists("AboutDialog")){
        throw TestException(tr("Not found AboutDialog"));
    }
    QWidget* dialog = findWidgetByName("AboutDialog");
    moveTo("AboutDialog");
    keyClick("AboutDialog",Qt::Key_T);
    waitForWidget("tetris_widget", true);
    if(dialog->windowTitle() != "Have fun ;-)"){
        throw TestException(tr("Wrong AboutDialog title"));
    }
    //TODO: check that tetris block moved
    keyClick("tetris_widget",Qt::Key_Space);
    //sleep(100);
    for(int i=0;i<10;i++){
        keyClick("tetris_widget",Qt::Key_Up);
        //sleep(50);
    }
    for(int i=0;i<5;i++){
        keyClick("tetris_widget",Qt::Key_Left);
        //sleep(50);
    }
    for(int i=0;i<10;i++){
        keyClick("tetris_widget",Qt::Key_Down);
        //sleep(50);
    }
    for(int i=0;i<5;i++){
        keyClick("tetris_widget",Qt::Key_Right);
        //sleep(50);
    }
    keyClick("AboutDialog",Qt::Key_T);
    coreLog.info("wait tetris begin");
    waitForWidget("tetris_widget", false);
    coreLog.info("wait tetris");
    if(dialog->windowTitle() != "About UGENE"){
        throw TestException(tr("Wrong AboutDialog title"));
    }
    keyClick("AboutDialog",Qt::Key_Escape);
    coreLog.info("key click");
    waitForWidget("AboutDialog", false);
    coreLog.info("wait about dialog");
}
void Test5AboutDialog::checkResult(){
    if(isWidgetExists("AboutDialog")){
        throw TestException(tr("About Dialog not closed. 'Escape'' shortcut not work"));
    }
}

} //namespace
