/**
 * UGENE - Integrated Bioinformatics Tools.
 * Copyright (C) 2008-2014 UniPro <ugene@unipro.ru>
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

#if (QT_VERSION < 0x050000) //Qt 5
#include <QtGui/QFileDialog>
#include <QtGui/QMainWindow>
#include <QtGui/QMessageBox>
#else
#include <QtWidgets/QFileDialog>
#include <QtWidgets/QMainWindow>
#include <QtWidgets/QMessageBox>
#endif

#include <U2Core/AppContext.h>
#include <U2Core/AppSettings.h>
#include <U2Core/MAlignmentObject.h>
#include <U2Core/U2OpStatusUtils.h>
#include <U2Core/U2SafePoints.h>
#include <U2Core/UserApplicationsSettings.h>

#include <U2Gui/AppSettingsGUI.h>
#include <U2Gui/DialogUtils.h>
#include <U2Gui/GUIUtils.h>
#include <U2Gui/MainWindow.h>

#include <U2View/MSAEditor.h>
#include <U2View/MSAEditorFactory.h>

#include "ClustalWSupport.h"
#include "ClustalWSupportRunDialog.h"
#include "ClustalWSupportTask.h"
#include "ExternalToolSupportSettingsController.h"
#include "ExternalToolSupportSettings.h"
#include "utils/AlignMsaAction.h"

namespace U2 {

ClustalWSupport::ClustalWSupport(const QString& name, const QString& path) : ExternalTool(name, path)
{
    if (AppContext::getMainWindow()) {
        viewCtx = new ClustalWSupportContext(this);
        icon = QIcon(":external_tool_support/images/clustalx.png");
        grayIcon = QIcon(":external_tool_support/images/clustalx_gray.png");
        warnIcon = QIcon(":external_tool_support/images/clustalx_warn.png");
    }
#ifdef Q_OS_WIN
    executableFileName="clustalw2.exe";
#else
    #if defined(Q_OS_UNIX)
    executableFileName="clustalw2";
    #endif
#endif
    validationArguments<<"-help";
    validMessage="CLUSTAL";
    description=tr("<i>ClustalW</i> is a free sequence alignment software for DNA or proteins.");
    versionRegExp=QRegExp("CLUSTAL (\\d+\\.\\d+) Multiple Sequence Alignments");
    toolKitName="ClustalW";
}

void ClustalWSupport::sl_runWithExtFileSpecify(){
    //Check that Clustal and tempory directory path defined
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
    U2OpStatus2Log os(LogLevel_DETAILS);
    ExternalToolSupportSettings::checkTemporaryDir(os);
    CHECK_OP(os, );
    
    //Call select input file and setup settings dialog
    ClustalWSupportTaskSettings settings;
    ClustalWWithExtFileSpecifySupportRunDialog clustalWRunDialog(settings, AppContext::getMainWindow()->getQMainWindow());
    if(clustalWRunDialog.exec() != QDialog::Accepted){
        return;
    }
    assert(!settings.inputFilePath.isEmpty());

    ClustalWWithExtFileSpecifySupportTask* clustalWSupportTask=new ClustalWWithExtFileSpecifySupportTask(settings);
    AppContext::getTaskScheduler()->registerTopLevelTask(clustalWSupportTask);
}

////////////////////////////////////////
//ExternalToolSupportMSAContext
ClustalWSupportContext::ClustalWSupportContext(QObject* p) : GObjectViewWindowContext(p, MSAEditorFactory::ID) {

}

void ClustalWSupportContext::initViewContext(GObjectView* view) {
    MSAEditor* msaed = qobject_cast<MSAEditor*>(view);
    assert(msaed!=NULL);
    if (msaed->getMSAObject() == NULL) {
            return;
    }
    bool objLocked = msaed->getMSAObject()->isStateLocked();
    bool isMsaEmpty = msaed->isAlignmentEmpty();

    AlignMsaAction *alignAction = new AlignMsaAction(this, ET_CLUSTAL, view, tr("Align with ClustalW..."), 2000);
    alignAction->setObjectName("Align with ClustalW");

    addViewAction(alignAction);
    alignAction->setEnabled(!objLocked && !isMsaEmpty);

    connect(msaed->getMSAObject(), SIGNAL(si_lockedStateChanged()), alignAction, SLOT(sl_updateState()));
    connect(msaed->getMSAObject(), SIGNAL(si_alignmentBecomesEmpty(bool)), alignAction, SLOT(sl_updateState()));
    connect(alignAction, SIGNAL(triggered()), SLOT(sl_align_with_ClustalW()));
}

void ClustalWSupportContext::buildMenu(GObjectView* view, QMenu* m) {
        QList<GObjectViewAction *> actions = getViewActions(view);
        QMenu* alignMenu = GUIUtils::findSubMenu(m, MSAE_MENU_ALIGN);
        SAFE_POINT(alignMenu != NULL, "alignMenu", );
        foreach(GObjectViewAction* a, actions) {
                a->addToMenuWithOrder(alignMenu);
        }
}

void ClustalWSupportContext::sl_align_with_ClustalW() {
    //Check that Clustal and tempory directory path defined
    if (AppContext::getExternalToolRegistry()->getByName(ET_CLUSTAL)->getPath().isEmpty()){
        QMessageBox msgBox;
        msgBox.setWindowTitle(ET_CLUSTAL);
        msgBox.setText(tr("Path for %1 tool not selected.").arg(ET_CLUSTAL));
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
    if (AppContext::getExternalToolRegistry()->getByName(ET_CLUSTAL)->getPath().isEmpty()){
        return;
    }
    U2OpStatus2Log os(LogLevel_DETAILS);
    ExternalToolSupportSettings::checkTemporaryDir(os);
    CHECK_OP(os, );
    
    //Call run ClustalW align dialog
    AlignMsaAction* action = qobject_cast<AlignMsaAction *>(sender());
    assert(action!=NULL);
    MSAEditor* ed = action->getMsaEditor();
    MAlignmentObject* obj = ed->getMSAObject();
    if (obj == NULL) {
        return;
    }
    assert(!obj->isStateLocked());

    ClustalWSupportTaskSettings settings;
    ClustalWSupportRunDialog clustalWRunDialog(obj->getMAlignment(), settings, AppContext::getMainWindow()->getQMainWindow());
    if(clustalWRunDialog.exec() != QDialog::Accepted){
        return;
    }

    ClustalWSupportTask* clustalWSupportTask = new ClustalWSupportTask(obj->getMAlignment(), GObjectReference(obj), settings);
    AppContext::getTaskScheduler()->registerTopLevelTask(clustalWSupportTask);

    // Turn off rows collapsing
    ed->resetCollapsibleModel();
}

}//namespace
