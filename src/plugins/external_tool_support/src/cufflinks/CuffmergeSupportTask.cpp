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

#include "CufflinksSupport.h"
#include "ExternalToolRunTask.h"

#include <U2Core/AppContext.h>
#include <U2Core/BaseDocumentFormats.h>
#include <U2Core/ExternalToolRegistry.h>
#include <U2Core/GUrlUtils.h>
#include <U2Core/U2SafePoints.h>

#include "CuffmergeSupportTask.h"

namespace U2 {

const QString CuffmergeSupportTask::outSubDirBaseName("cuffmerge_out");

CuffmergeSupportTask::CuffmergeSupportTask(const CuffmergeSettings &_settings)
: Task(tr("Running Cuffmerge task"), TaskFlags_FOSE_COSC), settings(_settings)
{
    mergeTask = NULL;
    fileNum = 0;
}

CuffmergeSupportTask::~CuffmergeSupportTask() {
    qDeleteAll(docs);
}

void CuffmergeSupportTask::prepare() {
    setupWorkingDirPath();

    settings.outDir = GUrlUtils::createDirectory(
                settings.outDir + "/" + outSubDirBaseName,
                "_", stateInfo);
    CHECK_OP(stateInfo, );

    foreach (const QList<SharedAnnotationData> &anns, settings.anns) {
        Task *t = createWriteTask(anns, getAnnsFilePath());
        CHECK_OP(stateInfo, );
        addSubTask(t);
    }
}

QList<Task*> CuffmergeSupportTask::onSubTaskFinished(Task *subTask) {
    if (writeTasks.contains(subTask)) {
        writeTasks.removeOne(subTask);
    }

    QList<Task*> result;
    if (writeTasks.isEmpty()) {
        if (NULL == mergeTask) {
            result << createCuffmergeTask();
        }
    }
    return result;
}

void CuffmergeSupportTask::run() {
    result = ExternalToolSupportUtils::getAnnotationsFromFile(
        outDir + "/merged.gtf",
        BaseDocumentFormats::GTF,
        CUFFMERGE_TOOL_NAME,
        stateInfo);

    ExternalToolSupportUtils::appendExistingFile(settings.outDir + "/merged.gtf", outputFiles);
    ExternalToolSupportUtils::appendExistingFile(settings.outDir + "/genes.fpkm_tracking", outputFiles);
    ExternalToolSupportUtils::appendExistingFile(settings.outDir + "/isoforms.fpkm_tracking", outputFiles);
    ExternalToolSupportUtils::appendExistingFile(settings.outDir + "/skipped.gtf", outputFiles);
    ExternalToolSupportUtils::appendExistingFile(settings.outDir + "/transcripts.gtf", outputFiles);
}

Task::ReportResult CuffmergeSupportTask::report() {
    settings.anns.clear();
    return ReportResult_Finished;
}

void CuffmergeSupportTask::setupWorkingDirPath() {
    if (0 == QString::compare(settings.workingDir, "default", Qt::CaseInsensitive)) {
        workingDir = ExternalToolSupportUtils::createTmpDir(CUFFMERGE_TMP_DIR, stateInfo);
    } else {
        workingDir = ExternalToolSupportUtils::createTmpDir(settings.workingDir, CUFFMERGE_TMP_DIR, stateInfo);
    }
}

QString CuffmergeSupportTask::getAnnsFilePath() {
    QString filePath = workingDir + QString("/tmp_%1.gtf").arg(fileNum);
    fileNum++;
    return filePath;
}

Task * CuffmergeSupportTask::createWriteTask(const QList<SharedAnnotationData> &anns, const QString &filePath) {
    Document *doc = ExternalToolSupportUtils::createAnnotationsDocument(
        filePath, BaseDocumentFormats::GTF, anns, stateInfo);
    CHECK_OP(stateInfo, NULL);
    docs << doc;
    SaveDocumentTask *t = new SaveDocumentTask(doc, doc->getIOAdapterFactory(), filePath);
    writeTasks << t;
    return t;
}

void CuffmergeSupportTask::writeFileList() {
    listFilePath = workingDir + "/gtf_list.txt";
    QFile file(listFilePath);
    bool res = file.open(QIODevice::WriteOnly);
    if (!res) {
        stateInfo.setError(tr("Can not create a file: %1").arg(listFilePath));
        return;
    }

    QString data;
    foreach (Document *doc, docs) {
        data += doc->getURLString() + "\n";
    }
    file.write(data.toAscii());
    file.close();
}

Task * CuffmergeSupportTask::createCuffmergeTask() {
    writeFileList();
    CHECK_OP(stateInfo, NULL);

    QStringList args;
    {
        if (!settings.refAnnsUrl.isEmpty()) {
            args << "--ref-gtf" << settings.refAnnsUrl;
        }
        if (!settings.refSeqUrl.isEmpty()) {
            args << "--ref-sequence" << settings.refSeqUrl;
        }
        outDir = workingDir + "/out";
        args << "-o" << outDir;
        args << "--min-isoform-fraction" << QString::number(settings.minIsoformFraction);
        args << listFilePath;
    }

    QStringList addPaths;
    {
        ExternalToolRegistry *registry = AppContext::getExternalToolRegistry();

        ExternalTool *cm =registry->getByName(CUFFMERGE_TOOL_NAME);
        ExternalTool *cc =registry->getByName(CUFFCOMPARE_TOOL_NAME);
        QFileInfo cmInfo(cm->getPath());
        QFileInfo ccInfo(cc->getPath());

        addPaths << cmInfo.dir().absolutePath();
        addPaths << ccInfo.dir().absolutePath();
    }

    logParser.reset(new ExternalToolLogParser());
    mergeTask = new ExternalToolRunTask(CUFFMERGE_TOOL_NAME, args, logParser.data(), workingDir, addPaths);
    return mergeTask;
}

QList<SharedAnnotationData> CuffmergeSupportTask::takeResult() {
    QList<SharedAnnotationData> ret = result;
    result.clear();
    return ret;
}

QStringList CuffmergeSupportTask::getOutputFiles() const {
    return outputFiles;
}

/************************************************************************/
/* CuffmergeSettings */
/************************************************************************/
CuffmergeSettings::CuffmergeSettings() {
    minIsoformFraction = 0.05;
}

} // U2
