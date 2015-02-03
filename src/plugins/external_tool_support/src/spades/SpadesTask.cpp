
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

#include <QtCore/QDir>
#include <QtCore/QTextStream>
#include <QtCore/QFileInfo>

#include <U2Core/Counter.h>
#include <U2Core/DocumentUtils.h>
#include <U2Core/BaseDocumentFormats.h>
#include <U2Core/AppResources.h>
#include <U2Core/U2SafePoints.h>
#include <U2Core/FileAndDirectoryUtils.h>



#include "SpadesSupport.h"
#include "SpadesTask.h"


namespace U2 {
// SpadesTask

const QString SpadesTask::OPTION_DATASET_TYPE = "dataset-type";
const QString SpadesTask::OPTION_RUNNING_MODE = "running-mode";
const QString SpadesTask::OPTION_K_MER = "k-mer";
const QString SpadesTask::OPTION_THREADS = "threads";
const QString SpadesTask::OPTION_MEMLIMIT = "memlimit";
const QString SpadesTask::YAML_FILE_NAME = "datasets.yaml";
const QString SpadesTask::CONTIGS_NAME = "contigs.fasta";
const QString SpadesTask::SCAFFOLDS_NAME = "scaffolds.fasta";

SpadesTask::SpadesTask(const  GenomeAssemblyTaskSettings &settings):
    GenomeAssemblyTask(settings, TaskFlags_NR_FOSCOE)
{
    GCOUNTER(cvar, tvar, "SpadesTask");
}

void SpadesTask::prepare() {
    const QDir outDir = QFileInfo(settings.outDir.getURLString()).absoluteDir();
    if (!outDir.exists()) {
        stateInfo.setError(tr("Directory does not exist: ") + outDir.absolutePath());
        return ;
    }
    writeYamlReads();
    if(hasError()){
       return;
    }

    QStringList arguments;

    if(settings.getCustomValue(SpadesTask::OPTION_DATASET_TYPE, "Multi Cell").toString() == "Single Cell"){
        arguments.append("--sc");
    }

    QString runningMode = settings.getCustomValue(SpadesTask::OPTION_RUNNING_MODE, "Error Correction and Assembly").toString();
    if(runningMode == "Assembly only"){
        arguments.append("--only-assembler");
    }else if(runningMode == "Error correction only"){
        arguments.append("--only-error-correction");
    }

    arguments.append("--dataset");
    arguments.append(settings.outDir.getURLString() + QDir::separator() + SpadesTask::YAML_FILE_NAME);

    arguments.append("-t");
    arguments.append(settings.getCustomValue(SpadesTask::OPTION_THREADS, "16").toString());

    arguments.append("-m");
    arguments.append(settings.getCustomValue(SpadesTask::OPTION_MEMLIMIT, "250").toString());

    QString k = settings.getCustomValue(SpadesTask::OPTION_K_MER, "auto").toString();
    if(k != "auto"){
        arguments.append("-k");
        arguments.append(k);
    }

    arguments.append("-o");
    arguments.append(settings.outDir.getURLString());

    assemblyTask = new ExternalToolRunTask(ET_SPADES, arguments, new SpadesLogParser(), settings.outDir.getURLString());
    addSubTask(assemblyTask);
}

Task::ReportResult SpadesTask::report() {
    CHECK(!hasError(), ReportResult_Finished);
    CHECK(!isCanceled(), ReportResult_Finished);

    QString res = settings.outDir.getURLString() + QDir::separator() + SpadesTask::SCAFFOLDS_NAME;
    if(!FileAndDirectoryUtils::isFileEmpty(res)){
        resultUrl = res;
    }else{
        stateInfo.setError(QString("File %1 has not been found in output directory %2").arg(SpadesTask::SCAFFOLDS_NAME).arg(settings.outDir.getURLString()));
    }


    return ReportResult_Finished;
}

QList<Task *> SpadesTask::onSubTaskFinished(Task * /*subTask*/) {

    QList<Task *> result;

    return result;
}

void SpadesTask::writeYamlReads(){
    QFile yaml (settings.outDir.getURLString() + QDir::separator() + YAML_FILE_NAME);
    if(!yaml.open(QFile::WriteOnly)){
        stateInfo.setError(QString("Cannot open write settings file %1").arg(settings.outDir.getURLString() + QDir::separator() + YAML_FILE_NAME));
        return;
    }
    QString res = "";
    res.append("[\n");
    foreach (const AssemblyReads& r , settings.reads){
        res.append("{\n");
        res.append(QString("orientation: \"%1\",\n").arg(r.orientation));
        res.append(QString("type: \"%1\",\n").arg(GenomeAssemblyUtils::getYamlLibraryName(r.libName, r.libType)));
        if(!GenomeAssemblyUtils::hasRightReads(r.libName)){
            if(r.libName == LIBRARY_PAIRED_UNPAIRED || r.libName == LIBRARY_PAIRED_INTERLACED){
                res.append("interlaced reads: [\n");
            }else{
                res.append("single reads: [\n");
            }
            res.append(QString("\"%1\",\n").arg(r.left.getURLString()));
            res.append("]\n");
        }else{
            res.append("left reads: [\n");
            res.append(QString("\"%1\",\n").arg(r.left.getURLString()));
            res.append("],\n");
            res.append("right reads: [\n");
            res.append(QString("\"%1\",\n").arg(r.right.getURLString()));
            res.append("],\n");
        }
        res.append("},\n");

    }
    res.append("]\n");

    QTextStream outStream(&yaml);
    outStream << res;
}

// SpadesTaskFactory

GenomeAssemblyTask *SpadesTaskFactory::createTaskInstance(const GenomeAssemblyTaskSettings &settings) {
    return new SpadesTask(settings);
}

SpadesLogParser::SpadesLogParser():ExternalToolLogParser(){

}

void SpadesLogParser::parseOutput(const QString &partOfLog){
    lastPartOfLog=partOfLog.split(QRegExp("(\n|\r)"));
    lastPartOfLog.first()=lastLine+lastPartOfLog.first();
    lastLine=lastPartOfLog.takeLast();
    foreach(QString buf, lastPartOfLog){
        if(buf.contains("== Error == ")
            || buf.contains(" ERROR ")){
                coreLog.error("Spades: " + buf);
                setLastError(buf);
        }else if (buf.contains("== Warning == ")
                  || buf.contains(" WARN ")){
            algoLog.info(buf);
        }else {
            ioLog.trace(buf);
        }
    }
}

void SpadesLogParser::parseErrOutput(const QString &partOfLog){
    lastPartOfLog=partOfLog.split(QRegExp("(\n|\r)"));
    lastPartOfLog.first()=lastErrLine+lastPartOfLog.first();
    lastErrLine=lastPartOfLog.takeLast();
    foreach(QString buf, lastPartOfLog){
        if(buf.contains("== Error == ")
            || buf.contains(" ERROR ")){
                coreLog.error("Spades: " + buf);
                setLastError(buf);
        }else if (buf.contains("== Warning == ")
                  || buf.contains(" WARN ")){
            algoLog.info(buf);
        }else {
            algoLog.trace(buf);
        }
    }
}

} // namespace U2

