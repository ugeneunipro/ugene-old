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
#include <U2Core/QObjectScopedPointer.h>

#include <U2View/MSAEditor.h>
#include <U2View/MSAEditorFactory.h>

#include "ExternalToolSupportSettings.h"
#include "ExternalToolSupportSettingsController.h"
#include "MAFFTSupport.h"
#include "MAFFTSupportRunDialog.h"
#include "MAFFTSupportTask.h"
#include "MafftAddToAlignmentTask.h"
#include "utils/AlignMsaAction.h"

namespace U2 {

MAFFTSupport::MAFFTSupport(const QString& name, const QString& path) : ExternalTool(name, path)
{
    if (AppContext::getMainWindow()) {
        viewCtx = new MAFFTSupportContext(this);
        icon = QIcon(":external_tool_support/images/cmdline.png");
        grayIcon = QIcon(":external_tool_support/images/cmdline_gray.png");
        warnIcon = QIcon(":external_tool_support/images/cmdline_warn.png");
    }

    executableFileName="mafft.bat";
    validationArguments<<"-help";
    validMessage="MAFFT";
    description=tr("<i>MAFFT</i> is a multiple sequence alignment program for unix-like operating systems. ");
    versionRegExp=QRegExp("MAFFT v(\\d+\\.\\d+\\w)");
    toolKitName="MAFFT";

    AppContext::getAlignmentAlgorithmsRegistry()->registerAlgorithm(new MafftAddToAligmnentAlgorithm());
}

void MAFFTSupport::sl_runWithExtFileSpecify(){
    //Check that Clustal and tempory directory path defined
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
    MAFFTSupportTaskSettings settings;
    QObjectScopedPointer<MAFFTWithExtFileSpecifySupportRunDialog> mAFFTRunDialog = new MAFFTWithExtFileSpecifySupportRunDialog(settings, AppContext::getMainWindow()->getQMainWindow());
    mAFFTRunDialog->exec();
    CHECK(!mAFFTRunDialog.isNull(), );

    if(mAFFTRunDialog->result() != QDialog::Accepted){
        return;
    }
    assert(!settings.inputFilePath.isEmpty());

    MAFFTWithExtFileSpecifySupportTask* mAFFTSupportTask=new MAFFTWithExtFileSpecifySupportTask(settings);
    AppContext::getTaskScheduler()->registerTopLevelTask(mAFFTSupportTask);
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
    bool isMsaEmpty = msaed->isAlignmentEmpty();

    AlignMsaAction* alignAction = new AlignMsaAction(this, ET_MAFFT, view, tr("Align with MAFFT..."), 2000);
    alignAction->setObjectName("Align with MAFFT");

    addViewAction(alignAction);
    alignAction->setEnabled(!objLocked && !isMsaEmpty);

    connect(msaed->getMSAObject(), SIGNAL(si_lockedStateChanged()), alignAction, SLOT(sl_updateState()));
    connect(msaed->getMSAObject(), SIGNAL(si_alignmentBecomesEmpty(bool)), alignAction, SLOT(sl_updateState()));
    connect(alignAction, SIGNAL(triggered()), SLOT(sl_align_with_MAFFT()));
}

void MAFFTSupportContext::buildMenu(GObjectView* view, QMenu* m) {
        QList<GObjectViewAction *> actions = getViewActions(view);
        QMenu* alignMenu = GUIUtils::findSubMenu(m, MSAE_MENU_ALIGN);
        SAFE_POINT(alignMenu != NULL, "alignMenu", );
        foreach(GObjectViewAction* a, actions) {
                a->addToMenuWithOrder(alignMenu);
        }
}

void MAFFTSupportContext::sl_align_with_MAFFT() {
    //Check that MAFFT and tempory directory path defined
    if (AppContext::getExternalToolRegistry()->getByName(ET_MAFFT)->getPath().isEmpty()){
        QObjectScopedPointer<QMessageBox> msgBox = new QMessageBox;
        msgBox->setWindowTitle(ET_MAFFT);
        msgBox->setText(tr("Path for %1 tool not selected.").arg(ET_MAFFT));
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
    if (AppContext::getExternalToolRegistry()->getByName(ET_MAFFT)->getPath().isEmpty()){
        return;
    }
    U2OpStatus2Log os(LogLevel_DETAILS);
    ExternalToolSupportSettings::checkTemporaryDir(os);
    CHECK_OP(os, );

    //Call run MAFFT align dialog
    AlignMsaAction *action = qobject_cast<AlignMsaAction *>(sender());
    assert(action!=NULL);
    MSAEditor* ed = action->getMsaEditor();
    MAlignmentObject* alignmentObject = ed->getMSAObject();
    SAFE_POINT(NULL != alignmentObject, "Alignment object is NULL during aligning with MAFFT!",);
    SAFE_POINT(!alignmentObject->isStateLocked(), "Alignment object is locked during aligning with MAFFT!",);

    MAFFTSupportTaskSettings settings;
    QObjectScopedPointer<MAFFTSupportRunDialog> mAFFTRunDialog = new MAFFTSupportRunDialog(settings, AppContext::getMainWindow()->getQMainWindow());
    mAFFTRunDialog->exec();
    CHECK(!mAFFTRunDialog.isNull(), );

    if(mAFFTRunDialog->result() != QDialog::Accepted){
        return;
    }

    MAFFTSupportTask* mAFFTSupportTask = new MAFFTSupportTask(alignmentObject->getMAlignment(), GObjectReference(alignmentObject), settings);
    connect(alignmentObject, SIGNAL(destroyed()), mAFFTSupportTask, SLOT(cancel()));
    AppContext::getTaskScheduler()->registerTopLevelTask(mAFFTSupportTask);

    // Turn off rows collapsing
    ed->resetCollapsibleModel();
}

}//namespace
