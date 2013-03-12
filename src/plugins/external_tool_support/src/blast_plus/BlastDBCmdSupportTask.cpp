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

#include "BlastDBCmdSupportTask.h"
#include "BlastDBCmdSupport.h"

#include <U2Core/AppContext.h>
#include <U2Core/Counter.h>
#include <U2Core/DocumentModel.h>
#include <U2Core/ExternalToolRegistry.h>
#include <U2Core/ProjectModel.h>
#include <U2Core/MAlignmentObject.h>

#include <U2Core/AddDocumentTask.h>
#include <U2Core/Log.h>

namespace U2 {

void BlastDBCmdSupportTaskSettings::reset() {
    query = "";
    outputPath = "";
    databasePath = "";
    isNuclDatabase = true;
}

BlastDBCmdSupportTask::BlastDBCmdSupportTask(const BlastDBCmdSupportTaskSettings& _settings) :
        Task("Run NCBI BlastDBCmd task", TaskFlags_NR_FOSCOE),settings(_settings)
{
    GCOUNTER( cvar, tvar, "BlastDBCmdSupportTask" );
    blastDBCmdTask=NULL;
    logParser=NULL;
    toolName = BLASTDBCMD_TOOL_NAME;
}

void BlastDBCmdSupportTask::prepare(){
    
    QStringList arguments;
    
    arguments << "-db" << settings.databasePath;
    arguments << "-dbtype" << (settings.isNuclDatabase ? "nucl" : "prot");
    arguments << "-entry" << settings.query;
    arguments << "-logfile" << settings.outputPath+"MakeBLASTDB.log";
    arguments << "-out" << settings.outputPath;
    
    logParser=new ExternalToolLogParser();
    blastDBCmdTask=new ExternalToolRunTask(toolName, arguments, logParser);
    blastDBCmdTask->setSubtaskProgressWeight(95);
    addSubTask(blastDBCmdTask);
}
Task::ReportResult BlastDBCmdSupportTask::report(){
    return ReportResult_Finished;
}

}//namespace
