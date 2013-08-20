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

#include "FormatDBSupportTask.h"
#include "FormatDBSupport.h"

#include <U2Core/AppContext.h>
#include <U2Core/Counter.h>
#include <U2Core/DocumentModel.h>
#include <U2Core/ExternalToolRegistry.h>
#include <U2Core/ProjectModel.h>
#include <U2Core/MAlignmentObject.h>

#include <U2Core/AddDocumentTask.h>
#include <U2Core/Log.h>

namespace U2 {

void FormatDBSupportTaskSettings::reset() {
    inputFilesPath=QList<QString>();
    outputPath="";
    databaseTitle="";
    typeOfFile=true;
}

FormatDBSupportTask::FormatDBSupportTask(const QString& name, const FormatDBSupportTaskSettings& _settings) :
        Task("Run NCBI FormatDB task", TaskFlags_NR_FOSCOE), toolName(name),
        settings(_settings)
{
    GCOUNTER( cvar, tvar, "FormatDBSupportTask" );
    formatDBTask=NULL;
    logParser=NULL;
}

void FormatDBSupportTask::prepare(){
    QStringList arguments;
    assert((toolName == ET_FORMATDB)||(toolName == ET_MAKEBLASTDB));
    if(toolName == ET_FORMATDB){
        for(int i=0; i< settings.inputFilesPath.length(); i++){
            if(settings.inputFilesPath[i].contains(" ")){
                stateInfo.setError(tr("Input files paths contain space characters."));
                return;
            }
        }
        arguments <<"-i"<< settings.inputFilesPath.join(" ");
        arguments <<"-l"<< settings.outputPath+"formatDB.log";
        arguments <<"-n"<< settings.outputPath;
        arguments <<"-p"<< (settings.typeOfFile ? "T" : "F");
    }else if (toolName == ET_MAKEBLASTDB){
        for(int i=0; i< settings.inputFilesPath.length(); i++){
            settings.inputFilesPath[i]="\""+settings.inputFilesPath[i]+"\"";
        }
        arguments <<"-in"<< settings.inputFilesPath.join(" ");
        arguments <<"-logfile"<< settings.outputPath+"MakeBLASTDB.log";
        if(settings.outputPath.contains(" ")){
            stateInfo.setError(tr("Output database path contain space characters."));
            return;
        }
        arguments <<"-out"<< settings.outputPath;
        arguments <<"-dbtype"<< (settings.typeOfFile ? "prot" : "nucl");
    }

    logParser=new ExternalToolLogParser();
    formatDBTask=new ExternalToolRunTask(toolName, arguments, logParser);
    formatDBTask->setSubtaskProgressWeight(95);
    addSubTask(formatDBTask);
}
Task::ReportResult FormatDBSupportTask::report(){
    return ReportResult_Finished;
}

}//namespace
