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

#ifndef _U2_SEPARATEPROCESSMONITOR_H_
#define _U2_SEPARATEPROCESSMONITOR_H_

#include <U2Lang/WorkflowMonitor.h>

namespace U2 {
namespace Workflow {

class SeparateProcessMonitor : public WorkflowMonitor {
    Q_OBJECT
public:
    SeparateProcessMonitor(WorkflowAbstractIterationRunner *task, const QList<Actor*> &procs);

    void parseLog(const QStringList &lines);
};

namespace Monitor {
class PackUtils {
public:
    static QString packFileInfo(const FileInfo &info);
    static bool unpackFileInfo(const QString &line, FileInfo &info);

    static QString packProblem(const Problem &info);
    static bool unpackProblem(const QString &line, Problem &info);

    static QString packWorkerInfo(const QString &actorId, const WorkerInfo &info);
    static bool unpackWorkerInfo(const QString &line, QString &actorId, WorkerInfo &info);

    static QString packRunState(bool paused);
    static bool unpackRunState(const QString &line, bool &paused);
};
} // Monitor

} // Workflow
} // U2

#endif // _U2_SEPARATEPROCESSMONITOR_H_
