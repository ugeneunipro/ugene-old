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

#ifndef _U2_BWA_TASK_H_
#define _U2_BWA_TASK_H_

#include <U2Algorithm/DnaAssemblyTask.h>

#include "ExternalToolRunTask.h"

namespace U2 {

class BwaBuildIndexTask : public Task {
    Q_OBJECT
public:
    BwaBuildIndexTask(const QString &referencePath, const QString &indexPath, const DnaAssemblyToRefTaskSettings &settings);

    void prepare();
private:
    class LogParser : public ExternalToolLogParser {
    public:
        LogParser();

        void parseOutput(const QString &partOfLog);
        void parseErrOutput(const QString &partOfLog);
    };

    LogParser logParser;
    QString referencePath;
    QString indexPath;
    DnaAssemblyToRefTaskSettings settings;
};

class BwaAlignTask : public Task {
    Q_OBJECT
public:
    BwaAlignTask(const QString &indexPath, const QList<ShortReadSet>& shortReadSets, const QString &resultPath, const DnaAssemblyToRefTaskSettings &settings);

    void prepare();
protected slots:
    QList<Task *> onSubTaskFinished(Task *subTask);
private:
    class LogParser : public ExternalToolLogParser {
    public:
        LogParser();

        void parseOutput(const QString &partOfLog);
        void parseErrOutput(const QString &partOfLog);
    };

    QList<Task*> alignTasks;
    LogParser logParser;
    QString indexPath;
    QList<ShortReadSet> readSets;
    QString resultPath;
    DnaAssemblyToRefTaskSettings settings;
    bool alignmentPerformed;
    inline QString getSAIPath(const QString& pathToReads);
};

class BwaTask : public DnaAssemblyToReferenceTask {
    Q_OBJECT
    DNA_ASSEMBLEY_TO_REF_TASK_FACTORY(BwaTask)
public:
    BwaTask(const DnaAssemblyToRefTaskSettings &settings, bool justBuildIndex = false);

    void prepare();
    ReportResult report();
protected slots:
    QList<Task *> onSubTaskFinished(Task *subTask);
public:
    static const QString OPTION_INDEX_ALGORITHM;
    static const QString OPTION_N;
    static const QString OPTION_MAX_GAP_OPENS;
    static const QString OPTION_MAX_GAP_EXTENSIONS;
    static const QString OPTION_INDEL_OFFSET;
    static const QString OPTION_MAX_LONG_DELETION_EXTENSIONS;
    static const QString OPTION_SEED_LENGTH;
    static const QString OPTION_MAX_SEED_DIFFERENCES;
    static const QString OPTION_MAX_QUEUE_ENTRIES;
    static const QString OPTION_THREADS;
    static const QString OPTION_MISMATCH_PENALTY;
    static const QString OPTION_GAP_OPEN_PENALTY;
    static const QString OPTION_GAP_EXTENSION_PENALTY;
    static const QString OPTION_BEST_HITS;
    static const QString OPTION_QUALITY_THRESHOLD;
    static const QString OPTION_BARCODE_LENGTH;
    static const QString OPTION_COLORSPACE;
    static const QString OPTION_LONG_SCALED_GAP_PENALTY_FOR_LONG_DELETIONS;
    static const QString OPTION_NON_ITERATIVE_MODE;
private:
    BwaBuildIndexTask *buildIndexTask;
    BwaAlignTask *assembleTask;
};

class BwaTaskFactory : public DnaAssemblyToRefTaskFactory {
public:
    DnaAssemblyToReferenceTask *createTaskInstance(const DnaAssemblyToRefTaskSettings &settings, bool justBuildIndex = false);
protected:
};

} // namespace U2

#endif // _U2_BWA_TASK_H_
