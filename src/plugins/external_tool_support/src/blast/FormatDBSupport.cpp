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

#include "FormatDBSupport.h"
#include "FormatDBSupportRunDialog.h"
#include "FormatDBSupportTask.h"
#include "ExternalToolSupportSettingsController.h"
#include "ExternalToolSupportSettings.h"

#include <U2Core/AppContext.h>
#include <U2Core/AppSettings.h>
#include <U2Core/UserApplicationsSettings.h>
#include <U2Gui/MainWindow.h>

#include <U2Gui/GUIUtils.h>
#include <U2Misc/DialogUtils.h>
#include <U2View/MSAEditor.h>
#include <U2View/MSAEditorFactory.h>

#include <QtGui/QMainWindow>
#include <QtGui/QMessageBox>
#include <QtGui/QFileDialog>


namespace U2 {


FormatDBSupport::FormatDBSupport(const QString& name, const QString& path) : ExternalTool(name, path)
{
    if (AppContext::getMainWindow() != NULL) {
        icon = QIcon(":external_tool_support/images/ncbi.png");
        grayIcon = QIcon(":external_tool_support/images/ncbi_gray.png");
        warnIcon = QIcon(":external_tool_support/images/ncbi_warn.png");
    }
    assert((name == FORMATDB_TOOL_NAME)||(name == MAKEBLASTDB_TOOL_NAME));
    if(name == FORMATDB_TOOL_NAME){
#ifdef Q_OS_WIN
    executableFileName="formatdb.exe";
#else
    #ifdef Q_OS_LINUX
    executableFileName="formatdb";
    #endif
#endif
    validationArguments<<"--help";
    validMessage="formatdb";
    description=tr("The <i>formatdb</i> formats protein or"
        " nucleotide source databases before these databases"
        " can be searched by <i>blastall</i>.");

    versionRegExp=QRegExp("formatdb (\\d+\\.\\d+\\.\\d+)");
    toolKitName="BLAST";
    }else if(name == MAKEBLASTDB_TOOL_NAME){
#ifdef Q_OS_WIN
    executableFileName="makeblastdb.exe";
#else
    #ifdef Q_OS_LINUX
    executableFileName="makeblastdb";
    #endif
#endif
    validationArguments<<"-help";
    validMessage="makeblastdb";
    description=tr("The <i>makeblastdb</i> formats protein or"
                   " nucleotide source databases before these databases"
                   " can be searched by other BLAST+ tools.");
    versionRegExp=QRegExp("Application to create BLAST databases, version (\\d+\\.\\d+\\.\\d+\\+?)");
    toolKitName="BLAST+";
    }
}

void FormatDBSupport::sl_runWithExtFileSpecify(){
    //Check that formatDB or makeblastdb and tempory directory path defined
    if (path.isEmpty()){
        QMessageBox msgBox;
        if(name == FORMATDB_TOOL_NAME){
            msgBox.setWindowTitle("BLAST "+name);
            msgBox.setText(tr("Path for BLAST %1 tool not selected.").arg(name));
        }else{
            msgBox.setWindowTitle("BLAST+ "+name);
            msgBox.setText(tr("Path for BLAST+ %1 tool not selected.").arg(name));
        }
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
    FormatDBSupportTaskSettings settings;
    FormatDBSupportRunDialog formatDBRunDialog(settings, AppContext::getMainWindow()->getQMainWindow());
    if(formatDBRunDialog.exec() != QDialog::Accepted){
        return;
    }
    //assert(!settings.inputFilePath.isEmpty());
    //
    FormatDBSupportTask* formatDBSupportTask=new FormatDBSupportTask(name, settings);
    AppContext::getTaskScheduler()->registerTopLevelTask(formatDBSupportTask);
}
}//namespace
