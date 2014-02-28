/**
 * UGENE - Integrated Bioinformatics Tools.
 * Copyright (C) 2008-2014 UniPro <ugene@unipro.ru>
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
#include <U2View/MSAEditorConsensusCache.h>
#include <U2View/MSAColorScheme.h>

#include <U2Algorithm/MSAConsensusAlgorithmStrict.h>
#include <U2Algorithm/MSAConsensusAlgorithmClustal.h>
#include <U2Algorithm/MSAConsensusAlgorithmRegistry.h>

#include <QtGui/QPolygonF>


namespace U2 {

void MSAGraphCalculationTask::run() {
    emit si_calculationStarted();
    constructPolygon(result);
    emit si_calculationStoped();
}

void MSAGraphCalculationTask::constructPolygon(QPolygonF &polygon) {
    SAFE_POINT_EXT(width != 0, setError(tr("Overview width is zero")), );

    if (msaLength == 0) {
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
        }
    }

    points.append(QPointF(width, height));
    polygon = QPolygonF(points);
}

int MSAConsensusOverviewCalculationTask::getGraphValue(int pos) const {
    return consensus->getConsensusCharPercent(pos);
}

MSAGapOverviewCalculationTask::MSAGapOverviewCalculationTask(MAlignmentObject* msa,
                              int msaLen,
                              int width, int height)
    : MSAGraphCalculationTask(msaLen, width, height),
      msa(msa)
{
    SAFE_POINT_EXT(msa != NULL, setError(tr("MSA is NULL")), );
    seqNumber = msa->getNumRows();
    SAFE_POINT_EXT(seqNumber != 0, setError(tr("No sequences in MSA")), );
}

int MSAGapOverviewCalculationTask::getGraphValue(int pos) const {
    SAFE_POINT(msa != NULL, tr("MAlignmentObject is NULL"), 0);

    int gapCounter = 0;
    for (int seq = 0; seq < seqNumber; seq++) {
        if (pos > msa->getLength()) {
            continue;
        }
        uchar c = static_cast<uchar>(msa->charAt(seq, pos));
        if (c == MAlignment_GapChar) {
            gapCounter++;
        }
    }

    return gapCounter * 100 / seqNumber;
}

MSAClustalOverviewCalculationTask::MSAClustalOverviewCalculationTask(MAlignmentObject *_msa,
                                                                     int msaLen,
                                                                     int width, int height)
    : MSAGraphCalculationTask(msaLen, width, height),
      msa(_msa)
{

    SAFE_POINT_EXT(msa != NULL, setError(tr("MAlignmentObject is NULL")), );
    SAFE_POINT_EXT(AppContext::getMSAConsensusAlgorithmRegistry() != NULL, setError(tr("MSAConsensusAlgorithmRegistry is NULL!")), );

    MSAConsensusAlgorithmFactory* factory = AppContext::getMSAConsensusAlgorithmRegistry()->getAlgorithmFactory(BuiltInConsensusAlgorithms::CLUSTAL_ALGO);
    SAFE_POINT_EXT(factory != NULL, setError(tr("Clustal algorithm factory is NULL")), );

    const MAlignment& ma = msa->getMAlignment();
    algorithm = factory->createAlgorithm(ma);
    algorithm->setParent(this);
}

int MSAClustalOverviewCalculationTask::getGraphValue(int pos) const {
    const MAlignment& ma = msa->getMAlignment();
    char c = algorithm->getConsensusChar(ma, pos);

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
                                                                               int msaLen,
                                                                               int width, int height)
    : MSAGraphCalculationTask(msaLen, width, height)
{
    msa = editor->getMSAObject();
    msaRowNumber = msa->getNumRows();

    SAFE_POINT_EXT(AppContext::getMSAHighlightingSchemeRegistry() != NULL,
                   setError(tr("MSA highlighting scheme registry is NULL")), );
    MSAHighlightingSchemeFactory* f_hs = AppContext::getMSAHighlightingSchemeRegistry()->getMSAHighlightingSchemeFactoryById( highlightingSchemeId );
    SAFE_POINT_EXT(f_hs != NULL, setError(tr("MSA highlighting scheme factory with '%1' id is NULL").arg(highlightingSchemeId)), );

    highlightingScheme = f_hs->create(this, msa);
    schemeId = f_hs->getId();

    MSAColorSchemeFactory* f_cs = AppContext::getMSAColorSchemeRegistry()->getMSAColorSchemeFactoryById( colorSchemeId );
    colorScheme = f_cs->create(this, msa);

    U2OpStatusImpl os;
    const MAlignment& ma = msa->getMAlignment();
    refSequenceId = ma.getRowIndexByRowId( editor->getReferenceRowId(), os);
}

bool MSAHighlightingOverviewCalculationTask::isCellHighlighted(MAlignmentObject *msa, MSAHighlightingScheme *highlightingScheme,
                                                               MSAColorScheme *colorScheme,
                                                               int seq, int pos,
                                                               int refSeq)
{
    SAFE_POINT(msa != NULL, tr("MAlignmentObject is NULL"), false);
    SAFE_POINT(colorScheme != NULL, tr("Color scheme is NULL"), false);
    SAFE_POINT(highlightingScheme != NULL, tr("Highlighting scheme is NULL"), false);
    SAFE_POINT(highlightingScheme->getFactory() != NULL, tr("Highlighting scheme factory is NULL"), false);
    QString schemeId = highlightingScheme->getFactory()->getId();

    if (seq == refSeq || isEmptyScheme(schemeId) ||
            ((refSeq == MAlignmentRow::invalidRowId()) && !isGapScheme(schemeId))) {
        if (colorScheme->getColor(seq, pos) != QColor()) {
            return true;
        }
    }
    else  {
        char refChar;
        if (isGapScheme(schemeId)) {
            refChar = 'z';
        } else {
            refChar = msa->charAt(refSeq, pos);
        }
        bool drawColor;
        char c = msa->charAt(seq, pos);

        highlightingScheme->process(refChar, c, drawColor);
        if (drawColor) {
            return true;
        }
    }

    return false;

}

int MSAHighlightingOverviewCalculationTask::getGraphValue(int pos) const {
    CHECK(msaRowNumber != 0, 0);

    int counter = 0;
    for (int i = 0; i < msaRowNumber; i++) {
        if ( isCellHighlighted(i, pos) ) {
            counter++;
        }
    }

    return 100 * counter / msaRowNumber;
}

bool MSAHighlightingOverviewCalculationTask::isGapScheme(const QString &schemeId) {
    return (schemeId == MSAHighlightingScheme::GAPS_AMINO || schemeId == MSAHighlightingScheme::GAPS_NUCL);
}

bool MSAHighlightingOverviewCalculationTask::isEmptyScheme(const QString &schemeId) {
    return (schemeId == MSAHighlightingScheme::EMPTY_AMINO || schemeId == MSAHighlightingScheme::EMPTY_NUCL);
}

bool MSAHighlightingOverviewCalculationTask::isCellHighlighted(int seq, int pos) const {
    return isCellHighlighted(msa, highlightingScheme, colorScheme, seq, pos, refSequenceId);
}

} // namespace
