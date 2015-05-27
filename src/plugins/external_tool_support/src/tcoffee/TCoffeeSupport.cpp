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

#include <QMainWindow>
#include <QMessageBox>

#include <U2Core/AppContext.h>
#include <U2Core/AppSettings.h>
#include <U2Core/MAlignmentObject.h>
#include <U2Core/U2OpStatusUtils.h>
#include <U2Core/U2SafePoints.h>
#include <U2Core/UserApplicationsSettings.h>

#include <U2Gui/DialogUtils.h>
#include <U2Gui/GUIUtils.h>
#include <U2Gui/MainWindow.h>
#include <U2Gui/QObjectScopedPointer.h>

#include <U2View/MSAEditor.h>
#include <U2View/MSAEditorFactory.h>

#include "ExternalToolSupportSettings.h"
#include "ExternalToolSupportSettingsController.h"
#include "TCoffeeSupport.h"
#include "TCoffeeSupportRunDialog.h"
#include "TCoffeeSupportTask.h"
#include "utils/AlignMsaAction.h"

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
    executableFileName="t_coffee.bat";
#else
    #if defined(Q_OS_UNIX)
    executableFileName="t_coffee";
    #endif
#endif
    validationArguments<<"-help";
    validMessage="PROGRAM: T-COFFEE";
    description=tr("<i>T-Coffee</i> is a multiple sequence alignment package.");
    versionRegExp=QRegExp("PROGRAM: T-COFFEE Version_(\\d+\\.\\d+)");
    toolKitName="T-Coffee";
}

void TCoffeeSupport::sl_runWithExtFileSpecify(){
    //Check that T-Coffee and tempory directory path defined
    if (path.isEmpty()){
        QObjectScopedPointer<QMessageBox> msgBox = new QMessageBox;
        msgBox->setWindowTitle(name);
        msgBox->setText(tr("Path for %1 tool not selected.").arg(name));
        msgBox->setInformativeText(tr("Do you want to select it now?"));
        msgBox->setStandardButtons(QMessageBox::Yes | QMessageBox::No);
        msgBox->setDefaultButton(QMessageBox::Yes);
        const int ret = msgBox->exec();
        CHECK(!msgBox.isNull(), );

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
    U2OpStatus2Log os(LogLevel_DETAILS);
    ExternalToolSupportSettings::checkTemporaryDir(os);
    CHECK_OP(os, );


    //Call select input file and setup settings dialog
    TCoffeeSupportTaskSettings settings;
    QObjectScopedPointer<TCoffeeWithExtFileSpecifySupportRunDialog> tCoffeeRunDialog = new TCoffeeWithExtFileSpecifySupportRunDialog(settings, AppContext::getMainWindow()->getQMainWindow());
    tCoffeeRunDialog->exec();
    CHECK(!tCoffeeRunDialog.isNull(), );

    if(tCoffeeRunDialog->result() != QDialog::Accepted){
        return;
    }
    assert(!settings.inputFilePath.isEmpty());
    //
    TCoffeeWithExtFileSpecifySupportTask* tCoffeeSupportTask=new TCoffeeWithExtFileSpecifySupportTask(settings);
    AppContext::getTaskScheduler()->registerTopLevelTask(tCoffeeSupportTask);
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
    bool isMsaEmpty = msaed->isAlignmentEmpty();

    AlignMsaAction* alignAction = new AlignMsaAction(this, ET_TCOFFEE, view, tr("Align with T-Coffee..."), 2000);
    alignAction->setObjectName("Align with T-Coffee");

    addViewAction(alignAction);
    alignAction->setEnabled(!objLocked && !isMsaEmpty);

    connect(msaed->getMSAObject(), SIGNAL(si_lockedStateChanged()), alignAction, SLOT(sl_updateState()));
    connect(msaed->getMSAObject(), SIGNAL(si_alignmentBecomesEmpty(bool)), alignAction, SLOT(sl_updateState()));
    connect(alignAction, SIGNAL(triggered()), SLOT(sl_align_with_TCoffee()));
}

void TCoffeeSupportContext::buildMenu(GObjectView* view, QMenu* m) {
    QList<GObjectViewAction *> actions = getViewActions(view);
    QMenu* alignMenu = GUIUtils::findSubMenu(m, MSAE_MENU_ALIGN);
    SAFE_POINT(alignMenu != NULL, "alignMenu", );
    foreach(GObjectViewAction* a, actions) {
        a->addToMenuWithOrder(alignMenu);
    }
}

void TCoffeeSupportContext::sl_align_with_TCoffee() {
    //Check that T-Coffee and temporary directory path defined
    if (AppContext::getExternalToolRegistry()->getByName(ET_TCOFFEE)->getPath().isEmpty()){
        QObjectScopedPointer<QMessageBox> msgBox = new QMessageBox;
        msgBox->setWindowTitle(ET_TCOFFEE);
        msgBox->setText(tr("Path for %1 tool not selected.").arg(ET_TCOFFEE));
        msgBox->setInformativeText(tr("Do you want to select it now?"));
        msgBox->setStandardButtons(QMessageBox::Yes | QMessageBox::No);
        msgBox->setDefaultButton(QMessageBox::Yes);
        const int ret = msgBox->exec();
        CHECK(!msgBox.isNull(), );

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
    if (AppContext::getExternalToolRegistry()->getByName(ET_TCOFFEE)->getPath().isEmpty()){
        return;
    }
    U2OpStatus2Log os(LogLevel_DETAILS);
    ExternalToolSupportSettings::checkTemporaryDir(os);
    CHECK_OP(os, );

    //Call run T-Coffee align dialog
    AlignMsaAction* action = qobject_cast<AlignMsaAction*>(sender());
    assert(action!=NULL);
    MSAEditor* ed = action->getMsaEditor();
    MAlignmentObject* obj = ed->getMSAObject();
    if (obj == NULL)
            return;
    assert(!obj->isStateLocked());

    TCoffeeSupportTaskSettings settings;
    QObjectScopedPointer<TCoffeeSupportRunDialog> tCoffeeRunDialog = new TCoffeeSupportRunDialog(settings, AppContext::getMainWindow()->getQMainWindow());
    tCoffeeRunDialog->exec();
    CHECK(!tCoffeeRunDialog.isNull(), );

    if(tCoffeeRunDialog->result() != QDialog::Accepted){
        return;
    }

    TCoffeeSupportTask* tCoffeeSupportTask = new TCoffeeSupportTask(obj->getMAlignment(), GObjectReference(obj), settings);
    connect(obj, SIGNAL(destroyed()), tCoffeeSupportTask, SLOT(cancel()));
    AppContext::getTaskScheduler()->registerTopLevelTask(tCoffeeSupportTask);

    // Turn off rows collapsing
    ed->resetCollapsibleModel();
}

}//namespace
