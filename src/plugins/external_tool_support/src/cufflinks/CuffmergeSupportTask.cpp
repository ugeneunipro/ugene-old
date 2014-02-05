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

#include "CufflinksSupport.h"
#include "CufflinksSupportTask.h"

#include <U2Core/AppContext.h>
#include <U2Core/BaseDocumentFormats.h>
#include <U2Core/ExternalToolRegistry.h>
#include <U2Core/ExternalToolRunTask.h>
#include <U2Core/GUrlUtils.h>
#include <U2Core/U2SafePoints.h>

#include "tophat/TopHatSettings.h"

#include "CuffmergeSupportTask.h"

namespace U2 {

const QString CuffmergeSupportTask::outSubDirBaseName("cuffmerge_out");

CuffmergeSupportTask::CuffmergeSupportTask(const CuffmergeSettings &_settings)
: ExternalToolSupportTask(tr("Running Cuffmerge task"), TaskFlags_FOSE_COSC), settings(_settings)
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

    foreach (const QList<AnnotationData> &anns, settings.anns) {
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
    result = CufflinksSupportTask::getAnnotationsFromFile(
        settings.outDir + "/merged.gtf",
        BaseDocumentFormats::GTF,
        ET_CUFFMERGE,
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

Task * CuffmergeSupportTask::createWriteTask(const QList<AnnotationData> &anns, const QString &filePath) {
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
    file.write(data.toLatin1());
    file.close();
}

Task * CuffmergeSupportTask::createCuffmergeTask() {
    writeFileList();
    CHECK_OP(stateInfo, NULL);

    QStringList args;
    {
        args << "-p" << QString::number(TopHatSettings::getThreadsCount());
        if (!settings.refAnnsUrl.isEmpty()) {
            args << "--ref-gtf" << settings.refAnnsUrl;
        }
        if (!settings.refSeqUrl.isEmpty()) {
            args << "--ref-sequence" << settings.refSeqUrl;
        }
        args << "-o" << settings.outDir;
        args << "--min-isoform-fraction" << QString::number(settings.minIsoformFraction);
        args << listFilePath;
    }

    QStringList addPaths;
    {
        ExternalToolRegistry *registry = AppContext::getExternalToolRegistry();

        ExternalTool *cm =registry->getByName(ET_CUFFMERGE);
        ExternalTool *cc =registry->getByName(ET_CUFFCOMPARE);
        QFileInfo cmInfo(cm->getPath());
        QFileInfo ccInfo(cc->getPath());

        addPaths << cmInfo.dir().absolutePath();
        addPaths << ccInfo.dir().absolutePath();
    }

    logParser.reset(new ExternalToolLogParser());
    mergeTask = new ExternalToolRunTask(ET_CUFFMERGE, args, logParser.data(), workingDir, addPaths);
    setListenerForTask(mergeTask);
    return mergeTask;
}

QList<AnnotationData> CuffmergeSupportTask::takeResult( ) {
    QList<AnnotationData> ret = result;
    result.clear( );
    return ret;
}

QStringList CuffmergeSupportTask::getOutputFiles( ) const {
    return outputFiles;
}

/************************************************************************/
/* CuffmergeSettings */
/************************************************************************/
CuffmergeSettings::CuffmergeSettings() {
    minIsoformFraction = 0.05;
}

} // U2
