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

#include "TabixSupportTask.h"
#include "TabixSupport.h"

#include <U2Core/Task.h>
#include <U2Core/AppContext.h>
#include <U2Core/AppSettings.h>
#include <U2Core/UserApplicationsSettings.h>
#include <U2Core/GUrlUtils.h>


namespace U2 {

// TabixSupportTask
TabixSupportTask::TabixSupportTask(const GUrl& fileUrl, const GUrl& outputUrl)
    : ExternalToolSupportTask(tr("Generate index with Tabix task"), TaskFlags_NR_FOSE_COSC),
      fileUrl(fileUrl),
      bgzfUrl(outputUrl),
      bgzipTask(NULL),
      copyTask(NULL),
      tabixTask(NULL),
      logParser(NULL)
{
}

TabixSupportTask::~TabixSupportTask() {
    delete logParser;
}

void TabixSupportTask::prepare() {
    algoLog.details(tr("Tabix indexing started"));

    if ( BgzipTask::checkBgzf( fileUrl )) {
        algoLog.info(tr("Input file '%1' is already bgzipped").arg(fileUrl.getURLString()));

        copyTask = new CopyFileTask( fileUrl, bgzfUrl);
        addSubTask(copyTask);
        return;
    }

    if (bgzfUrl.isEmpty()) {
        bgzfUrl = GUrl(fileUrl.getURLString() + ".gz");
    }

    algoLog.info(tr("Saving data to file '%1'").arg(bgzfUrl.getURLString()));

    bgzipTask = new BgzipTask(fileUrl, bgzfUrl);
    addSubTask(bgzipTask);
}

QList<Task*> TabixSupportTask::onSubTaskFinished(Task *subTask) {
    QList<Task*> res;

    if (hasError() || isCanceled()) {
        return res;
    }
    if ((subTask != bgzipTask) && (subTask != copyTask)) {
        return res;
    }

    initTabixTask();
    res.append(tabixTask);
    return res;
}

const GUrl& TabixSupportTask::getOutputBgzf() const {
    return bgzfUrl;
}

const GUrl TabixSupportTask::getOutputTbi() const {
    GUrl tbi(bgzfUrl.getURLString() + ".tbi");
    return tbi;
}

void TabixSupportTask::initTabixTask() {
    QStringList arguments;
    arguments << "-f";
    arguments << bgzfUrl.getURLString();
    logParser = new ExternalToolLogParser();
    tabixTask = new ExternalToolRunTask(ET_TABIX, arguments, logParser);
    setListenerForTask(tabixTask);
}

// CopyFileTask
CopyFileTask::CopyFileTask(const GUrl &from, const GUrl &to)
    : Task(tr("Copy file task"), (TaskFlag)(TaskFlag_ReportingIsSupported | TaskFlag_ReportingIsEnabled)),
      from(from),
      to(to)
{
}

void CopyFileTask::run() {
    if (from.isEmpty()) {
        setError(tr("Input file is not set"));
        return;
    }
    if (to.isEmpty()) {
        setError(tr("Output file is not set"));
        return;
    }

    taskLog.details(tr("Copy %1 to %2").arg(from.getURLString()).arg(to.getURLString()));
    if (QFile::exists(to.getURLString())) {
        bool res = GUrlUtils::renameFileWithNameRoll(to.getURLString(), stateInfo);
        if (!res) {
            setError(tr("Can not rename existing file '%1'").arg(to.getURLString()));
            return;
        }
    }
    bool res = QFile::copy(from.getURLString(), to.getURLString());
    if (!res) {
        setError(tr("Error copying file").arg(from.getURLString()));
        return;
    }
    taskLog.details(tr("File copying finished"));
}

QString CopyFileTask::generateReport() const {
    if (hasError() || isCanceled()) {
        return tr("File copying task was finished with an error: %1").arg(getError());
    }
    return tr("File copy was finished. Copy of '%1' is '%2'").arg(from.getURLString()).arg(to.getURLString());
}

} // namespace U2
