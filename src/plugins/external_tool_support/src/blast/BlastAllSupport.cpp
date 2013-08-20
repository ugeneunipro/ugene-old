/**
 * UGENE - Integrated Bioinformatics Tools.
 * Copyright (C) 2008-2013 UniPro <ugene@unipro.ru>
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
#include <U2Core/U2OpStatusUtils.h>
#include <U2Core/U2SafePoints.h>

#include <U2Gui/MainWindow.h>

#include <U2Gui/GUIUtils.h>
#include <U2Gui/DialogUtils.h>
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
        viewCtx = NULL; // new BlastAllSupportContext(this);
        icon = QIcon(":external_tool_support/images/ncbi.png");
        grayIcon = QIcon(":external_tool_support/images/ncbi_gray.png");
        warnIcon = QIcon(":external_tool_support/images/ncbi_warn.png");
    }
    if(ET_BLASTALL == name) {
#ifdef Q_OS_WIN
        executableFileName="blastall.exe";
#else
    #if defined(Q_OS_LINUX) || defined(Q_OS_MAC)
        executableFileName="blastall";
    #endif
#endif
        validationArguments<<"--help";
        validMessage="blastall";
        description=tr("The <i>blastall</i> is the old program developed and distributed " \
                       "by the NCBI for running BLAST searches. The NCBI recommends " \
                       "that people start using the programs of the BLAST+ package instead.");
        versionRegExp=QRegExp("blastall (\\d+\\.\\d+\\.\\d+)");
        toolKitName="BLAST";
    } else if(ET_CUDA_BLASTP == name) {
#ifdef Q_OS_WIN
        executableFileName="CUDA-BLASTP.exe";
#else
    #if defined(Q_OS_LINUX) || defined(Q_OS_MAC)
        executableFileName="CUDA-BLASTP";
    #endif
#endif
        validationArguments<<"--help";
        validMessage="Specify any particular device to be used";
        description=tr("The <i>blastall</i> is the old program developed and distributed " \
                       "by the NCBI for running BLAST searches. The NCBI recommends " \
                       "that people start using the programs of the BLAST+ package instead.");
        toolKitName="CUDA-BLAST";
    }
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
    U2OpStatus2Log os(LogLevel_DETAILS);
    ExternalToolSupportSettings::checkTemporaryDir(os);
    CHECK_OP(os, );

    //Call select input file and setup settings dialog
    BlastAllWithExtFileSpecifySupportRunDialog blastallRunDialog(lastDBPath, lastDBName, AppContext::getMainWindow()->getQMainWindow());
    if(blastallRunDialog.exec() != QDialog::Accepted){
        return;
    }
    QList<BlastTaskSettings> settingsList = blastallRunDialog.getSettingsList();
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

    ExternalToolSupportAction* queryAction = new ExternalToolSupportAction(this, view, tr("Query with BLAST..."), 2000, QStringList(ET_BLASTALL));

    addViewAction(queryAction);
    connect(queryAction, SIGNAL(triggered()), SLOT(sl_showDialog()));
}

void BlastAllSupportContext::buildMenu(GObjectView* view, QMenu* m) {
        QList<GObjectViewAction *> actions = getViewActions(view);
        QMenu* analyseMenu = GUIUtils::findSubMenu(m, ADV_MENU_ANALYSE);
        SAFE_POINT(analyseMenu != NULL, "analyzeMenu", );
        foreach(GObjectViewAction* a, actions) {
                a->addToMenuWithOrder(analyseMenu);
        }
}

void BlastAllSupportContext::sl_showDialog() {
    //Checking the BlastAll path and temporary directory path are defined
    if (AppContext::getExternalToolRegistry()->getByName(ET_BLASTALL)->getPath().isEmpty()){
        QMessageBox msgBox;
        msgBox.setWindowTitle("BLAST "+QString(ET_BLASTALL));
        msgBox.setText(tr("Path for BLAST %1 tool not selected.").arg(ET_BLASTALL));
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
           default:
               assert(NULL);
               break;
         }
    }
    if (AppContext::getExternalToolRegistry()->getByName(ET_BLASTALL)->getPath().isEmpty()){
        return;
    }
    U2OpStatus2Log os;
    ExternalToolSupportSettings::checkTemporaryDir(os);
    CHECK_OP(os, );
    
    QAction* a = (QAction*)sender();
    GObjectViewAction* viewAction = qobject_cast<GObjectViewAction*>(a);
    AnnotatedDNAView* av = qobject_cast<AnnotatedDNAView*>(viewAction->getObjectView());
    assert(av);

    ADVSequenceObjectContext* seqCtx = av->getSequenceInFocus();
    BlastAllSupportRunDialog dlg(seqCtx->getSequenceObject(), lastDBPath, lastDBName, av->getWidget());
    //Call run blastall dialog
    if(dlg.exec() == QDialog::Accepted) {

        BlastTaskSettings settings = dlg.getSettings();
        //prepare query
        DNASequenceSelection* s = seqCtx->getSequenceSelection();
        QVector<U2Region> regions;
        if(s->isEmpty()) {
            regions.append(U2Region(0, seqCtx->getSequenceLength()));
        } else {
            regions =  s->getSelectedRegions();
        }
        foreach(const U2Region& r, regions) {
            settings.querySequence = seqCtx->getSequenceData(r);
//            DNATranslation * aminoT = (dlg.translateToAmino ? seqCtx->getAminoTT() : 0);
//            DNATranslation * complT = (dlg.translateToAmino ? seqCtx->getComplementTT() : 0);
            settings.offsInGlobalSeq=r.startPos;
            Task * t = new BlastAllSupportTask(settings);
            AppContext::getTaskScheduler()->registerTopLevelTask( t );
        }
    }
}

}//namespace
