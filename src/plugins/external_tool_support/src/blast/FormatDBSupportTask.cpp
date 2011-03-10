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
    assert((toolName == FORMATDB_TOOL_NAME)||(toolName == MAKEBLASTDB_TOOL_NAME));
    if(toolName == FORMATDB_TOOL_NAME){
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
    }else if (toolName == MAKEBLASTDB_TOOL_NAME){
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
