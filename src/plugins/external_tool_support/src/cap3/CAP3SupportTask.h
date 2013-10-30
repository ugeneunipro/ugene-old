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

#ifndef _U2_CAP3_SUPPORT_TASK_H_
#define _U2_CAP3_SUPPORT_TASK_H_

#include <QtCore/QFile>

#include <U2Formats/StreamSequenceReader.h>
#include <U2Formats/StreamSequenceWriter.h>

#include "utils/ExportTasks.h"

#include <U2Core/ExternalToolRunTask.h>
#include <U2Core/MAlignmentObject.h>

namespace U2 {

class CAP3LogParser;
class CopyDataTask;
class LoadDocumentTask;

class CAP3SupportTaskSettings {
public:
    CAP3SupportTaskSettings() 
    {
        openView = true;
        bandExpansionSize = defaultBandExpansionSize;
        baseQualityDiffCutoff = defaultBaseQualityDiffCutoff;
        baseQualityClipCutoff = defaultBaseQualityClipCutoff;
        maxQScoreSum = defaultMaxQScoreSum;
        maxGapLength = defaultMaxGapLength;
        gapPenaltyFactor = defaultGapPenaltyFactor;
        maxOverhangPercent = defaultMaxOverhangPercent;
        matchScoreFactor = defaultMatchScoreFactor;
        mismatchScoreFactor = defaultMismatchScoreFactor;
        overlapSimilarityScoreCutoff = defaultOverlapSimilarityScoreCutoff;
        overlapLengthCutoff = defaultOverlapLengthCutoff;
        overlapPercentIdentityCutoff = defaultOverlapPercentIdentityCutoff;
        maxNumberOfWordMatches = defaultMaxNumberOfWordMatches;
        clippingRange = defaultClippingRange;
        reverseReads = defaultReverseReads;
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
    int overlapPercentIdentityCutoff;
    int overlapSimilarityScoreCutoff;
    int maxNumberOfWordMatches;
    int clippingRange;
    bool reverseReads;

    static const int defaultBandExpansionSize = 20;
    static const int defaultBaseQualityDiffCutoff = 20;
    static const int defaultBaseQualityClipCutoff = 12;
    static const int defaultMaxQScoreSum = 200;
    static const int defaultMaxGapLength = 20;
    static const int defaultGapPenaltyFactor = 6;
    static const int defaultMaxOverhangPercent = 20;
    static const int defaultMatchScoreFactor = 2;
    static const int defaultMismatchScoreFactor = -5;
    static const int defaultOverlapSimilarityScoreCutoff = 900;
    static const int defaultOverlapLengthCutoff = 40;
    static const int defaultOverlapPercentIdentityCutoff = 90;
    static const int defaultMaxNumberOfWordMatches = 300;
    static const int defaultClippingRange = 100;
    static const bool defaultReverseReads = true;
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
    QString outputDir, preparedPath, qualityFilePath;
    bool onlyCopyFiles;
};


class CAP3SupportTask : public ExternalToolSupportTask {
    Q_OBJECT
public:
    CAP3SupportTask(const CAP3SupportTaskSettings& settings);
    void prepare();
    MAlignmentObject* getResultAlignment() { return maObject; }

    /**
     * Returns output file URL if the file has already been produced
     * Otherwise, returns an empty string
     */
    QString getOutputFile() const;

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
    QString                     outputFile;
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
