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

#include "SpideySupport.h"
#include "SpideySupportTask.h"
#include "ExternalToolSupportSettingsController.h"
#include "ExternalToolSupportSettings.h"

#include <U2Core/AppContext.h>
#include <U2Core/AppSettings.h>
#include <U2Core/U2SafePoints.h>
#include <U2Core/U2OpStatusUtils.h>
#include <U2Gui/AppSettingsGUI.h>
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


SpideySupport::SpideySupport(const QString& name, const QString& path) : ExternalTool(name, path)
{
    /*if (AppContext::getMainWindow()) {
        viewCtx = NULL; //new SpideySupportContext(this);
        icon = QIcon(":external_tool_support/images/cmdline.png");
        grayIcon = QIcon(":external_tool_support/images/cmdline_gray.png");
        warnIcon = QIcon(":external_tool_support/images/cmdline_warn.png");
    }*/
#ifdef Q_OS_WIN
    executableFileName="Spidey.exe";
#else
    #if defined(Q_OS_LINUX) || defined(Q_OS_MAC)
    executableFileName="spidey";
    #endif
#endif
    validMessage="Spidey File_of_reads [options]";
    description=tr("<i>Spidey</i> is mRNA-to-DNA alignment program. \
                   <br>Binaries can be downloaded from http://www.ncbi.nlm.nih.gov/spidey/spideyexec.html");
    description+=tr("<br><br> Wheelan SJ, Church DM, Ostell JM.");
    description+=tr("<br>Spidey: a tool for mRNA-to-genomic alignments");
    description+=tr("<br>Genome Res. 2001 Nov;11(11):1952-7.");
    versionRegExp=QRegExp("VersionDate: (\\d+\\/\\d+\\/\\d+)");
    toolKitName="Spidey";
}

void SpideySupport::sl_runWithExtFileSpecify(){
    //Check that Spidey and temporary directory path defined
    /*if (path.isEmpty()){
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
    CHECK_OP(os,);

    //Call select input file and setup settings dialog
    // TODO: provide GUI to Spidey as additional feature
    SpideySupportTaskSettings settings;
    SpideySupportDialog SpideyDialog(settings, QApplication::activeWindow());
    
    if(SpideyDialog.exec() != QDialog::Accepted){
        return;
    }
    
    assert(!settings.inputFiles.isEmpty());

    SpideySupportTask* task = new SpideySupportTask(settings);
    AppContext::getTaskScheduler()->registerTopLevelTask(task);*/
    
}



}//namespace
