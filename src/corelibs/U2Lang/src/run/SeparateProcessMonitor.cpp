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

#include <U2Core/U2SafePoints.h>

#include "SeparateProcessMonitor.h"

namespace U2 {
namespace Workflow {
using namespace Monitor;

static const QString SEP(":;");
static const QString ADD_FILE_TAG("ADD_FILE_TAG");
static const QString PROMLEM_TAG("PROBLEM_TAG");
static const QString WORKER_TAG("WORKER_INFO_TAG");
static const QString RUN_STATE_TAG("RUN_STATE_TAG");

SeparateProcessMonitor::SeparateProcessMonitor(WorkflowAbstractIterationRunner *task, Schema *schema)
: WorkflowMonitor(task, schema)
{

}

inline QString cutLogStart(const QString &line, const QString &tag) {
    return line.mid(line.indexOf(tag));
}

void SeparateProcessMonitor::parseLog(const QStringList &lines) {
    foreach (const QString &line, lines) {
        if (line.contains(ADD_FILE_TAG)) {
            FileInfo info("", "");
            if (PackUtils::unpackFileInfo(cutLogStart(line, ADD_FILE_TAG), info)) {
                addOutputFile(info.url, info.actor);
            }
        } else if (line.contains(PROMLEM_TAG)) {
            Problem info("", "");
            if (PackUtils::unpackProblem(cutLogStart(line, PROMLEM_TAG), info)) {
                addProblem(info);
            }
        } else if (line.contains(WORKER_TAG)) {
            WorkerInfo info;
            QString actorId;
            if (PackUtils::unpackWorkerInfo(cutLogStart(line, WORKER_TAG), actorId, info)) {
                setWorkerInfo(actorId, info);
            }
        } else if (line.contains(RUN_STATE_TAG)) {
            bool paused = false;
            if (PackUtils::unpackRunState(cutLogStart(line, RUN_STATE_TAG), paused)) {
                setRunState(paused);
            }
        }
    }
}

QString PackUtils::packFileInfo(const FileInfo &info) {
    return ADD_FILE_TAG + SEP + info.actor + SEP + info.url;
}

bool PackUtils::unpackFileInfo(const QString &line, FileInfo &info) {
    QStringList words = line.split(SEP);
    CHECK(3 == words.size(), false);

    info = FileInfo(words[2], words[1]);
    return true;
}

QString PackUtils::packProblem(const Problem &info) {
    return PROMLEM_TAG + SEP + info.actor + SEP + info.message;
}

bool PackUtils::unpackProblem(const QString &line, Problem &info) {
    QStringList words = line.split(SEP);
    CHECK(3 == words.size(), false);

    info = Problem(words[2], words[1]);
    return true;
}

QString PackUtils::packWorkerInfo(const QString &actorId, const WorkerInfo &info) {
    return WORKER_TAG + SEP + actorId + SEP + QString::number(info.ticks) + SEP + QString::number(info.timeMks);
}

bool PackUtils::unpackWorkerInfo(const QString &line, QString &actorId, WorkerInfo &info) {
    QStringList words = line.split(SEP);
    CHECK(4 == words.size(), false);

    bool ok = false;
    actorId = words[1];
    info.ticks = words[2].toInt(&ok);
    CHECK(ok, false);
    info.timeMks = words[3].toLongLong(&ok);
    CHECK(ok, false);

    return true;
}

QString PackUtils::packRunState(bool paused) {
    return RUN_STATE_TAG + SEP + QString::number(int(paused));
}

bool PackUtils::unpackRunState(const QString &line, bool &paused) {
    QStringList words = line.split(SEP);
    CHECK(2 == words.size(), false);

    bool ok = false;
    paused = words[1].toInt(&ok);
    return ok;
}

} // Workflow
} // U2
