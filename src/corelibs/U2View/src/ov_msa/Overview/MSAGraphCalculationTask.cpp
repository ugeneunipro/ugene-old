/**
 * UGENE - Integrated Bioinformatics Tools.
 * Copyright (C) 2008-2016 UniPro <ugene@unipro.ru>
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

#include "MSAGraphCalculationTask.h"

#include <U2Core/U2SafePoints.h>
#include <U2Core/MAlignmentObject.h>
#include <U2Core/U2OpStatusUtils.h>

#include <U2View/MSAEditor.h>
#include <U2Algorithm/MSAColorScheme.h>

#include <U2Algorithm/MSAConsensusAlgorithmStrict.h>
#include <U2Algorithm/MSAConsensusAlgorithmClustal.h>
#include <U2Algorithm/MSAConsensusAlgorithmRegistry.h>

#include <QtGui/QPolygonF>


namespace U2 {

MSAGraphCalculationTask::MSAGraphCalculationTask(MAlignmentObject* msa, int width, int height)
    : BackgroundTask<QPolygonF>(tr("Render overview"), TaskFlag_None),
      memLocker(stateInfo),
      msaLength(0),
      seqNumber(0),
      width(width),
      height(height)
{
    SAFE_POINT_EXT(msa != NULL, setError(tr("MSA is NULL")), );
    msaLength = msa->getLength();
    seqNumber = msa->getNumRows();
    if(!memLocker.tryAcquire(msa->getMAlignment().getLength() * msa->getMAlignment().getNumRows())) {
        setError(memLocker.getError());
        return;
    }
    ma.reset(new MAlignment(msa->getMAlignment()));
    connect(msa, SIGNAL(si_invalidateAlignmentObject()), this, SLOT(cancel()));
    connect(msa, SIGNAL(si_startMsaUpdating()), this, SLOT(cancel()));
    connect(msa, SIGNAL(si_alignmentChanged(MAlignment,MAlignmentModInfo)), this, SLOT(cancel()));
}

void MSAGraphCalculationTask::run() {
    CHECK(!hasError(), );
    emit si_calculationStarted();
    constructPolygon(result);
    emit si_calculationStoped();
}

void MSAGraphCalculationTask::constructPolygon(QPolygonF &polygon) {
    SAFE_POINT_EXT(width != 0, setError(tr("Overview width is zero")), );
    stateInfo.setProgress(0);
    emit si_progressChanged();

    if (msaLength == 0 || seqNumber == 0) {
        polygon = QPolygonF();
        return;
    }

    double stepY = height / static_cast<double>(100);
    QVector<QPointF> points;
    points.append(QPointF(0, height));

    if ( msaLength < width ) {
        double stepX = width / static_cast<double>(msaLength);
        points.append(QPointF(0, qRound( height - stepY * static_cast<double>(getGraphValue(0)))));
        for (int pos = 0; pos < msaLength; pos++) {
            if (isCanceled()) {
                polygon = QPolygonF();
                return;
            }
            int percent = getGraphValue(pos);
            points.append(QPointF(qRound( stepX * static_cast<double>(pos) + stepX / 2),
                                  height - stepY * percent));
            stateInfo.setProgress(100 * pos / msaLength);
            emit si_progressChanged();
        }
        points.append(QPointF( width, qRound( height - stepY * static_cast<double>(getGraphValue(msaLength - 1)))));

    } else {
        double stepX = msaLength / static_cast<double>(width);
        for (int pos = 0; pos < width; pos++) {
            double average = 0;
            int count = 0;
            for (int i = stepX * pos; i < qRound( stepX * (pos + 1) ); i++) {
                if (isCanceled()) {
                    polygon = QPolygonF();
                    return;
                }
                if (i > msaLength) {
                    break;
                }
                average += getGraphValue(i);
                count++;
            }
            CHECK(count != 0, );
            average /= count;
            points.append( QPointF(pos, height - stepY * average ));
            stateInfo.setProgress(100 * pos / width);
            emit si_progressChanged();
        }
    }

    points.append(QPointF(width, height));
    polygon = QPolygonF(points);
    stateInfo.setProgress(100);
    emit si_progressChanged();
}

MSAConsensusOverviewCalculationTask::MSAConsensusOverviewCalculationTask(MAlignmentObject* msa,
                                    int width, int height)
    : MSAGraphCalculationTask(msa, width, height)
{
    SAFE_POINT_EXT(AppContext::getMSAConsensusAlgorithmRegistry() != NULL, setError(tr("MSAConsensusAlgorithmRegistry is NULL!")), );

    MSAConsensusAlgorithmFactory* factory = AppContext::getMSAConsensusAlgorithmRegistry()->getAlgorithmFactory(BuiltInConsensusAlgorithms::STRICT_ALGO);
    SAFE_POINT_EXT(factory != NULL, setError(tr("Strict consensus algorithm factory is NULL")), );

    SAFE_POINT_EXT(msa != NULL, setError(tr("MSA is NULL")), );
    const MAlignment& ma = msa->getMAlignment();
    algorithm = factory->createAlgorithm(ma);
    algorithm->setParent(this);
}

int MSAConsensusOverviewCalculationTask::getGraphValue(int pos) const {
    int score;
    algorithm->getConsensusCharAndScore(*ma, pos, score);
    return qRound(score * 100. / seqNumber);
}

MSAGapOverviewCalculationTask::MSAGapOverviewCalculationTask(MAlignmentObject* msa, int width, int height)
    : MSAGraphCalculationTask(msa, width, height) {}

int MSAGapOverviewCalculationTask::getGraphValue(int pos) const {
    int gapCounter = 0;
    for (int seq = 0; seq < seqNumber; seq++) {
        if (pos > ma->getLength()) {
            continue;
        }
        uchar c = static_cast<uchar>(ma->charAt(seq, pos));
        if (c == MAlignment_GapChar) {
            gapCounter++;
        }
    }

    return qRound(gapCounter * 100. / seqNumber);
}

MSAClustalOverviewCalculationTask::MSAClustalOverviewCalculationTask(MAlignmentObject *msa, int width, int height)
    : MSAGraphCalculationTask(msa, width, height) {
    SAFE_POINT_EXT(AppContext::getMSAConsensusAlgorithmRegistry() != NULL, setError(tr("MSAConsensusAlgorithmRegistry is NULL!")), );

    MSAConsensusAlgorithmFactory* factory = AppContext::getMSAConsensusAlgorithmRegistry()->getAlgorithmFactory(BuiltInConsensusAlgorithms::CLUSTAL_ALGO);
    SAFE_POINT_EXT(factory != NULL, setError(tr("Clustal algorithm factory is NULL")), );

    SAFE_POINT_EXT(msa != NULL, setError(tr("MSA is NULL")), );
    algorithm = factory->createAlgorithm(*ma);
    algorithm->setParent(this);
}

int MSAClustalOverviewCalculationTask::getGraphValue(int pos) const {
    char c = algorithm->getConsensusChar(*ma, pos);

    switch (c) {
    case '*':
        return 100;
    case ':':
        return 60;
    case '.':
        return 30;
    default:
        return 0;
    }
}

MSAHighlightingOverviewCalculationTask::MSAHighlightingOverviewCalculationTask(MSAEditor *editor,
                                                                               const QString &colorSchemeId,
                                                                               const QString &highlightingSchemeId,
                                                                               int width, int height)
    : MSAGraphCalculationTask(editor->getMSAObject(), width, height) {

    SAFE_POINT_EXT(AppContext::getMSAHighlightingSchemeRegistry() != NULL,
                   setError(tr("MSA highlighting scheme registry is NULL")), );
    MSAHighlightingSchemeFactory* f_hs = AppContext::getMSAHighlightingSchemeRegistry()->getMSAHighlightingSchemeFactoryById( highlightingSchemeId );
    SAFE_POINT_EXT(f_hs != NULL, setError(tr("MSA highlighting scheme factory with '%1' id is NULL").arg(highlightingSchemeId)), );

    highlightingScheme = f_hs->create(this, editor->getMSAObject());
    schemeId = f_hs->getId();

    MSAColorSchemeFactory* f_cs = AppContext::getMSAColorSchemeRegistry()->getMSAColorSchemeFactoryById( colorSchemeId );
    colorScheme = f_cs->create(this, editor->getMSAObject());

    U2OpStatusImpl os;
    refSequenceId = ma->getRowIndexByRowId(editor->getReferenceRowId(), os);
}

bool MSAHighlightingOverviewCalculationTask::isCellHighlighted(const MAlignment &ma, MSAHighlightingScheme *highlightingScheme,
                                                               MSAColorScheme *colorScheme,
                                                               int seq, int pos,
                                                               int refSeq)
{
    SAFE_POINT(colorScheme != NULL, tr("Color scheme is NULL"), false);
    SAFE_POINT(highlightingScheme != NULL, tr("Highlighting scheme is NULL"), false);
    SAFE_POINT(highlightingScheme->getFactory() != NULL, tr("Highlighting scheme factory is NULL"), false);
    QString schemeId = highlightingScheme->getFactory()->getId();

    if (seq == refSeq || isEmptyScheme(schemeId) ||
            ((refSeq == MAlignmentRow::invalidRowId()) && !isGapScheme(schemeId) &&
            !highlightingScheme->getFactory()->isRefFree())) {
        if (colorScheme->getColor(seq, pos, ma.charAt(seq, pos)) != QColor()) {
            return true;
        }
    }
    else  {
        char refChar;
        if (isGapScheme(schemeId) || highlightingScheme->getFactory()->isRefFree()) {
            refChar = 'z';
        } else {
            refChar = ma.charAt(refSeq, pos);
        }
        bool drawColor;
        char c = ma.charAt(seq, pos);

        highlightingScheme->process(refChar, c, drawColor, pos, seq);
        if (drawColor) {
            return true;
        }
    }

    return false;

}

int MSAHighlightingOverviewCalculationTask::getGraphValue(int pos) const {
    CHECK(seqNumber != 0, 0);

    int counter = 0;
    for (int i = 0; i < seqNumber; i++) {
        if ( isCellHighlighted(i, pos) ) {
            counter++;
        }
    }

    return 100 * counter / seqNumber;
}

bool MSAHighlightingOverviewCalculationTask::isGapScheme(const QString &schemeId) {
    return (schemeId == MSAHighlightingScheme::GAPS_AMINO || schemeId == MSAHighlightingScheme::GAPS_NUCL);
}

bool MSAHighlightingOverviewCalculationTask::isEmptyScheme(const QString &schemeId) {
    return (schemeId == MSAHighlightingScheme::EMPTY_AMINO || schemeId == MSAHighlightingScheme::EMPTY_NUCL);
}

bool MSAHighlightingOverviewCalculationTask::isCellHighlighted(int seq, int pos) const {
    return isCellHighlighted(*ma, highlightingScheme, colorScheme, seq, pos, refSequenceId);
}

} // namespace
