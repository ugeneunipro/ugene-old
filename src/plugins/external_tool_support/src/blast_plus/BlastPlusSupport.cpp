/**
 * UGENE - Integrated Bioinformatics Tools.
 * Copyright (C) 2008-2012 UniPro <ugene@unipro.ru>
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

#include "BlastPlusSupport.h"
#include "BlastPlusSupportRunDialog.h"
#include "BlastPlusSupportCommonTask.h"
#include "BlastNPlusSupportTask.h"
#include "BlastPPlusSupportTask.h"
#include "BlastXPlusSupportTask.h"
#include "TBlastNPlusSupportTask.h"
#include "TBlastXPlusSupportTask.h"
#include "RPSBlastSupportTask.h"
#include "utils/ExternalToolSupportAction.h"
#include "ExternalToolSupportSettingsController.h"
#include "ExternalToolSupportSettings.h"

#include <U2Core/AppContext.h>
#include <U2Core/AppSettings.h>
#include <U2Core/UserApplicationsSettings.h>
#include <U2Core/U2SafePoints.h>
#include <U2Core/U2OpStatusUtils.h>

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


BlastPlusSupport::BlastPlusSupport(const QString& name, const QString& path) : ExternalTool(name, path)
{
    if (AppContext::getMainWindow()) {
        icon = QIcon(":external_tool_support/images/ncbi.png");
        grayIcon = QIcon(":external_tool_support/images/ncbi_gray.png");
        warnIcon = QIcon(":external_tool_support/images/ncbi_warn.png");
    }
    validationArguments<<"-h";

    if(name == BLASTN_TOOL_NAME){
#ifdef Q_OS_WIN
    executableFileName="blastn.exe";
#else
    #if defined(Q_OS_LINUX) || defined(Q_OS_MAC)
    executableFileName="blastn";
    #endif
#endif
    validMessage="Nucleotide-Nucleotide BLAST";
    description="The <i>blastn</i> tool searches a nucleotide database \
                using a nucleotide query.";
    versionRegExp=QRegExp("Nucleotide-Nucleotide BLAST (\\d+\\.\\d+\\.\\d+\\+?)");
    }else if(name == BLASTP_TOOL_NAME){
#ifdef Q_OS_WIN
    executableFileName="blastp.exe";
#else
    #if defined(Q_OS_LINUX) || defined(Q_OS_MAC)
    executableFileName="blastp";
    #endif
#endif
    validMessage="Protein-Protein BLAST";
    description="The <i>blastp</i> tool searches a protein database \
                using a protein query.";
    versionRegExp=QRegExp("Protein-Protein BLAST (\\d+\\.\\d+\\.\\d+\\+?)");
    }else if(name == GPU_BLASTP_TOOL_NAME){
#ifdef Q_OS_WIN
    executableFileName="blastp.exe";
#else
    #ifdef Q_OS_LINUX
    executableFileName="blastp";
    #endif
#endif
    validMessage="[-gpu boolean]";
    description="The <i>blastp</i> tool searches a protein database \
                using a protein query.";
    versionRegExp=QRegExp("Protein-Protein BLAST (\\d+\\.\\d+\\.\\d+\\+?)");
    }else if(name == BLASTX_TOOL_NAME){
#ifdef Q_OS_WIN
    executableFileName="blastx.exe";
#else
    #if defined(Q_OS_LINUX) || defined(Q_OS_MAC)
    executableFileName="blastx";
    #endif
#endif
    validMessage="Translated Query-Protein Subject";
    description="The <i>blastx</i> tool searches a protein database \
                using a translated nucleotide query.";
    versionRegExp=QRegExp("Translated Query-Protein Subject BLAST (\\d+\\.\\d+\\.\\d+\\+?)");
    }else if(name == TBLASTN_TOOL_NAME){
#ifdef Q_OS_WIN
    executableFileName="tblastn.exe";
#else
    #if defined(Q_OS_LINUX) || defined(Q_OS_MAC)
    executableFileName="tblastn";
    #endif
#endif
    validMessage="Protein Query-Translated Subject";
    description="The <i>tblastn</i> compares a protein query against \
                a translated nucleotide database";
    versionRegExp=QRegExp("Protein Query-Translated Subject BLAST (\\d+\\.\\d+\\.\\d+\\+?)");
    }else if(name == TBLASTX_TOOL_NAME){
#ifdef Q_OS_WIN
    executableFileName="tblastx.exe";
#else
    #if defined(Q_OS_LINUX) || defined(Q_OS_MAC)
    executableFileName="tblastx";
    #endif
#endif
    validMessage="Translated Query-Translated Subject";
    description="The <i>tblastx</i> translates the query nucleotide \
                sequence in all six possible frames and compares it \
                against the six-frame translations of a nucleotide \
                sequence database.";
    versionRegExp=QRegExp("Translated Query-Translated Subject BLAST (\\d+\\.\\d+\\.\\d+\\+?)");
    }else if(name == RPSBLAST_TOOL_NAME) {
#ifdef Q_OS_WIN
    executableFileName="rpsblast.exe";
#else
    #if defined(Q_OS_LINUX) || defined(Q_OS_MAC)
    executableFileName="rpsblast";
    #endif
#endif
    validMessage="Reverse Position Specific BLAST";
    description="";
    versionRegExp=QRegExp("Reverse Position Specific BLAST (\\d+\\.\\d+\\.\\d+\\+?)");
    }
    if(name == GPU_BLASTP_TOOL_NAME) {
        toolKitName="GPU-BLAST+";
    } else {
        toolKitName="BLAST+";
    }
    lastDBName="";
    lastDBPath="";
}

void BlastPlusSupport::sl_runWithExtFileSpecify(){
    //Check that blastal and tempory directory path defined
    QStringList toolList;
    toolList << BLASTN_TOOL_NAME << BLASTP_TOOL_NAME << BLASTX_TOOL_NAME << TBLASTN_TOOL_NAME << TBLASTX_TOOL_NAME << RPSBLAST_TOOL_NAME;
    bool isOneOfToolConfigured=false;
    foreach(QString toolName, toolList){
        if(!AppContext::getExternalToolRegistry()->getByName(toolName)->getPath().isEmpty()){
            isOneOfToolConfigured=true;
        }
    }
    if (!isOneOfToolConfigured){
        QMessageBox msgBox;
        msgBox.setWindowTitle("BLAST+ Search");
        msgBox.setText(tr("Path for BLAST+ tools not selected."));
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
        bool isOneOfToolConfigured=false;
        foreach(QString toolName, toolList){
            if(!AppContext::getExternalToolRegistry()->getByName(toolName)->getPath().isEmpty()){
                isOneOfToolConfigured=true;
            }
        }
        if (!isOneOfToolConfigured){
            return;
        }
    }
    U2OpStatus2Log os;
    ExternalToolSupportSettings::checkTemporaryDir(os);
    CHECK_OP(os, );
    
    //Call select input file and setup settings dialog
    BlastPlusWithExtFileSpecifySupportRunDialog blastPlusRunDialog(lastDBPath, lastDBName, AppContext::getMainWindow()->getQMainWindow());
    if(blastPlusRunDialog.exec() != QDialog::Accepted){
        return;
    }
    QList<BlastTaskSettings> settingsList = blastPlusRunDialog.getSettingsList();
    BlastPlusSupportMultiTask* blastPlusSupportMultiTask = new BlastPlusSupportMultiTask(settingsList,settingsList[0].outputResFile);
    AppContext::getTaskScheduler()->registerTopLevelTask(blastPlusSupportMultiTask);
}

////////////////////////////////////////
//BlastPlusSupportContext
BlastPlusSupportContext::BlastPlusSupportContext(QObject* p) : GObjectViewWindowContext(p, ANNOTATED_DNA_VIEW_FACTORY_ID) {
    toolList << BLASTN_TOOL_NAME << BLASTP_TOOL_NAME << BLASTX_TOOL_NAME << TBLASTN_TOOL_NAME << TBLASTX_TOOL_NAME << RPSBLAST_TOOL_NAME;
    lastDBName="";
    lastDBPath="";
}

void BlastPlusSupportContext::initViewContext(GObjectView* view) {
    AnnotatedDNAView* av = qobject_cast<AnnotatedDNAView*>(view);
    assert(av!=NULL);
    Q_UNUSED(av);

    ExternalToolSupportAction* queryAction = new ExternalToolSupportAction(this, view, tr("Query with BLAST+..."), 2000, toolList);

    addViewAction(queryAction);
    connect(queryAction, SIGNAL(triggered()), SLOT(sl_showDialog()));
}

void BlastPlusSupportContext::buildMenu(GObjectView* view, QMenu* m) {
        QList<GObjectViewAction *> actions = getViewActions(view);
        QMenu* analyseMenu = GUIUtils::findSubMenu(m, ADV_MENU_ANALYSE);
        assert(analyseMenu!=NULL);
        foreach(GObjectViewAction* a, actions) {
                a->addToMenuWithOrder(analyseMenu);
        }
}

void BlastPlusSupportContext::sl_showDialog() {
    //Check that any of BLAST+ tools and tempory directory path defined
    bool isOneOfToolConfigured=false;
    foreach(QString toolName, toolList){
        if(!AppContext::getExternalToolRegistry()->getByName(toolName)->getPath().isEmpty()){
            isOneOfToolConfigured=true;
        }
    }
    if (!isOneOfToolConfigured){
        QMessageBox msgBox;
        msgBox.setWindowTitle("BLAST+ Search");
        msgBox.setText(tr("Path for BLAST+ tools not selected."));
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
        bool isOneOfToolConfigured=false;
        foreach(QString toolName, toolList){
            if(!AppContext::getExternalToolRegistry()->getByName(toolName)->getPath().isEmpty()){
                isOneOfToolConfigured=true;
            }
        }
        if (!isOneOfToolConfigured){
            return;
        }
    }
    
    U2OpStatus2Log os(LogLevel_DETAILS);
    ExternalToolSupportSettings::checkTemporaryDir(os);
    CHECK_OP(os, );

    QAction* a = (QAction*)sender();
    GObjectViewAction* viewAction = qobject_cast<GObjectViewAction*>(a);
    AnnotatedDNAView* av = qobject_cast<AnnotatedDNAView*>(viewAction->getObjectView());
    assert(av);

    ADVSequenceObjectContext* seqCtx = av->getSequenceInFocus();
    BlastPlusSupportRunDialog dlg(seqCtx->getSequenceObject(), lastDBPath, lastDBName, av->getWidget());
    //Call run blastall dialog
    if(dlg.exec() == QDialog::Accepted) {

        BlastTaskSettings settings = dlg.getSettings();
        //prepare query
        DNASequenceSelection* s = seqCtx->getSequenceSelection();
        QVector<U2Region> regions;
        if (s->isEmpty()) {
            regions.append(U2Region(0, seqCtx->getSequenceLength()));
        } else {
            regions = s->getSelectedRegions();
        }
        foreach(const U2Region& r, regions) {
            settings.querySequence = seqCtx->getSequenceData(r);
//            DNATranslation * aminoT = (dlg.translateToAmino ? seqCtx->getAminoTT() : 0);
//            DNATranslation * complT = (dlg.translateToAmino ? seqCtx->getComplementTT() : 0);
            settings.offsInGlobalSeq=r.startPos;
            Task * t=NULL;
            if(settings.programName == "blastn"){
                t = new BlastNPlusSupportTask(settings);
            }else if(settings.programName == "blastp" || settings.programName == "gpu-blastp"){
                t = new BlastPPlusSupportTask(settings);
            }else if(settings.programName == "blastx"){
                t = new BlastXPlusSupportTask(settings);
            }else if(settings.programName == "tblastn"){
                t = new TBlastNPlusSupportTask(settings);
            }else if(settings.programName == "tblastx"){
                t = new TBlastXPlusSupportTask(settings);
            }else if(settings.programName == "rpsblast"){
                t = new RPSBlastSupportTask(settings);
            }
            assert(t);
            AppContext::getTaskScheduler()->registerTopLevelTask( t );
        }
    }
}

}//namespace
