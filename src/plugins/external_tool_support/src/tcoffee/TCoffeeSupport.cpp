/**
 * UGENE - Integrated Bioinformatics Tools.
 * Copyright (C) 2008-2011 UniPro <ugene@unipro.ru>
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

#include "TCoffeeSupport.h"
#include "TCoffeeSupportRunDialog.h"
#include "TCoffeeSupportTask.h"
#include "ExternalToolSupportSettingsController.h"
#include "ExternalToolSupportSettings.h"

#include <U2Core/AppContext.h>
#include <U2Core/AppSettings.h>
#include <U2Core/UserApplicationsSettings.h>
#include <U2Gui/MainWindow.h>
#include <QtGui/QMainWindow>
#include <QtGui/QMessageBox>
#include <QtGui/QFileDialog>
#include <U2View/MSAEditor.h>
#include <U2View/MSAEditorFactory.h>

#include <U2Core/MAlignmentObject.h>

#include <U2Gui/GUIUtils.h>
#include <U2Misc/DialogUtils.h>

namespace U2 {


TCoffeeSupport::TCoffeeSupport(const QString& name, const QString& path) : ExternalTool(name, path)
{
    if (AppContext::getMainWindow()) {
        viewCtx = new TCoffeeSupportContext(this);
        icon = QIcon(":external_tool_support/images/tcoffee.png");
        grayIcon = QIcon(":external_tool_support/images/tcoffee_gray.png");
        warnIcon = QIcon(":external_tool_support/images/tcoffee_warn.png");
    }
#ifdef Q_OS_WIN
    executableFileName="t_coffee.exe";//need check on windows
#else
    #ifdef Q_OS_LINUX
    executableFileName="t_coffee";
    #endif
#endif
    validationArguments<<"-h";
    validMessage="PROGRAM: T-COFFEE";
    description=tr("<i>T-Coffee</i> is a multiple sequence alignment package.");
    versionRegExp=QRegExp("PROGRAM: T-COFFEE \\(Version_(\\d+\\.\\d+_\\d+)");
    toolKitName="T-Coffee";
}

void TCoffeeSupport::sl_runWithExtFileSpecify(){
    //Check that T-Coffee and tempory directory path defined
    if (path.isEmpty()){
        QMessageBox msgBox;
        msgBox.setWindowTitle(name);
        msgBox.setText(tr("Path for %1 tool not selected.").arg(name));
        msgBox.setInformativeText(tr("Do you want to select it now?"));
        msgBox.setStandardButtons(QMessageBox::Yes | QMessageBox::No);
        msgBox.setDefaultButton(QMessageBox::Yes);
        int ret = msgBox.exec();
        switch (ret) {
           case QMessageBox::Yes:
               AppContext::getAppSettingsGUI()->showSettingsDialog(ExternalToolSupportSettingsPageId);
               break;
           case QMessageBox::No:
               return;
               break;
           default:
               assert(NULL);
               break;
         }
    }
    if (path.isEmpty()){
        return;
    }
    ExternalToolSupportSettings::checkTemporaryDir();
    if (AppContext::getAppSettings()->getUserAppsSettings()->getTemporaryDirPath().isEmpty()){
        return;
    }
    //Call select input file and setup settings dialog
    TCoffeeSupportTaskSettings settings;
    TCoffeeWithExtFileSpecifySupportRunDialog tCoffeeRunDialog(settings, AppContext::getMainWindow()->getQMainWindow());
    if(tCoffeeRunDialog.exec() != QDialog::Accepted){
        return;
    }
    assert(!settings.inputFilePath.isEmpty());
    //
    TCoffeeWithExtFileSpecifySupportTask* tCoffeeSupportTask=new TCoffeeWithExtFileSpecifySupportTask(settings);
    AppContext::getTaskScheduler()->registerTopLevelTask(tCoffeeSupportTask);
}

////////////////////////////////////////
//TCoffeeSupportAction ???
MSAEditor* TCoffeeSupportAction::getMSAEditor() const {
        MSAEditor* e = qobject_cast<MSAEditor*>(getObjectView());
        assert(e!=NULL);
        return e;
}

void TCoffeeSupportAction::sl_lockedStateChanged() {
        StateLockableItem* item = qobject_cast<StateLockableItem*>(sender());
        assert(item!=NULL);
        setEnabled(!item->isStateLocked());
}
////////////////////////////////////////
//TCoffeeSupportContext
TCoffeeSupportContext::TCoffeeSupportContext(QObject* p) : GObjectViewWindowContext(p, MSAEditorFactory::ID) {

}

void TCoffeeSupportContext::initViewContext(GObjectView* view) {
    MSAEditor* msaed = qobject_cast<MSAEditor*>(view);
    assert(msaed!=NULL);
    if (msaed->getMSAObject() == NULL) {
            return;
    }
    bool objLocked = msaed->getMSAObject()->isStateLocked();

    TCoffeeSupportAction* alignAction = new TCoffeeSupportAction(this, view, tr("Align with T-Coffee..."), 2000);

    addViewAction(alignAction);
    alignAction->setEnabled(!objLocked);

    connect(msaed->getMSAObject(), SIGNAL(si_lockedStateChanged()), alignAction, SLOT(sl_lockedStateChanged()));
    connect(alignAction, SIGNAL(triggered()), SLOT(sl_align_with_TCoffee()));
}
void TCoffeeSupportContext::buildMenu(GObjectView* view, QMenu* m) {
        QList<GObjectViewAction *> actions = getViewActions(view);
        QMenu* alignMenu = GUIUtils::findSubMenu(m, MSAE_MENU_ALIGN);
        assert(alignMenu!=NULL);
        foreach(GObjectViewAction* a, actions) {
                a->addToMenuWithOrder(alignMenu);
        }
}
void TCoffeeSupportContext::sl_align_with_TCoffee() {
    //Check that T-Coffee and tempory directory path defined
    if (AppContext::getExternalToolRegistry()->getByName(TCOFFEE_TOOL_NAME)->getPath().isEmpty()){
        QMessageBox msgBox;
        msgBox.setWindowTitle(TCOFFEE_TOOL_NAME);
        msgBox.setText(tr("Path for %1 tool not selected.").arg(TCOFFEE_TOOL_NAME));
        msgBox.setInformativeText(tr("Do you want to select it now?"));
        msgBox.setStandardButtons(QMessageBox::Yes | QMessageBox::No);
        msgBox.setDefaultButton(QMessageBox::Yes);
        int ret = msgBox.exec();
        switch (ret) {
           case QMessageBox::Yes:
               AppContext::getAppSettingsGUI()->showSettingsDialog(ExternalToolSupportSettingsPageId);
               break;
           case QMessageBox::No:
               return;
               break;
           default:
               assert(NULL);
               break;
         }
    }
    if (AppContext::getExternalToolRegistry()->getByName(TCOFFEE_TOOL_NAME)->getPath().isEmpty()){
        return;
    }
    ExternalToolSupportSettings::checkTemporaryDir();
    if (AppContext::getAppSettings()->getUserAppsSettings()->getTemporaryDirPath().isEmpty()){
        return;
    }
    //Call run T-Coffee align dialog
    TCoffeeSupportAction* action = qobject_cast<TCoffeeSupportAction*>(sender());
    assert(action!=NULL);
    MSAEditor* ed = action->getMSAEditor();
    MAlignmentObject* obj = ed->getMSAObject();
    if (obj == NULL)
            return;
    assert(!obj->isStateLocked());

    TCoffeeSupportTaskSettings settings;
    TCoffeeSupportRunDialog tCoffeeRunDialog(settings, AppContext::getMainWindow()->getQMainWindow());
    if(tCoffeeRunDialog.exec() != QDialog::Accepted){
        return;
    }

    TCoffeeSupportTask* tCoffeeSupportTask=new TCoffeeSupportTask(obj, settings);
    AppContext::getTaskScheduler()->registerTopLevelTask(tCoffeeSupportTask);

}

}//namespace
