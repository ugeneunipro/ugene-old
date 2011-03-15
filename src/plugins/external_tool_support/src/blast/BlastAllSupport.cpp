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

#include "BlastAllSupport.h"
#include "BlastAllSupportRunDialog.h"
#include "BlastAllSupportTask.h"
#include "utils/BlastTaskSettings.h"
#include "ExternalToolSupportSettingsController.h"
#include "ExternalToolSupportSettings.h"
#include "utils/ExternalToolSupportAction.h"

#include <U2Core/AppContext.h>
#include <U2Core/AppSettings.h>
#include <U2Core/UserApplicationsSettings.h>

#include <U2Gui/MainWindow.h>

#include <U2Gui/GUIUtils.h>
#include <U2Misc/DialogUtils.h>
#include <U2View/ADVConstants.h>
#include <U2View/ADVSequenceObjectContext.h>
#include <U2View/ADVUtils.h>
#include <U2View/AnnotatedDNAView.h>
#include <U2Core/DNASequenceSelection.h>

#include <QtGui/QMainWindow>
#include <QtGui/QMessageBox>
#include <QtGui/QFileDialog>


namespace U2 {


BlastAllSupport::BlastAllSupport(const QString& name, const QString& path) : ExternalTool(name, path)
{
    if (AppContext::getMainWindow()) {
        viewCtx = new BlastAllSupportContext(this);
        icon = QIcon(":external_tool_support/images/ncbi.png");
        grayIcon = QIcon(":external_tool_support/images/ncbi_gray.png");
        warnIcon = QIcon(":external_tool_support/images/ncbi_warn.png");
    }
#ifdef Q_OS_WIN
    executableFileName="blastall.exe";
#else
    #ifdef Q_OS_LINUX
    executableFileName="blastall";
    #endif
#endif
    validationArguments<<"--help";
    validMessage="blastall";
    description=tr("The <i>blastall</i> is the old program developed and distributed  \
                   by the NCBI for running BLAST searches. The NCBI recommends \
                   that people start using the programs of the BLAST+ package instead.");
    versionRegExp=QRegExp("blastall (\\d+\\.\\d+\\.\\d+)");
    toolKitName="BLAST";
    lastDBName="";
    lastDBPath="";
}

void BlastAllSupport::sl_runWithExtFileSpecify(){
    //Check that blastall and tempory directory path defined
    if (path.isEmpty()){
        QMessageBox msgBox;
        msgBox.setWindowTitle("BLAST "+name);
        msgBox.setText(tr("Path for BLAST %1 tool not selected.").arg(name));
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
    QList<BlastTaskSettings> settingsList;
    BlastTaskSettings settings;
    settingsList.append(settings);
    BlastAllWithExtFileSpecifySupportRunDialog blastallRunDialog(settingsList, lastDBPath, lastDBName, AppContext::getMainWindow()->getQMainWindow());
    if(blastallRunDialog.exec() != QDialog::Accepted){
        return;
    }
    BlastAllSupportMultiTask* blastallSupportMultiTask = new BlastAllSupportMultiTask(settingsList,settingsList[0].outputResFile);
    AppContext::getTaskScheduler()->registerTopLevelTask(blastallSupportMultiTask);
}
////////////////////////////////////////
//BlastAllSupportContext
BlastAllSupportContext::BlastAllSupportContext(QObject* p) : GObjectViewWindowContext(p, ANNOTATED_DNA_VIEW_FACTORY_ID) {
    lastDBName="";
    lastDBPath="";
}

void BlastAllSupportContext::initViewContext(GObjectView* view) {
    AnnotatedDNAView* av = qobject_cast<AnnotatedDNAView*>(view);
    assert(av!=NULL);
    Q_UNUSED(av);

    ExternalToolSupprotAction* queryAction = new ExternalToolSupprotAction(this, view, tr("Query with BLAST..."), 2000, QStringList(BLASTALL_TOOL_NAME));

    addViewAction(queryAction);
    connect(queryAction, SIGNAL(triggered()), SLOT(sl_showDialog()));
}

void BlastAllSupportContext::buildMenu(GObjectView* view, QMenu* m) {
        QList<GObjectViewAction *> actions = getViewActions(view);
        QMenu* analyseMenu = GUIUtils::findSubMenu(m, ADV_MENU_ANALYSE);
        assert(analyseMenu!=NULL);
        foreach(GObjectViewAction* a, actions) {
                a->addToMenuWithOrder(analyseMenu);
        }
}

void BlastAllSupportContext::sl_showDialog() {
    //Checking the BlastAll path and temporary directory path are defined
    if (AppContext::getExternalToolRegistry()->getByName(BLASTALL_TOOL_NAME)->getPath().isEmpty()){
        QMessageBox msgBox;
        msgBox.setWindowTitle("BLAST "+QString(BLASTALL_TOOL_NAME));
        msgBox.setText(tr("Path for BLAST %1 tool not selected.").arg(BLASTALL_TOOL_NAME));
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
    if (AppContext::getExternalToolRegistry()->getByName(BLASTALL_TOOL_NAME)->getPath().isEmpty()){
        return;
    }
    ExternalToolSupportSettings::checkTemporaryDir();
    if (AppContext::getAppSettings()->getUserAppsSettings()->getTemporaryDirPath().isEmpty()){
        return;
    }
    QAction* a = (QAction*)sender();
    GObjectViewAction* viewAction = qobject_cast<GObjectViewAction*>(a);
    AnnotatedDNAView* av = qobject_cast<AnnotatedDNAView*>(viewAction->getObjectView());
    assert(av);

    BlastTaskSettings settings;
    ADVSequenceObjectContext* seqCtx = av->getSequenceInFocus();
    BlastAllSupportRunDialog dlg(seqCtx->getSequenceObject(), settings, lastDBPath, lastDBName, av->getWidget());
    //Call run blastall dialog
    if(dlg.exec() == QDialog::Accepted) {

        //prepare query
        const QByteArray& sequence = seqCtx->getSequenceData();
        DNASequenceSelection* s = seqCtx->getSequenceSelection();
        QVector<U2Region> regions;
        if(s->isEmpty()) {
            int seqLen = seqCtx->getSequenceLen();
            regions.append(U2Region(0, seqLen));
        }
        else {
            regions =  s->getSelectedRegions();
        }
        foreach(const U2Region& r, regions) {
            QByteArray query( sequence.constData() + r.startPos, r.length );
            settings.querySequence = query;
//            DNATranslation * aminoT = (dlg.translateToAmino ? seqCtx->getAminoTT() : 0);
//            DNATranslation * complT = (dlg.translateToAmino ? seqCtx->getComplementTT() : 0);
            settings.offsInGlobalSeq=r.startPos;
            Task * t = new BlastAllSupportTask(settings);
            AppContext::getTaskScheduler()->registerTopLevelTask( t );
        }
    }
}

}//namespace
