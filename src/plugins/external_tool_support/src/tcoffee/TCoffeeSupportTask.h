#ifndef _U2_TCOFFEE_SUPPORT_TASK_H
#define _U2_TCOFFEE_SUPPORT_TASK_H

#include <U2Core/Task.h>
#include <U2Core/IOAdapter.h>

#include <U2Core/LoadDocumentTask.h>
#include <U2Core/SaveDocumentTask.h>
#include "utils/ExportTasks.h"

#include <U2Core/MAlignmentObject.h>

#include "ExternalToolRunTask.h"

namespace U2 {

/*Options for MAFFT
+ --op # :         Gap opening penalty, default: 1.53
+ --ep # :         Offset (works like gap extension penalty), default: 0.0
+ --maxiterate # : Maximum number of iterative refinement, default: 0
? --clustalout :   Output: clustal format, default: fasta // With this option bad output clustal file is generated.(dkandrov)
? --reorder :      Outorder: aligned, default: input order
? --quiet :        Do not report progress
? --thread # :     Number of threads. (# must be <= number of physical cores - 1)

*/
class TCoffeeLogParser;
class TCoffeeSupportTaskSettings {
public:
    TCoffeeSupportTaskSettings() {reset();}
    void reset();

    float   gapOpenPenalty;
    float   gapExtenstionPenalty;
    int     numIterations;
    QString inputFilePath;
};


class TCoffeeSupportTask : public Task {
    Q_OBJECT
public:
    TCoffeeSupportTask(MAlignmentObject* _mAObject, const TCoffeeSupportTaskSettings& settings);
    void prepare();
    Task::ReportResult report();

    QList<Task*> onSubTaskFinished(Task* subTask);

    MAlignment                  resultMA;
private:
    MAlignmentObject*           mAObject;
    Document*                   currentDocument;
    Document*                   newDocument;
    QString                     url;
    TCoffeeLogParser*           logParser;

    SaveMSA2SequencesTask*      saveTemporaryDocumentTask;
    ExternalToolRunTask*        tCoffeeTask;
    LoadDocumentTask*           loadTemporyDocumentTask;
    TCoffeeSupportTaskSettings  settings;
};

class TCoffeeWithExtFileSpecifySupportTask : public Task {
    Q_OBJECT
public:
    TCoffeeWithExtFileSpecifySupportTask(const TCoffeeSupportTaskSettings& settings);
    void prepare();
    Task::ReportResult report();

    QList<Task*> onSubTaskFinished(Task* subTask);
private:
    MAlignmentObject*           mAObject;
    Document*                   currentDocument;

    SaveDocumentTask*           saveDocumentTask;
    LoadDocumentTask*           loadDocumentTask;
    TCoffeeSupportTask*           tCoffeeSupportTask;
    TCoffeeSupportTaskSettings    settings;
};

class TCoffeeLogParser : public ExternalToolLogParser {
public:
    TCoffeeLogParser();

    int getProgress();
    void parseOutput(const QString& partOfLog);
    void parseErrOutput(const QString& partOfLog);

private:
    QString lastErrLine;
    int     progress;
};

}//namespace
#endif // _U2_TCOFFEE_SUPPORT_TASK_H
