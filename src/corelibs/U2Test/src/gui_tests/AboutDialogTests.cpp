#include "AboutDialogTests.h"

#include <QtGui>
#include <U2Core/ProjectModel.h>
#include <U2Core/Task.h>
#include <U2Gui/MainWindow.h>

namespace U2 {
///////////////////////////////////////////////////////////////////////////////
//util methods for testing AboutDialog
void AboutDialogTests::openDialogByMenu() {
    expandTopLevelMenu("&Help","mw_menu_bar");
    sleep(1000);
    clickMenu("&About", MWMENU_HELP);
}
void AboutDialogTests::openDialogByShortcut() {
    keyClick("mw_menu_bar", Qt::Key_F1);
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
    sleep(1000);
}
void Test1AboutDialog::checkResult(){
    if(!isWidgetExists("AboutDialog")){
        throw TestException(tr("Not found AboutDialog"));
    }
    QDialog* dialog=getActiveDialog();
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
    sleep(1000);
}
void Test2AboutDialog::checkResult(){
    if(!isWidgetExists("AboutDialog")){
        throw TestException(tr("Not found AboutDialog"));
    }
    sleep(500);
}
//Test 3: Close with butoon "Close"
void Test3AboutDialog::execute(){
    openDialogByShortcut();
    sleep(1000);
    if(!isWidgetExists("AboutDialog")){
        throw TestException(tr("Not found AboutDialog"));
    }
    sleep(1000);
    QPoint pos=getButtonPos("close_button");
    moveTo("AboutDialog",pos);
    sleep(1000);
    mouseClick("close_button",Qt::LeftButton);
    sleep(2000);
}
void Test3AboutDialog::checkResult(){
    if(isWidgetExists("AboutDialog")){
        throw TestException(tr("AboutDialog not closed. Close button not work"));
    }
}
//Test 4: Check that tetris game started/stoped by mouse click. Close with shortcut "Escape"
void Test4AboutDialog::execute(){
    openDialogByShortcut();
    sleep(1000);
    if(!isWidgetExists("AboutDialog")){
        throw TestException(tr("Not found AboutDialog"));
    }
    sleep(1000);
    QDialog* dialog=getActiveDialog();
    moveTo("AboutDialog");
    sleep(1000);
    mouseClick("about_widget",Qt::LeftButton);
    sleep(1000);
    if(dialog->windowTitle() != "Have fun ;-)"){
        throw TestException(tr("Wrong AboutDialog title"));
    }
    moveTo("tetris_widget");
    sleep(1000);
    mouseClick("tetris_widget",Qt::LeftButton);//"tetris_widget"
    sleep(1000);
    if(dialog->windowTitle() != "About UGENE"){
        throw TestException(tr("Wrong AboutDialog title"));
    }
    keyClick("AboutDialog",Qt::Key_Escape);
    sleep(1000);
}
void Test4AboutDialog::checkResult(){
    if(isWidgetExists("AboutDialog")){
        throw TestException(tr("About Dialog not closed. 'Escape' shortcut not work"));
    }
    sleep(500);
}
//Test 5: Check that tetris game worked. Close with shortcut "Escape"
void Test5AboutDialog::execute(){
    openDialogByShortcut();
    sleep(1000);
    if(!isWidgetExists("AboutDialog")){
        throw TestException(tr("Not found AboutDialog"));
    }
    sleep(1000);
    QDialog* dialog=getActiveDialog();
    moveTo("AboutDialog");
    sleep(1000);
    keyClick("AboutDialog",Qt::Key_T);
    sleep(1000);
    if(dialog->windowTitle() != "Have fun ;-)"){
        throw TestException(tr("Wrong AboutDialog title"));
    }
    //TODO: check that tetris block moved
    keyClick("tetris_widget",Qt::Key_Space);
    sleep(100);
    for(int i=0;i<10;i++){
        keyClick("tetris_widget",Qt::Key_Up);
        sleep(100);
    }
    for(int i=0;i<5;i++){
        keyClick("tetris_widget",Qt::Key_Left);
        sleep(100);
    }
    for(int i=0;i<10;i++){
        keyClick("tetris_widget",Qt::Key_Down);
        sleep(100);
    }
    for(int i=0;i<5;i++){
        keyClick("tetris_widget",Qt::Key_Right);
        sleep(100);
    }
    sleep(1000);
    keyClick("AboutDialog",Qt::Key_T);
    sleep(1000);
    if(dialog->windowTitle() != "About UGENE"){
        throw TestException(tr("Wrong AboutDialog title"));
    }
    keyClick("AboutDialog",Qt::Key_Escape);
    sleep(1000);
}
void Test5AboutDialog::checkResult(){
    if(isWidgetExists("AboutDialog")){
        throw TestException(tr("About Dialog not closed. 'Escape'' shortcut not work"));
    }
}

} //namespace
