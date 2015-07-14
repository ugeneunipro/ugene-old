/**
 * UGENE - Integrated Bioinformatics Tools.
 * Copyright (C) 2008-2015 UniPro <ugene@unipro.ru>
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

#ifndef _U2_MSA_GRAPH_CALCULATION_TASK_H_
#define _U2_MSA_GRAPH_CALCULATION_TASK_H_

#include <U2Core/AppResources.h>
#include <U2Core/global.h>
#include <U2Core/BackgroundTaskRunner.h>
#include <U2View/MSAEditorConsensusCache.h>

#include <U2Core/MAlignment.h>

#include <QtGui/QPolygonF>

namespace U2 {

class MSAEditor;
class MAlignmentObject;
class MSAConsensusAlgorithm;
class MSAColorScheme;
class MSAHighlightingScheme;

class MSAGraphCalculationTask : public BackgroundTask<QPolygonF> {
    Q_OBJECT
public:
    MSAGraphCalculationTask(MAlignmentObject* msa, int msaLength, int width, int height);

    void run();
signals:
    void si_calculationStarted();
    void si_calculationStoped();
protected:
    void constructPolygon(QPolygonF &polygon);
    virtual int getGraphValue(int) const { return height; }

    QScopedPointer<MAlignment> ma;
    MemoryLocker memLocker;
    int msaLength;
    int width;
    int height;
};

class MSAConsensusOverviewCalculationTask : public MSAGraphCalculationTask {
    Q_OBJECT
public:
    MSAConsensusOverviewCalculationTask(MAlignmentObject* msa,
                                        int msaLen,
                                        int width, int height);
private:
    int getGraphValue(int pos) const;

    MSAConsensusAlgorithm*  algorithm;
    int seqNumber;
};

class MSAGapOverviewCalculationTask : public MSAGraphCalculationTask {
    Q_OBJECT
public:
    MSAGapOverviewCalculationTask(MAlignmentObject* msa,
                                  int msaLen,
                                  int width, int height);
private:
    int getGraphValue(int pos) const;

    int seqNumber;
};

class MSAClustalOverviewCalculationTask : public MSAGraphCalculationTask {
    Q_OBJECT
public:
    MSAClustalOverviewCalculationTask(MAlignmentObject* msa,
                                      int msaLen,
                                      int width, int height);
private:
    int getGraphValue(int pos) const;

    MSAConsensusAlgorithm*  algorithm;
};

class MSAHighlightingOverviewCalculationTask : public MSAGraphCalculationTask {
    Q_OBJECT
public:
    MSAHighlightingOverviewCalculationTask(MSAEditor* _editor,
                                           const QString &colorSchemeId,
                                           const QString &highlightingSchemeId,
                                           int msaLen,
                                           int width, int height);

    static bool isCellHighlighted(const MAlignment &msa,
                                  MSAHighlightingScheme* highlightingScheme,
                                  MSAColorScheme* colorScheme,
                                  int seq, int pos,
                                  int refSeq);

    static bool isGapScheme(const QString &schemeId);
    static bool isEmptyScheme(const QString &schemeId);

private:
    int getGraphValue(int pos) const;

    bool isCellHighlighted(int seq, int pos) const;

    int msaRowNumber;
    int refSequenceId;

    MSAColorScheme*         colorScheme;
    MSAHighlightingScheme*  highlightingScheme;
    QString                 schemeId;
};

} // namespace

#endif // _U2_MSA_GRAPH_CALCULATION_TASK_H_
