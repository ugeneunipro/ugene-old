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

#include "MAFFTSupport.h"
#include "MAFFTSupportRunDialog.h"
#include "MAFFTSupportTask.h"
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
#include <U2Gui/DialogUtils.h>

namespace U2 {


MAFFTSupport::MAFFTSupport(const QString& name, const QString& path) : ExternalTool(name, path)
{
    if (AppContext::getMainWindow()) {
        viewCtx = new MAFFTSupportContext(this);
        icon = QIcon(":external_tool_support/images/cmdline.png");
        grayIcon = QIcon(":external_tool_support/images/cmdline_gray.png");
        warnIcon = QIcon(":external_tool_support/images/cmdline_warn.png");
    }
#ifdef Q_OS_WIN
    executableFileName="mafft.bat";
#else
    #ifdef Q_OS_LINUX
    executableFileName="mafft";
    #endif
#endif
    validationArguments<<"-help";
    validMessage="MAFFT";
    description=tr("<i>MAFFT</i> is a multiple sequence alignment program for unix-like operating systems. ");
    versionRegExp=QRegExp("MAFFT v(\\d+\\.\\d+\\w)");
    toolKitName="MAFFT";
}

void MAFFTSupport::sl_runWithExtFileSpecify(){
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
    ExternalToolSupportSettings::checkTemporaryDir();
    if (AppContext::getAppSettings()->getUserAppsSettings()->getTemporaryDirPath().isEmpty()){
        return;
    }
    //Call select input file and setup settings dialog
    MAFFTSupportTaskSettings settings;
    MAFFTWithExtFileSpecifySupportRunDialog mAFFTRunDialog(settings, AppContext::getMainWindow()->getQMainWindow());
    if(mAFFTRunDialog.exec() != QDialog::Accepted){
        return;
    }
    assert(!settings.inputFilePath.isEmpty());
    //
    MAFFTWithExtFileSpecifySupportTask* mAFFTSupportTask=new MAFFTWithExtFileSpecifySupportTask(settings);
    AppContext::getTaskScheduler()->registerTopLevelTask(mAFFTSupportTask);
}

////////////////////////////////////////
//ClustalWSupportAction ???    
MSAEditor* MAFFTSupportAction::getMSAEditor() const {
        MSAEditor* e = qobject_cast<MSAEditor*>(getObjectView());
        assert(e!=NULL);
        return e;
}

void MAFFTSupportAction::sl_lockedStateChanged() {
        StateLockableItem* item = qobject_cast<StateLockableItem*>(sender());
        assert(item!=NULL);
        setEnabled(!item->isStateLocked());
}
////////////////////////////////////////
//ExternalToolSupportMSAContext
MAFFTSupportContext::MAFFTSupportContext(QObject* p) : GObjectViewWindowContext(p, MSAEditorFactory::ID) {

}

void MAFFTSupportContext::initViewContext(GObjectView* view) {
    MSAEditor* msaed = qobject_cast<MSAEditor*>(view);
    assert(msaed!=NULL);
    if (msaed->getMSAObject() == NULL) {
            return;
    }
    bool objLocked = msaed->getMSAObject()->isStateLocked();

    MAFFTSupportAction* alignAction = new MAFFTSupportAction(this, view, tr("Align with MAFFT..."), 2000);

    addViewAction(alignAction);
    alignAction->setEnabled(!objLocked);

    connect(msaed->getMSAObject(), SIGNAL(si_lockedStateChanged()), alignAction, SLOT(sl_lockedStateChanged()));
    connect(alignAction, SIGNAL(triggered()), SLOT(sl_align_with_MAFFT()));
}
void MAFFTSupportContext::buildMenu(GObjectView* view, QMenu* m) {
        QList<GObjectViewAction *> actions = getViewActions(view);
        QMenu* alignMenu = GUIUtils::findSubMenu(m, MSAE_MENU_ALIGN);
        assert(alignMenu!=NULL);
        foreach(GObjectViewAction* a, actions) {
                a->addToMenuWithOrder(alignMenu);
        }
}
void MAFFTSupportContext::sl_align_with_MAFFT() {
    //Check that MAFFT and tempory directory path defined
    if (AppContext::getExternalToolRegistry()->getByName(MAFFT_TOOL_NAME)->getPath().isEmpty()){
        QMessageBox msgBox;
        msgBox.setWindowTitle(MAFFT_TOOL_NAME);
        msgBox.setText(tr("Path for %1 tool not selected.").arg(MAFFT_TOOL_NAME));
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
    if (AppContext::getExternalToolRegistry()->getByName(MAFFT_TOOL_NAME)->getPath().isEmpty()){
        return;
    }
    ExternalToolSupportSettings::checkTemporaryDir();
    if (AppContext::getAppSettings()->getUserAppsSettings()->getTemporaryDirPath().isEmpty()){
        return;
    }
    //Call run MAFFT align dialog
    MAFFTSupportAction* action = qobject_cast<MAFFTSupportAction*>(sender());
    assert(action!=NULL);
    MSAEditor* ed = action->getMSAEditor();
    MAlignmentObject* obj = ed->getMSAObject();
    if (obj == NULL)
            return;
    assert(!obj->isStateLocked());

    MAFFTSupportTaskSettings settings;
    MAFFTSupportRunDialog mAFFTRunDialog(settings, AppContext::getMainWindow()->getQMainWindow());
    if(mAFFTRunDialog.exec() != QDialog::Accepted){
        return;
    }

    MAFFTSupportTask* mAFFTSupportTask=new MAFFTSupportTask(obj, settings);
    AppContext::getTaskScheduler()->registerTopLevelTask(mAFFTSupportTask);

}

}//namespace
