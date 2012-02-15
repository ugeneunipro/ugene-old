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

#include <U2Core/DocumentUtils.h>
#include <U2Core/BaseDocumentFormats.h>
#include <U2Core/AppResources.h>

#include "BowtieSupport.h"
#include "BowtieTask.h"

namespace U2 {

// BowtieBuildIndexTask

BowtieBuildIndexTask::BowtieBuildIndexTask(const QString &referencePath, const QString &indexPath, bool colorspace):
    Task("Build Bowtie index", TaskFlags_NR_FOSCOE),
    referencePath(referencePath),
    indexPath(indexPath),
    colorspace(colorspace)
{
}

void BowtieBuildIndexTask::prepare() {
    {
        QFileInfo file(referencePath);
        if(!file.exists()) {
            stateInfo.setError(tr("Reference file \"%1\" does not exist").arg(referencePath));
            return;
        }
        qint64 memUseMB = file.size() * 3 / 1024 / 1024 + 100;
        coreLog.trace(QString("bowtie-build:Memory resourse %1").arg(memUseMB));
        addTaskResource(TaskResourceUsage(RESOURCE_MEMORY, memUseMB));
    }

    QStringList arguments;
    arguments.append(referencePath);
    arguments.append(indexPath);
    if(colorspace) {
        arguments.append("--color");
    }
    ExternalToolRunTask *task = new ExternalToolRunTask(BOWTIE_BUILD_TOOL_NAME, arguments, &logParser);
    addSubTask(task);
}

// BowtieBuildIndexTask::LogParser

BowtieBuildIndexTask::LogParser::LogParser():
    stage(PREPARE),
    substage(UNKNOWN),
    bucketSortIteration(0),
    blockIndex(0),
    blockCount(0),
    substageProgress(0),
    progress(0)
{
}

void BowtieBuildIndexTask::LogParser::parseOutput(const QString &partOfLog) {
    ExternalToolLogParser::parseOutput(partOfLog);
    foreach(const QString &buf, lastPartOfLog) {
        QRegExp blockRegExp("Getting block (\\d+) of (\\d+)");
        QRegExp percentRegexp("(\\d+)%");
        if(buf.contains("Binary sorting into buckets")) {
            if(BUCKET_SORT != substage) {
                bucketSortIteration = 0;
                substage = BUCKET_SORT;
                if(PREPARE == stage) {
                    stage = FORWARD_INDEX;
                } else if(FORWARD_INDEX == stage) {
                    stage = MIRROR_INDEX;
                }
            } else {
                bucketSortIteration++;
            }
            substageProgress = bucketSortIteration*30/5;
        } else if(buf.contains(blockRegExp)) {
            substage = GET_BLOCKS;
            blockIndex = blockRegExp.cap(1).toInt() - 1;
            blockCount = blockRegExp.cap(2).toInt();
            substageProgress = 30 + blockIndex*70/blockCount;
        } else if(buf.contains(percentRegexp)) {
            int percent = percentRegexp.cap(1).toInt();
            if(BUCKET_SORT == substage) {
                substageProgress = (bucketSortIteration*30 + percent*30/100)/5;
            } else if(GET_BLOCKS == substage) {
                substageProgress = 30 + (blockIndex*70 + percent*70/100)/blockCount;
            }
        }
        if(FORWARD_INDEX == stage) {
            progress = substageProgress/2;
        } else if(MIRROR_INDEX == stage) {
            progress = 50 + substageProgress/2;
        } else {
            progress = 0;
        }
    }
}

void BowtieBuildIndexTask::LogParser::parseErrOutput(const QString &partOfLog) {
    ExternalToolLogParser::parseErrOutput(partOfLog);
}

int BowtieBuildIndexTask::LogParser::getProgress() {
    return progress;
}

// BowtieAssembleTask

BowtieAssembleTask::BowtieAssembleTask(const DnaAssemblyToRefTaskSettings &settings):
    Task("Bowtie reads assembly", TaskFlags_NR_FOSCOE),
    settings(settings)
{
}

bool BowtieAssembleTask::isHaveResults()const {
    return logParser.isHaveResults();
}

void BowtieAssembleTask::prepare() {
    if(settings.indexFileName.isEmpty()) {
        if(settings.prebuiltIndex) {
            settings.indexFileName = settings.refSeqUrl.dirPath() + "/" + settings.refSeqUrl.baseFileName();
        } else {
            settings.indexFileName = settings.resultFileName.dirPath() + "/" + settings.resultFileName.baseFileName();
        }
    }
    {
        QString indexSuffixes[] = {".1.ebwt", ".2.ebwt", ".3.ebwt", ".4.ebwt", ".rev.1.ebwt", ".rev.2.ebwt" };
        for(int i=0; i < 6; i++) {
            QFileInfo file(settings.indexFileName + indexSuffixes[i]);
            if(!file.exists()) {
                stateInfo.setError(tr("Reference index file \"%1\" does not exist").arg(settings.indexFileName + indexSuffixes[i]));
                return;
            }
        }

        static const int SHORT_READ_AVG_LENGTH = 1000;
        QFileInfo file(settings.indexFileName + indexSuffixes[0]);
        qint64 memUseMB = (file.size() *  4 + SHORT_READ_AVG_LENGTH*10 ) / 1024 / 1024 + 100;
        addTaskResource(TaskResourceUsage(RESOURCE_MEMORY, memUseMB, false));
    }

    QStringList arguments;
    arguments.append(QString("-n"));
    arguments.append(settings.getCustomValue(BowtieTask::OPTION_N_MISMATCHES, 2).toString());
    {
        int vMismatches = settings.getCustomValue(BowtieTask::OPTION_V_MISMATCHES, -1).toInt();
        if(-1 != vMismatches) {
            arguments.append(QString("-v"));
            arguments.append(QString::number(vMismatches));
        }
    }
    arguments.append(QString("--maqerr"));
    arguments.append(settings.getCustomValue(BowtieTask::OPTION_MAQERR, 70).toString());
    arguments.append(QString("--seedlen"));
    arguments.append(settings.getCustomValue(BowtieTask::OPTION_SEED_LEN, 28).toString());
    if(settings.getCustomValue(BowtieTask::OPTION_NOMAQROUND, false).toBool()) {
        arguments.append("--nomaqround");
    }
    if(settings.getCustomValue(BowtieTask::OPTION_NOFW, false).toBool()) {
        arguments.append("--nofw");
    }
    if(settings.getCustomValue(BowtieTask::OPTION_NORC, false).toBool()) {
        arguments.append("--norc");
    }
    {
        int maxBts = settings.getCustomValue(BowtieTask::OPTION_MAXBTS, -1).toInt();
        if(-1 != maxBts) {
            arguments.append(QString("--maxbts"));
            arguments.append(QString::number(maxBts));
        }
    }
    if(settings.getCustomValue(BowtieTask::OPTION_TRYHARD, false).toBool()) {
        arguments.append("--tryhard");
    }
    arguments.append(QString("--chunkmbs"));
    arguments.append(settings.getCustomValue(BowtieTask::OPTION_CHUNKMBS, 64).toString());
    {
        int seed = settings.getCustomValue(BowtieTask::OPTION_SEED, -1).toInt();
        if(-1 != seed) {
            arguments.append(QString("--seed"));
            arguments.append(QString::number(seed));
        }
    }
    if(settings.getCustomValue(BowtieTask::OPTION_BEST, false).toBool()) {
        arguments.append("--best");
    }
    if(settings.getCustomValue(BowtieTask::OPTION_ALL, false).toBool()) {
        arguments.append("--all");
    }
    if(settings.getCustomValue(BowtieTask::OPTION_COLORSPACE, false).toBool()) {
        arguments.append("-C");
    }
    {
        int threads = settings.getCustomValue(BowtieTask::OPTION_THREADS, 1).toInt();
        arguments.append(QString("--threads"));
        arguments.append(QString::number(threads));
    }
    if(!settings.shortReadUrls.isEmpty())
    {
        QList<FormatDetectionResult> detectionResults = DocumentUtils::detectFormat(settings.shortReadUrls.first());
        if(!detectionResults.isEmpty()) {
            if(detectionResults.first().format->getFormatId() == BaseDocumentFormats::FASTA) {
                arguments.append("-f");
            } else if(detectionResults.first().format->getFormatId() == BaseDocumentFormats::RAW_DNA_SEQUENCE) {
                arguments.append("-r");
            }
        }
    }
    arguments.append("-S");
    arguments.append(settings.indexFileName);
    {
        QString readUrlsArgument;
        for(int index = 0;index < settings.shortReadUrls.size();index++) {
            if(0 != index) {
                readUrlsArgument.append(",");
            }
            readUrlsArgument.append(settings.shortReadUrls[index].getURLString());
        }
        arguments.append(readUrlsArgument);
    }
    arguments.append(settings.resultFileName.getURLString());
    ExternalToolRunTask *task = new ExternalToolRunTask(BOWTIE_TOOL_NAME, arguments, &logParser);
    addSubTask(task);
}

// BowtieAssembleTask::LogParser

BowtieAssembleTask::LogParser::LogParser():
    haveResults(false)
{
}

void BowtieAssembleTask::LogParser::parseOutput(const QString &partOfLog) {
    ExternalToolLogParser::parseErrOutput(partOfLog);
}

void BowtieAssembleTask::LogParser::parseErrOutput(const QString &partOfLog) {
    ExternalToolLogParser::parseErrOutput(partOfLog);
    QRegExp blockRegExp("# reads with at least one reported alignment: (\\d+) \\(\\d+\\.\\d+%\\)");
    foreach(const QString &buf, lastPartOfLog) {
        if(buf.contains(blockRegExp)) {
            if(blockRegExp.cap(1).toInt() > 0) {
                haveResults = true;
            }
        }
    }
}

bool BowtieAssembleTask::LogParser::isHaveResults()const {
    return haveResults;
}

// BowtieTask

const QString BowtieTask::taskName = "Bowtie";

const QString BowtieTask::OPTION_N_MISMATCHES = "n-mismatches";
const QString BowtieTask::OPTION_V_MISMATCHES = "v-mismatches";
const QString BowtieTask::OPTION_MAQERR = "maqerr";
const QString BowtieTask::OPTION_SEED_LEN = "seedLen";
const QString BowtieTask::OPTION_NOFW = "nofw";
const QString BowtieTask::OPTION_NORC = "norc";
const QString BowtieTask::OPTION_MAXBTS = "maxbts";
const QString BowtieTask::OPTION_TRYHARD = "tryhard";
const QString BowtieTask::OPTION_CHUNKMBS = "chunkmbs";
const QString BowtieTask::OPTION_NOMAQROUND = "nomaqround";
const QString BowtieTask::OPTION_SEED = "seed";
const QString BowtieTask::OPTION_BEST = "best";
const QString BowtieTask::OPTION_ALL = "all";
const QString BowtieTask::OPTION_COLORSPACE = "colorspace";
const QString BowtieTask::OPTION_THREADS = "threads";

BowtieTask::BowtieTask(const DnaAssemblyToRefTaskSettings &settings, bool justBuildIndex):
    DnaAssemblyToReferenceTask(settings, TaskFlags_NR_FOSCOE, justBuildIndex)
{
}

void BowtieTask::prepare() {
    if(!settings.prebuiltIndex) {
        QString indexFileName = settings.indexFileName;
        if(indexFileName.isEmpty()) {
            if(justBuildIndex) {
                indexFileName = settings.refSeqUrl.dirPath() + "/" + settings.refSeqUrl.baseFileName();
            } else {
                indexFileName = settings.resultFileName.dirPath() + "/" + settings.resultFileName.baseFileName();
            }
        }
        buildIndexTask = new BowtieBuildIndexTask(settings.refSeqUrl.getURLString(), indexFileName,
                                                  settings.getCustomValue(BowtieTask::OPTION_COLORSPACE, false).toBool());
    }
    if(!justBuildIndex) {
        assembleTask = new BowtieAssembleTask(settings);
    }

    if(!settings.prebuiltIndex) {
        addSubTask(buildIndexTask);
    } else if(!justBuildIndex) {
        addSubTask(assembleTask);
    } else {
        assert(false);
    }
}

Task::ReportResult BowtieTask::report() {
    if(!justBuildIndex) {
        haveResults = assembleTask->isHaveResults();
    }
    return ReportResult_Finished;
}

QList<Task *> BowtieTask::onSubTaskFinished(Task *subTask) {
    QList<Task *> result;
    if((subTask == buildIndexTask) && !justBuildIndex) {
        result.append(assembleTask);
    }
    return result;
}

// BowtieTaskFactory

DnaAssemblyToReferenceTask *BowtieTaskFactory::createTaskInstance(const DnaAssemblyToRefTaskSettings &settings, bool justBuildIndex) {
    return new BowtieTask(settings, justBuildIndex);
}

} // namespace U2
