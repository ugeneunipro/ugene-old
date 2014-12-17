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

#include <QtCore/QDir>

#include "SnpEffSupport.h"
#include "SnpEffTask.h"

#include <U2Core/AppContext.h>
#include <U2Core/AppSettings.h>
#include <U2Core/UserApplicationsSettings.h>
#include <U2Core/U2SafePoints.h>
#include <U2Core/ExternalToolRegistry.h>
#include <U2Core/GUrlUtils.h>
#include <U2Core/ExternalToolRunTask.h>

namespace U2 {

#define RES_FILE_BASE "snp_eff"
#define GENE_FILE "snpEff_genes.txt"
#define SUMMARY_FILE "snpEff_summary.html"

//////////////////////////////////////////////////////////////////////////
//SnpEffParser
SnpEffParser::SnpEffParser()
    :ExternalToolLogParser() {

}

void SnpEffParser::parseOutput( const QString& partOfLog ){
    ExternalToolLogParser::parseOutput(partOfLog);
}

void SnpEffParser::parseErrOutput( const QString& partOfLog ){
    lastPartOfLog=partOfLog.split(QRegExp("(\n|\r)"));
    lastPartOfLog.first()=lastErrLine+lastPartOfLog.first();
    lastErrLine=lastPartOfLog.takeLast();
    foreach(QString buf, lastPartOfLog){
            if(buf.contains("ERROR", Qt::CaseInsensitive)){
                    coreLog.error("SnpEff: " + buf);
            }
    }
}


//////////////////////////////////////////////////////////////////////////
//SnpEffTask
SnpEffTask::SnpEffTask(const SnpEffSetting &settings)
:Task(QString("snpEff for %1").arg(settings.inputUrl), TaskFlags_FOSE_COSC)
,settings(settings)
{

}

void SnpEffTask::prepare(){

    if (settings.inputUrl.isEmpty()){
        setError("No input URL");
        return ;
    }

    const QDir outDir = QFileInfo(settings.outDir).absoluteDir();
    if (!outDir.exists()) {
        setError("Directory does not exist: " + outDir.absolutePath());
        return ;
    }

    if(settings.genome.isEmpty()){
        setError("No path to genome lengths");
        return ;
    }

    const QStringList args = getParameters(stateInfo);
    CHECK_OP(stateInfo, );

    ExternalToolLogParser* logParser = new SnpEffParser();
    ExternalToolRunTask* etTask = new ExternalToolRunTask(ET_SNPEFF, args, logParser, settings.outDir);
    etTask->setStandartOutputFile( getResFileUrl() );
    addSubTask(etTask);
}

void SnpEffTask::run(){
    CHECK_OP(stateInfo, );

    const QFileInfo resFile = QFileInfo(settings.outDir + "/" + GENE_FILE);
    if (!resFile.exists()) {
        setError("Result file does not exist: " + resFile.absoluteFilePath());
        return ;
    }
    resultUrl = settings.outDir + "/" + GENE_FILE;
}

QString SnpEffTask::getSummaryUrl(){
    QString res = "";
    const QFileInfo resFile = QFileInfo(getResFileUrl());
    if (!resFile.exists()) {
        return res;
    }
    res = getResFileUrl();
    return res;
}

QString SnpEffTask::getResFileUrl(){
    QString res = "";
    res = settings.outDir + "/" + RES_FILE_BASE + "." + settings.outFormat;
    return res;
}

QString SnpEffTask::getDataPath(){
    return AppContext::getAppSettings()->getUserAppsSettings()->getDownloadDirPath() + "/" + "snpeff_data";
}

QStringList SnpEffTask::getParameters(U2OpStatus & /*os*/){
    QStringList res;

    res << QString("-dataDir %1").arg( GUrlUtils::getQuotedString(getDataPath()) );

    res << QString("-i %1").arg(settings.inFormat);
    res << QString("-o %1").arg(settings.outFormat);
    res << QString("-upDownStreamLen %1").arg(settings.updownLength);

    if (settings.homohetero.size() != 0){
        res << settings.homohetero;
    }

    if (settings.seqChange.size() != 0){
        res << settings.seqChange;
    }

    if (settings.filterOut.size() != 0){
        QStringList pars = settings.filterOut.split(",");
        foreach(const QString &p, pars){
            res << p;
        }
    }

    if (settings.chrPos.size() != 0){
        res << settings.chrPos;
    }

    res << QString("-v");

    res << settings.genome;

    res << GUrlUtils::getQuotedString(settings.inputUrl);

    return res;
}

} //namespace U2

