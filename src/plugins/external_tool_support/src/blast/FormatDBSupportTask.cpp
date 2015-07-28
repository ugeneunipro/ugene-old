/**
 * UGENE - Integrated Bioinformatics Tools.
 * Copyright (C) 2008-2015 UniPro <ugene@unipro.ru>
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

#include <QtCore/QDir>

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
    isInputAmino=true;
}

FormatDBSupportTask::FormatDBSupportTask(const QString& name, const FormatDBSupportTaskSettings& _settings) :
        Task(tr("Run NCBI FormatDB task"), TaskFlags_NR_FOSCOE | TaskFlag_ReportingIsSupported | TaskFlag_ReportingIsEnabled), toolName(name),
        settings(_settings)
{
    GCOUNTER( cvar, tvar, "FormatDBSupportTask" );
    formatDBTask=NULL;
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
        arguments <<"-l"<< settings.outputPath + "formatDB.log";
        arguments <<"-n"<< settings.outputPath;
        arguments <<"-p"<< (settings.isInputAmino ? "T" : "F");
        externalToolLog = settings.outputPath + "formatDB.log";
    }else if (toolName == ET_MAKEBLASTDB){
        for(int i=0; i< settings.inputFilesPath.length(); i++){
            settings.inputFilesPath[i]="\""+settings.inputFilesPath[i]+"\"";
        }
        arguments <<"-in"<< settings.inputFilesPath.join(" ");
        arguments <<"-logfile"<< settings.outputPath + "MakeBLASTDB.log";
        externalToolLog = settings.outputPath + "MakeBLASTDB.log";
        if(settings.outputPath.contains(" ")){
            stateInfo.setError(tr("Output database path contain space characters."));
            return;
        }
        arguments <<"-out"<< settings.outputPath;
        arguments <<"-dbtype"<< (settings.isInputAmino ? "prot" : "nucl");
    }

    formatDBTask = new ExternalToolRunTask(toolName, arguments, new ExternalToolLogParser());
    formatDBTask->setSubtaskProgressWeight(95);
    addSubTask(formatDBTask);
}
Task::ReportResult FormatDBSupportTask::report(){
    return ReportResult_Finished;
}

QString FormatDBSupportTask::generateReport() const {
    QString res;
    if (isCanceled()) {
        res += QString(tr("Blast database creation has been cancelled")) + "<br>";
        if (QFile::exists(externalToolLog)) {
            res += prepareLink(externalToolLog);
        }
        return res;
    }
    if (!hasError()) {
        res += QString(tr("Blast database has been successfully created") + "<br><br>");
        res += QString(tr("Source sequences: "));
        foreach(const QString &filePath, settings.inputFilesPath){
            res += prepareLink(filePath);
            if(filePath.size() > 1){
                res += "<br>    ";
            }
        }
        res += "<br>";
        res += QString(tr("Database file path: %1")).arg(QDir::toNativeSeparators(settings.outputPath)) + "<br>";
        QString type = settings.isInputAmino ? "protein" : "nucleotide";
        res += QString(tr("Type: %1")).arg(type) + "<br>";
        if (QFile::exists(externalToolLog)) {
            res += QString(tr("Formatdb log file path: "));
            res += prepareLink(externalToolLog);
        }
    }else{
        res += QString(tr("Blast database creation has been failed")) + "<br><br>";
        if (QFile::exists(externalToolLog)) {
            res += QString(tr("Formatdb log file path: "));
            res += prepareLink(externalToolLog);
        }
    }
    return res;
}

QString FormatDBSupportTask::prepareLink( const QString &path ) const {
    QString preparedPath = path;
    if(preparedPath.startsWith("'") || preparedPath.startsWith("\"")) {
        preparedPath.remove(0,1);
    }
    if (preparedPath.endsWith("'") || preparedPath.endsWith("\"")) {
        preparedPath.chop(1);
    }
    return "<a href=\"file:///" + QDir::toNativeSeparators(preparedPath) + "\">" +
        QDir::toNativeSeparators(preparedPath) + "</a><br>";
}

}//namespace
