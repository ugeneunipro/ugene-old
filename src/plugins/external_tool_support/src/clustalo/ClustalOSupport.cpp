/**
 * UGENE - Integrated Bioinformatics Tools.
 * Copyright (C) 2008-2016 UniPro <ugene@unipro.ru>
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
#include <U2Core/DNAAlphabet.h>
#include <U2Core/MAlignmentObject.h>
#include <U2Core/U2OpStatusUtils.h>
#include <U2Core/U2SafePoints.h>
#include <U2Core/UserApplicationsSettings.h>

#include <U2Gui/AppSettingsGUI.h>
#include <U2Gui/DialogUtils.h>
#include <U2Gui/GUIUtils.h>
#include <U2Gui/MainWindow.h>
#include <U2Core/QObjectScopedPointer.h>

#include <U2View/MSAEditor.h>
#include <U2View/MSAEditorFactory.h>

#include "ClustalOSupport.h"
#include "ClustalOSupportRunDialog.h"
#include "ClustalOSupportTask.h"
#include "ExternalToolSupportSettings.h"
#include "ExternalToolSupportSettingsController.h"

namespace U2 {

ClustalOSupport::ClustalOSupport(const QString& name, const QString& path) : ExternalTool(name, path)
{
    if (AppContext::getMainWindow()) {
        viewCtx = new ClustalOSupportContext(this);
        icon = QIcon(":external_tool_support/images/clustalo.png");
        grayIcon = QIcon(":external_tool_support/images/clustalo_gray.png");
        warnIcon = QIcon(":external_tool_support/images/clustalo_warn.png");
    }
#ifdef Q_OS_WIN
    executableFileName="ClustalO.exe";
#else
    #if defined(Q_OS_UNIX)
    executableFileName="clustalo";
    #endif
#endif
    validationArguments<<"--help";
    validMessage="Clustal Omega";
    description=tr("<i>Clustal Omega</i> is a free sequence alignment software for proteins.");
    versionRegExp=QRegExp("Clustal Omega - (\\d+\\.\\d+\\.\\d+).*");
    toolKitName="ClustalO";
}

void ClustalOSupport::sl_runWithExtFileSpecify(){
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
               assert(false);
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
    ClustalOSupportTaskSettings settings;
    QObjectScopedPointer<ClustalOWithExtFileSpecifySupportRunDialog> clustalORunDialog = new ClustalOWithExtFileSpecifySupportRunDialog(settings, AppContext::getMainWindow()->getQMainWindow());
    clustalORunDialog->exec();
    CHECK(!clustalORunDialog.isNull(), );

    if (clustalORunDialog->result() != QDialog::Accepted){
        return;
    }
    assert(!settings.inputFilePath.isEmpty());

    ClustalOWithExtFileSpecifySupportTask* ClustalOSupportTask=new ClustalOWithExtFileSpecifySupportTask(settings);
    AppContext::getTaskScheduler()->registerTopLevelTask(ClustalOSupportTask);
}

////////////////////////////////////////
//ExternalToolSupportMSAContext
ClustalOSupportContext::ClustalOSupportContext(QObject* p) : GObjectViewWindowContext(p, MSAEditorFactory::ID) {

}

void ClustalOSupportContext::initViewContext(GObjectView* view) {
    MSAEditor* msaed = qobject_cast<MSAEditor*>(view);
    assert(msaed!=NULL);
    if (msaed->getMSAObject() == NULL) {
        return;
    }
    bool objLocked = msaed->getMSAObject()->isStateLocked();
    bool isMsaEmpty = msaed->isAlignmentEmpty();

    AlignMsaAction* alignAction = new AlignMsaAction(this, ET_CLUSTALO, view, tr("Align with ClustalO..."), 2000);
    alignAction->setObjectName("Align with ClustalO");

    addViewAction(alignAction);
    alignAction->setEnabled(!objLocked && !isMsaEmpty);

    connect(msaed->getMSAObject(), SIGNAL(si_lockedStateChanged()), alignAction, SLOT(sl_updateState()));
    connect(msaed->getMSAObject(), SIGNAL(si_alignmentBecomesEmpty(bool)), alignAction, SLOT(sl_updateState()));
    connect(alignAction, SIGNAL(triggered()), SLOT(sl_align_with_ClustalO()));
}

void ClustalOSupportContext::buildMenu(GObjectView* view, QMenu* m) {
        QList<GObjectViewAction *> actions = getViewActions(view);
        QMenu* alignMenu = GUIUtils::findSubMenu(m, MSAE_MENU_ALIGN);
        SAFE_POINT(alignMenu != NULL, "alignMenu", );
        foreach(GObjectViewAction* a, actions) {
            a->addToMenuWithOrder(alignMenu);
        }
}

void ClustalOSupportContext::sl_align_with_ClustalO() {
    //Check that Clustal and tempory directory path defined
    if (AppContext::getExternalToolRegistry()->getByName(ET_CLUSTALO)->getPath().isEmpty()){
        QObjectScopedPointer<QMessageBox> msgBox = new QMessageBox;
        msgBox->setWindowTitle(ET_CLUSTALO);
        msgBox->setText(tr("Path for %1 tool not selected.").arg(ET_CLUSTALO));
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
               assert(false);
               break;
         }
    }
    if (AppContext::getExternalToolRegistry()->getByName(ET_CLUSTALO)->getPath().isEmpty()){
        return;
    }
    U2OpStatus2Log os(LogLevel_DETAILS);
    ExternalToolSupportSettings::checkTemporaryDir(os);
    CHECK_OP(os, );

    //Call run ClustalO align dialog
    AlignMsaAction* action = qobject_cast<AlignMsaAction *>(sender());
    assert(action!=NULL);
    MSAEditor* ed = action->getMsaEditor();
    MAlignmentObject* obj = ed->getMSAObject();
    if (obj == NULL) {
        return;
    }
    assert(!obj->isStateLocked());

    ClustalOSupportTaskSettings settings;
    QObjectScopedPointer<ClustalOSupportRunDialog> clustalORunDialog = new ClustalOSupportRunDialog(obj->getMAlignment(), settings, AppContext::getMainWindow()->getQMainWindow());
    clustalORunDialog->exec();
    CHECK(!clustalORunDialog.isNull(), );

    if(clustalORunDialog->result() != QDialog::Accepted){
        return;
    }

    ClustalOSupportTask* clustalOSupportTask = new ClustalOSupportTask(obj->getMAlignment(), GObjectReference(obj), settings);
    connect(obj, SIGNAL(destroyed()), clustalOSupportTask, SLOT(cancel()));
    AppContext::getTaskScheduler()->registerTopLevelTask(clustalOSupportTask);

    // Turn off rows collapsing
    ed->resetCollapsibleModel();
}

}//namespace
