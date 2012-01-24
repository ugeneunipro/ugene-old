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

#include <U2Core/DocumentUtils.h>
#include <U2Core/BaseDocumentFormats.h>
#include <U2Core/AppResources.h>

#include "BwaSupport.h"
#include "BwaTask.h"

namespace U2 {

// BwaBuildIndexTask

BwaBuildIndexTask::BwaBuildIndexTask(const QString &referencePath, const QString &indexPath, const DnaAssemblyToRefTaskSettings &settings):
    Task("Build Bwa index", TaskFlags_NR_FOSCOE),
    referencePath(referencePath),
    indexPath(indexPath),
    settings(settings)
{
}

void BwaBuildIndexTask::prepare() {
    QStringList arguments;
    arguments.append("index");
    arguments.append("-a");
    arguments.append(settings.getCustomValue(BwaTask::OPTION_INDEX_ALGORITHM, "is").toString());
    if(settings.getCustomValue(BwaTask::OPTION_COLORSPACE, false).toBool()) {
        arguments.append("-c");
    }
    arguments.append("-p");
    arguments.append(indexPath);
    arguments.append(referencePath);
    ExternalToolRunTask *task = new ExternalToolRunTask(BWA_TOOL_NAME, arguments, &logParser);
    addSubTask(task);
}

// BwaBuildIndexTask::LogParser

BwaBuildIndexTask::LogParser::LogParser() {
}

void BwaBuildIndexTask::LogParser::parseOutput(const QString &partOfLog) {
    ExternalToolLogParser::parseOutput(partOfLog);
}

void BwaBuildIndexTask::LogParser::parseErrOutput(const QString &partOfLog) {
    ExternalToolLogParser::parseErrOutput(partOfLog);
}

// BwaAssembleTask

BwaAssembleTask::BwaAssembleTask(const QString &indexPath, const QString &readsPath, const QString &resultPath, const DnaAssemblyToRefTaskSettings &settings):
    Task("Bwa reads assembly", TaskFlags_NR_FOSCOE),
    indexPath(indexPath),
    readsPath(readsPath),
    resultPath(resultPath),
    settings(settings)
{
}

void BwaAssembleTask::prepare() {
    QStringList arguments;
    arguments.append("aln");

    arguments.append("-n");
    arguments.append(settings.getCustomValue(BwaTask::OPTION_N, 0.04).toString());

    arguments.append("-o");
    arguments.append(settings.getCustomValue(BwaTask::OPTION_MAX_GAP_OPENS, 1).toString());

    arguments.append("-e");
    arguments.append(settings.getCustomValue(BwaTask::OPTION_MAX_GAP_EXTENSIONS, -1).toString());

    arguments.append("-i");
    arguments.append(settings.getCustomValue(BwaTask::OPTION_INDEL_OFFSET, 5).toString());

    arguments.append("-d");
    arguments.append(settings.getCustomValue(BwaTask::OPTION_MAX_LONG_DELETION_EXTENSIONS, 10).toString());

    arguments.append("-l");
    arguments.append(settings.getCustomValue(BwaTask::OPTION_SEED_LENGTH, 32).toString());

    arguments.append("-k");
    arguments.append(settings.getCustomValue(BwaTask::OPTION_MAX_SEED_DIFFERENCES, 2).toString());

    arguments.append("-m");
    arguments.append(settings.getCustomValue(BwaTask::OPTION_MAX_QUEUE_ENTRIES, 2000000).toString());

    arguments.append("-t");
    arguments.append(settings.getCustomValue(BwaTask::OPTION_THREADS, 1).toString());

    arguments.append("-M");
    arguments.append(settings.getCustomValue(BwaTask::OPTION_MISMATCH_PENALTY, 3).toString());

    arguments.append("-O");
    arguments.append(settings.getCustomValue(BwaTask::OPTION_GAP_OPEN_PENALTY, 11).toString());

    arguments.append("-E");
    arguments.append(settings.getCustomValue(BwaTask::OPTION_GAP_EXTENSION_PENALTY, 4).toString());

    arguments.append("-R");
    arguments.append(settings.getCustomValue(BwaTask::OPTION_BEST_HITS, 30).toString());

    arguments.append("-q");
    arguments.append(settings.getCustomValue(BwaTask::OPTION_QUALITY_THRESHOLD, 0).toString());

    arguments.append("-B");
    arguments.append(settings.getCustomValue(BwaTask::OPTION_BARCODE_LENGTH, 0).toString());

    if(settings.getCustomValue(BwaTask::OPTION_COLORSPACE, false).toBool()) {
        arguments.append("-c");
    }

    if(settings.getCustomValue(BwaTask::OPTION_LONG_SCALED_GAP_PENALTY_FOR_LONG_DELETIONS, false).toBool()) {
        arguments.append("-L");
    }

    if(settings.getCustomValue(BwaTask::OPTION_NON_ITERATIVE_MODE, false).toBool()) {
        arguments.append("-N");
    }

    arguments.append("-f");
    arguments.append(resultPath + ".sai");
    arguments.append(indexPath);
    arguments.append(readsPath);
    alignTask = new ExternalToolRunTask(BWA_TOOL_NAME, arguments, &logParser);
    addSubTask(alignTask);
}

QList<Task *> BwaAssembleTask::onSubTaskFinished(Task *subTask) {
    QList<Task *> result;

    if(subTask == alignTask) {
        QStringList arguments;
        arguments.append("samse");
        arguments.append("-f");
        arguments.append(resultPath);
        arguments.append(indexPath);
        arguments.append(resultPath + ".sai");
        arguments.append(readsPath);
        ExternalToolRunTask *task = new ExternalToolRunTask(BWA_TOOL_NAME, arguments, &logParser);
        result.append(task);
    }

    return result;
}

// BwaAssembleTask::LogParser

BwaAssembleTask::LogParser::LogParser() {
}

void BwaAssembleTask::LogParser::parseOutput(const QString &partOfLog) {
    ExternalToolLogParser::parseErrOutput(partOfLog);
}

void BwaAssembleTask::LogParser::parseErrOutput(const QString &partOfLog) {
    ExternalToolLogParser::parseErrOutput(partOfLog);
}

// BwaTask

const QString BwaTask::taskName = "BWA";

const QString BwaTask::OPTION_INDEX_ALGORITHM = "index-algorithm";
const QString BwaTask::OPTION_N = "n";
const QString BwaTask::OPTION_MAX_GAP_OPENS = "max-gap-opens";
const QString BwaTask::OPTION_MAX_GAP_EXTENSIONS = "max-gap-extensions";
const QString BwaTask::OPTION_INDEL_OFFSET = "indel-offset";
const QString BwaTask::OPTION_MAX_LONG_DELETION_EXTENSIONS = "max-long-deletion-extensions";
const QString BwaTask::OPTION_SEED_LENGTH = "seed-length";
const QString BwaTask::OPTION_MAX_SEED_DIFFERENCES = "max-seed-differences";
const QString BwaTask::OPTION_MAX_QUEUE_ENTRIES = "max-queue-entries";
const QString BwaTask::OPTION_THREADS = "threads";
const QString BwaTask::OPTION_MISMATCH_PENALTY = "mismatch-penalty";
const QString BwaTask::OPTION_GAP_OPEN_PENALTY = "gap-open-penalty";
const QString BwaTask::OPTION_GAP_EXTENSION_PENALTY = "gap-extension-penalty";
const QString BwaTask::OPTION_BEST_HITS = "best-hits";
const QString BwaTask::OPTION_QUALITY_THRESHOLD = "quality-threshold";
const QString BwaTask::OPTION_BARCODE_LENGTH = "barcode-length";
const QString BwaTask::OPTION_COLORSPACE = "colorspace";
const QString BwaTask::OPTION_LONG_SCALED_GAP_PENALTY_FOR_LONG_DELETIONS = "long-scaled-gap-penalty-for-long-deletions";
const QString BwaTask::OPTION_NON_ITERATIVE_MODE = "non-iterative-mode";

BwaTask::BwaTask(const DnaAssemblyToRefTaskSettings &settings, bool justBuildIndex):
    DnaAssemblyToReferenceTask(settings, TaskFlags_NR_FOSCOE, justBuildIndex)
{
}

void BwaTask::prepare() {
    QString indexFileName = settings.indexFileName;
    if(indexFileName.isEmpty()) {
        if(settings.prebuiltIndex) {
            indexFileName = settings.refSeqUrl.dirPath() + "/" + settings.refSeqUrl.baseFileName();
        } else {
            indexFileName = settings.resultFileName.dirPath() + "/" + settings.resultFileName.baseFileName();
        }
    }
    if(!settings.prebuiltIndex) {
        buildIndexTask = new BwaBuildIndexTask(settings.refSeqUrl.getURLString(), indexFileName, settings);
    }
    if(!justBuildIndex) {
        if(settings.shortReadUrls.size() > 1) {
            setError(tr("Multiple read files are not supported"));
            return;
        }
        assembleTask = new BwaAssembleTask(indexFileName, settings.shortReadUrls.first().getURLString(), settings.resultFileName.getURLString(), settings);
    }

    if(!settings.prebuiltIndex) {
        addSubTask(buildIndexTask);
    } else if(!justBuildIndex) {
        addSubTask(assembleTask);
    } else {
        assert(false);
    }
}

Task::ReportResult BwaTask::report() {
    if(!justBuildIndex) {
        haveResults = true;
    }
    return ReportResult_Finished;
}

QList<Task *> BwaTask::onSubTaskFinished(Task *subTask) {
    QList<Task *> result;
    if((subTask == buildIndexTask) && !justBuildIndex) {
        result.append(assembleTask);
    }
    return result;
}

// BwaTaskFactory

DnaAssemblyToReferenceTask *BwaTaskFactory::createTaskInstance(const DnaAssemblyToRefTaskSettings &settings, bool justBuildIndex) {
    return new BwaTask(settings, justBuildIndex);
}

} // namespace U2
