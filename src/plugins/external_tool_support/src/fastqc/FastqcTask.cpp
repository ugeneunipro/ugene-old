/**
 * UGENE - Integrated Bioinformatics Tools.
 * Copyright (C) 2008-2016 UniPro <ugene@unipro.ru>
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

#include <QtCore/QDir>

#include "FastqcSupport.h"
#include "FastqcTask.h"

#include <U2Core/AppContext.h>
#include <U2Core/AppSettings.h>
#include <U2Core/UserApplicationsSettings.h>
#include <U2Core/U2SafePoints.h>
#include <U2Core/ExternalToolRegistry.h>
#include <U2Core/GUrlUtils.h>
#include <U2Core/ExternalToolRunTask.h>

namespace U2 {

//////////////////////////////////////////////////////////////////////////
//FastQCParser

FastQCParser::FastQCParser()
    :ExternalToolLogParser(),progress(-1)
{

}

void FastQCParser::parseOutput( const QString& partOfLog ){
    ExternalToolLogParser::parseOutput(partOfLog);
}

void FastQCParser::parseErrOutput( const QString& partOfLog ){
    lastPartOfLog=partOfLog.split(QRegExp("(\n|\r)"));
    lastPartOfLog.first()=lastErrLine+lastPartOfLog.first();
    lastErrLine=lastPartOfLog.takeLast();
    foreach(const QString& buf, lastPartOfLog){
        if(buf.contains("ERROR", Qt::CaseInsensitive)){
            coreLog.error("FastQC: " + buf);
        }
    }
}

int FastQCParser::getProgress(){
    //parsing Approx 20% complete for filename
    if(!lastPartOfLog.isEmpty()){
        QString lastMessage=lastPartOfLog.last();
        QRegExp rx("Approx (\\d+)% complete");
        if(lastMessage.contains(rx)){
            SAFE_POINT(rx.indexIn(lastMessage) > -1, "bad progress index", 0);
            int step = rx.cap(1).toInt();
            if(step > progress){
                return  progress = step;
            }
        }
    }
    return progress;
}



//////////////////////////////////////////////////////////////////////////
//FastQCTask
FastQCTask::FastQCTask(const FastQCSetting &settings)
:ExternalToolSupportTask(QString("FastQC for %1").arg(settings.inputUrl), TaskFlags_FOSE_COSC)
,settings(settings)
{

}

void FastQCTask::prepare(){
    if (settings.inputUrl.isEmpty()){
        setError("No input URL");
        return ;
    }

    const QDir outDir = QFileInfo(settings.outDir).absoluteDir();
    if (!outDir.exists()) {
        setError("Directory does not exist: " + outDir.absolutePath());
        return ;
    }

    const QStringList args = getParameters(stateInfo);
    CHECK_OP(stateInfo, );

    ExternalToolRunTask* etTask = new ExternalToolRunTask(ET_FASTQC, args, new FastQCParser(), settings.outDir);
    setListenerForTask(etTask);
    addSubTask(etTask);
}

void FastQCTask::run(){
    CHECK_OP(stateInfo, );

    const QFileInfo resFile(getResFileUrl());
    if (!resFile.exists()) {
        setError("Result file does not exist: " + resFile.absoluteFilePath());
        return ;
    }
    resultUrl = getResFileUrl();
}

QString FastQCTask::getResFileUrl() const{
    QString res = "";

    QFileInfo fi(settings.inputUrl);
    QString name = fi.fileName();
    //taken from FastQC source "OfflineRunner.java"
    //.replaceAll("\\.gz$","").replaceAll("\\.bz2$","").replaceAll("\\.txt$","").replaceAll("\\.fastq$", "").replaceAll("\\.fq$", "").replaceAll("\\.csfastq$", "").replaceAll("\\.sam$", "").replaceAll("\\.bam$", "")+"_fastqc.html");
    name.replace(QRegExp(".gz$"),"")
            .replace(QRegExp(".bz2$"),"")
            .replace(QRegExp(".txt$"),"")
            .replace(QRegExp(".fastq$"), "")
            .replace(QRegExp(".csfastq$"), "")
            .replace(QRegExp(".sam$"), "")
            .replace(QRegExp(".bam$"), "");
    name += "_fastqc.html";

    res = settings.outDir + QDir::separator() + name;
    return res;
}

QStringList FastQCTask::getParameters(U2OpStatus & /*os*/) const{
    QStringList res;

    res << QString("-o");
    res << settings.outDir;


    if(!settings.conts.isEmpty()){
        res << QString("-c");
        res << settings.conts;
    }

    if(!settings.adapters.isEmpty()){
        res << QString("-a");
        res << settings.adapters;
    }

    ExternalTool *java = FastQCSupport::getJava();
    CHECK(NULL != java, res);
    res << QString("-java");
    res << java->getPath();

    res << settings.inputUrl;

    return res;
}

} //namespace U2
