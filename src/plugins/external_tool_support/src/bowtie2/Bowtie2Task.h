/**
 * UGENE - Integrated Bioinformatics Tools.
 * Copyright (C) 2008-2014 UniPro <ugene@unipro.ru>
 * http://ugene.unipro.ru
 *
 * This program is free software; you can redistribute it and/or * modify it under the terms of the GNU General Public License
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

#ifndef _U2_BOWTIE2_TASK_H
#define _U2_BOWTIE2_TASK_H

#include <U2Algorithm/DnaAssemblyTask.h>

#include <U2Core/ExternalToolRunTask.h>

namespace U2 {

class Bowtie2BuildIndexTask : public Task {
    Q_OBJECT
public:
    Bowtie2BuildIndexTask(const QString &referencePath, const QString &indexPath);

    void prepare();
private:
    ExternalToolLogParser logParser;
    QString referencePath;
    QString indexPath;
};

class Bowtie2AlignTask : public Task {
    Q_OBJECT
public:
    Bowtie2AlignTask(const DnaAssemblyToRefTaskSettings &settings);
    void prepare();
private:
    ExternalToolLogParser logParser;
    DnaAssemblyToRefTaskSettings settings;
};

class Bowtie2Task : public DnaAssemblyToReferenceTask {
    Q_OBJECT
    DNA_ASSEMBLEY_TO_REF_TASK_FACTORY(Bowtie2Task)
public:
    Bowtie2Task(const DnaAssemblyToRefTaskSettings &settings, bool justBuildIndex = false);

    void prepare();
    ReportResult report();
protected slots:
    QList<Task *> onSubTaskFinished(Task *subTask);
public:
    static const QString OPTION_MODE;
    static const QString OPTION_MISMATCHES;
    static const QString OPTION_SEED_LEN;
    static const QString OPTION_DPAD;
    static const QString OPTION_GBAR;
    static const QString OPTION_SEED;
    static const QString OPTION_OFFRATE;
    static const QString OPTION_THREADS;

    static const QString OPTION_NOMIXED;
    static const QString OPTION_NODISCORDANT;
    static const QString OPTION_NOFW;
    static const QString OPTION_NORC;
    static const QString OPTION_NOOVERLAP;
    static const QString OPTION_NOCONTAIN;

private:
    Bowtie2BuildIndexTask *buildIndexTask;
    Bowtie2AlignTask *alignTask;
};

class Bowtie2TaskFactory : public DnaAssemblyToRefTaskFactory {
public:
    DnaAssemblyToReferenceTask *createTaskInstance(const DnaAssemblyToRefTaskSettings &settings, bool justBuildIndex = false);
};

} // namespace U2

#endif // _U2_BOWTIE2_TASK_H
