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

#include "CAP3Support.h"
#include "CAP3SupportDialog.h"
#include "CAP3SupportTask.h"
#include "ExternalToolSupportSettingsController.h"
#include "ExternalToolSupportSettings.h"

#include <U2Core/AppContext.h>
#include <U2Core/AppSettings.h>
#include <U2Core/U2SafePoints.h>
#include <U2Core/U2OpStatusUtils.h>
#include <U2Gui/AppSettingsGUI.h>
#include <U2Core/UserApplicationsSettings.h>
#include <U2Gui/MainWindow.h>

#if (QT_VERSION < 0x050000) //Qt 5
#include <QtGui/QMainWindow>
#include <QtGui/QMessageBox>
#else
#include <QtWidgets/QMainWindow>
#include <QtWidgets/QMessageBox>
#endif
#include <U2View/MSAEditor.h>
#include <U2View/MSAEditorFactory.h>

#include <U2Core/MAlignmentObject.h>

#include <U2Gui/GUIUtils.h>
#include <U2Gui/DialogUtils.h>

namespace U2 {


CAP3Support::CAP3Support(const QString& name, const QString& path) : ExternalTool(name, path)
{
    if (AppContext::getMainWindow()) {
        viewCtx = NULL; //new CAP3SupportContext(this);
        icon = QIcon(":external_tool_support/images/cmdline.png");
        grayIcon = QIcon(":external_tool_support/images/cmdline_gray.png");
        warnIcon = QIcon(":external_tool_support/images/cmdline_warn.png");
    }
#ifdef Q_OS_WIN
    executableFileName="cap3.exe";
#else
    #if defined(Q_OS_UNIX)
    executableFileName="cap3";
    #endif
#endif
    validMessage="cap3 File_of_reads \\[options\\]";
    description=tr("<i>CAP3</i> is a contig assembly program. \
                   <br>It allows to assembly long DNA reads (up to 1000 bp). \
                   <br>Binaries can be downloaded from http://seq.cs.iastate.edu/cap3.html");
    description+=tr("<br><br> Huang, X. and Madan, A.  (1999)");
    description+=tr("<br>CAP3: A DNA Sequence Assembly Program,");
    description+=tr("<br>Genome Research, 9: 868-877.");
    versionRegExp=QRegExp("VersionDate: (\\d+\\/\\d+\\/\\d+)");
    toolKitName="CAP3";
}

void CAP3Support::sl_runWithExtFileSpecify(){
    //Check that CAP3 and temporary directory path defined
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
    CHECK_OP(os,);

    //Call select input file and setup settings dialog
    CAP3SupportTaskSettings settings;
    CAP3SupportDialog cap3Dialog(settings, QApplication::activeWindow());

    if(cap3Dialog.exec() != QDialog::Accepted){
        return;
    }

    assert(!settings.inputFiles.isEmpty());

    RunCap3AndOpenResultTask* task = new RunCap3AndOpenResultTask(settings);
    AppContext::getTaskScheduler()->registerTopLevelTask(task);

}



}//namespace
