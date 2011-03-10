#ifndef _U2_MAFFT_SUPPORT_TASK_H
#define _U2_MAFFT_SUPPORT_TASK_H

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
class MAFFTLogParser;
class MAFFTSupportTaskSettings {
public:
    MAFFTSupportTaskSettings() {reset();}
    void reset();

    float   gapOpenPenalty;
    float   gapExtenstionPenalty;
    int     maxNumberIterRefinement;
    QString inputFilePath;
};


class MAFFTSupportTask : public Task {
    Q_OBJECT
public:
    MAFFTSupportTask(MAlignmentObject* _mAObject, const MAFFTSupportTaskSettings& settings);
    void prepare();
    Task::ReportResult report();

    QList<Task*> onSubTaskFinished(Task* subTask);

    MAlignment                  resultMA;
private:
    MAlignmentObject*           mAObject;
    Document*                   currentDocument;
    Document*                   newDocument;
    QString                     url;
    MAFFTLogParser*             logParser;

    SaveMSA2SequencesTask*      saveTemporaryDocumentTask;
    ExternalToolRunTask*        mAFFTTask;
    LoadDocumentTask*           loadTemporyDocumentTask;
    MAFFTSupportTaskSettings    settings;
};

class MAFFTWithExtFileSpecifySupportTask : public Task {
    Q_OBJECT
public:
    MAFFTWithExtFileSpecifySupportTask(const MAFFTSupportTaskSettings& settings);
    void prepare();
    Task::ReportResult report();

    QList<Task*> onSubTaskFinished(Task* subTask);
private:
    MAlignmentObject*           mAObject;
    Document*                   currentDocument;

    SaveDocumentTask*           saveDocumentTask;
    LoadDocumentTask*           loadDocumentTask;
    MAFFTSupportTask*           mAFFTSupportTask;
    MAFFTSupportTaskSettings    settings;
};

class MAFFTLogParser : public ExternalToolLogParser {
public:
    MAFFTLogParser(int countSequencesInMSA, int countRefinementIter, const QString& outputFileName);
    ~MAFFTLogParser(){ outFile.close(); }
    int getProgress();
    void parseOutput(const QString& partOfLog);
    void parseErrOutput(const QString& partOfLog);

    bool isOutFileCreated(){ return isOutputFileCreated; }
private:
    int     countSequencesInMSA;
    int     countRefinementIter;
    QString outputFileName;
    QFile   outFile;
    bool    isOutputFileCreated;
    QString lastErrLine;

    bool    firstDistanceMatrix;
    bool    firstUPGMATree;
    bool    firstProAlign;
    int     progress;
};

}//namespace
#endif // _U2_MAFFT_SUPPORT_TASK_H
