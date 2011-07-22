#ifndef _U2_BOWTIE_TASK_H_
#define _U2_BOWTIE_TASK_H_

#include <U2Algorithm/DnaAssemblyTask.h>

#include "ExternalToolRunTask.h"

namespace U2 {

class BowtieBuildIndexTask : public Task {
    Q_OBJECT
public:
    BowtieBuildIndexTask(const QString &referencePath, const QString &indexPath);

    void prepare();
private:
    class LogParser : public ExternalToolLogParser {
    public:
        enum Stage {
            PREPARE,
            FORWARD_INDEX,
            MIRROR_INDEX
        };
        enum Substage {
            UNKNOWN,
            BUCKET_SORT,
            GET_BLOCKS
        };

        LogParser();
        void parseOutput(const QString &partOfLog);
        void parseErrOutput(const QString &partOfLog);
        int getProgress();
    private:
        Stage stage;
        Substage substage;
        int bucketSortIteration;
        int blockIndex;
        int blockCount;
        int substageProgress;
        int progress;
    };

    LogParser logParser;
    QString referencePath;
    QString indexPath;
};

class BowtieAssembleTask : public Task {
    Q_OBJECT
public:
    BowtieAssembleTask(const DnaAssemblyToRefTaskSettings &settings);

    void prepare();
private:
    ExternalToolLogParser logParser;
    DnaAssemblyToRefTaskSettings settings;
};

class BowtieTask : public DnaAssemblyToReferenceTask {
    Q_OBJECT
    DNA_ASSEMBLEY_TO_REF_TASK_FACTORY(BowtieTask)
public:
    BowtieTask(const DnaAssemblyToRefTaskSettings &settings, bool justBuildIndex = false);

    void prepare();
protected slots:
    QList<Task *> onSubTaskFinished(Task *subTask);
public:
    static const QString OPTION_N_MISMATCHES;
    static const QString OPTION_V_MISMATCHES;
    static const QString OPTION_MAQERR;
    static const QString OPTION_SEED_LEN;
    static const QString OPTION_NOFW;
    static const QString OPTION_NORC;
    static const QString OPTION_MAXBTS;
    static const QString OPTION_TRYHARD;
    static const QString OPTION_CHUNKMBS;
    static const QString OPTION_NOMAQROUND;
    static const QString OPTION_SEED;
    static const QString OPTION_BEST;
    static const QString OPTION_ALL;
private:
    BowtieBuildIndexTask *buildIndexTask;
    BowtieAssembleTask *assembleTask;
};

class BowtieTaskFactory : public DnaAssemblyToRefTaskFactory {
public:
    DnaAssemblyToReferenceTask *createTaskInstance(const DnaAssemblyToRefTaskSettings &settings, bool justBuildIndex = false);
protected:
};

} // namespace U2

#endif // _U2_BOWTIE_TASK_H_
