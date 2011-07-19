/**
 * UGENE - Integrated Bioinformatics Tools.
 * Copyright (C) 2008-2011 UniPro <ugene@unipro.ru>
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

#ifndef _U2_CAP3_SUPPORT_TASK_H_
#define _U2_CAP3_SUPPORT_TASK_H_

#include <QtCore/QFile>

#include <U2Core/Task.h>
#include <U2Core/IOAdapter.h>
#include <U2Core/LoadDocumentTask.h>
#include <U2Core/SaveDocumentTask.h>
#include <U2Formats/StreamSequenceReader.h>
#include <U2Formats/StreamSequenceWriter.h>
#include "utils/ExportTasks.h"

#include <U2Core/MAlignmentObject.h>

#include "ExternalToolRunTask.h"

namespace U2 {

class CAP3LogParser;
class CopyDataTask;

class CAP3SupportTaskSettings {
public:
    CAP3SupportTaskSettings() 
    {
        openView = true;
        bandExpansionSize = 20;
        baseQualityDiffCutoff = 20;
        baseQualityClipCutoff = 12;
        maxQScoreSum = 200;
        maxGapLength = 20;
        gapPenaltyFactor = 6;
        maxOverhangPercent = 20;
        matchScoreFactor = 2;
        mismatchScoreFactor = -5;
        overlapSimilarityScoreCutoff = 900;
        overlapLengthCutoff = 40;
        overlapPercentIdentetyCutoff = 80;
        maxNumberOfWordMatches = 300;
        clippingRange = 100;
        reverseReads = 1;
    }
    
    QStringList getArgumentsList();

    QStringList inputFiles;
    QString outputFilePath;
    bool openView;
    int bandExpansionSize;
    int baseQualityDiffCutoff;
    int baseQualityClipCutoff;
    int maxQScoreSum;
    int maxGapLength;
    int gapPenaltyFactor;
    int maxOverhangPercent;
    int matchScoreFactor;
    int mismatchScoreFactor;
    int overlapLengthCutoff;
    int overlapPercentIdentetyCutoff;
    int overlapSimilarityScoreCutoff;
    int maxNumberOfWordMatches;
    int clippingRange;
    int reverseReads;

};

class PrepareInputForCAP3Task : public Task {
    Q_OBJECT
public:
    PrepareInputForCAP3Task(const QStringList& inputFiles, const QString& outputDirPath);
    void prepare();
    void run();
    bool onlyCopyInputFiles() { return onlyCopyFiles; }
    const QString& getPreparedPath() { return preparedPath; }
private:
    QList<CopyDataTask*> copyTasks;
    QStringList inputUrls;
    QStringList filesToCopy;
    StreamSequenceReader seqReader;
    StreamShortReadWriter seqWriter;
    QString outputDir, preparedPath;
    bool onlyCopyFiles;
};


class CAP3SupportTask : public Task {
    Q_OBJECT
public:
    CAP3SupportTask(const CAP3SupportTaskSettings& settings);
    void prepare();
    MAlignmentObject* getResultAlignment() { return maObject; }
    Task::ReportResult report();
    QList<Task*> onSubTaskFinished(Task* subTask);
private:
    MAlignmentObject*           maObject;
    QString                     tmpDirUrl, tmpOutputUrl;
    Document*                   newDoc;
    LoadDocumentTask*           loadTmpDocumentTask;
    PrepareInputForCAP3Task*    prepareDataForCAP3Task;
    ExternalToolRunTask*        cap3Task;
    CopyDataTask*               copyResultTask;
    CAP3LogParser*              logParser;
    CAP3SupportTaskSettings     settings;
};

class CAP3LogParser : public ExternalToolLogParser {
public:
    CAP3LogParser();
    int getProgress();
//private:
//    int countSequencesInMSA;
};

}//namespace
#endif // _U2_CAP3_SUPPORT_TASK_H_
