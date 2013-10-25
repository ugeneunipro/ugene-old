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

#include "ADVGraphModel.h"
#include "GSequenceGraphView.h"
#include "GraphSettingsDialog.h"
#include "WindowStepSelectorWidget.h"
#include "SaveGraphCutoffsDialogController.h"

#include <math.h>

namespace U2 {

const QByteArray& GSequenceGraphAlgorithm::getSequenceData(U2SequenceObject* seqObj) {
    if(seqObj != lastSeqObj) {
        lastSeqObj = seqObj;
        lastSeqData = seqObj->getWholeSequenceData();
    }
    return lastSeqData;
}

GSequenceGraphData::GSequenceGraphData(const QString& _graphName) : graphName(_graphName), ga(NULL)
{
    cachedFrom = cachedLen = cachedW = cachedS = 0;;
}

GSequenceGraphData::~GSequenceGraphData() {
    delete ga;
}

void GSequenceGraphUtils::calculateMinMax(const QVector<float>& data, float& min, float& max)  {
    assert(data.size() > 0);
    min = max = data.first();
    const float* d = data.constData();
    for (int i=1, n = data.size() ; i<n ; i++) {
        float val = d[i];
        if (min > val) {
            min = val;
        } else if (max < val) {
            max = val;
        }
    }
}

#define ACCEPTABLE_FLOAT_PRESISION_LOSS 0.0001
float GSequenceGraphUtils::calculateAverage(const QVector<float>& data, float start, float range) {
    float result;
    if (int(start)!=int(start+range)) {
        //result constructed from 3 parts: ave[start, startIdx] + ave[startIdx, endIdx] + ave[endIdx, end]
        float part1 = 0;
        float part2 = 0;
        float part3 = 0;

        int startIdx = int(floor(start));
        float startDiff = 1 - (start - startIdx);
        float end = start + range;
        int endIdx = int(end);
        float endDiff = end - endIdx;

        assert(qAbs(startDiff + (endIdx-(startIdx+1)) + endDiff - range) / range <= ACCEPTABLE_FLOAT_PRESISION_LOSS);

        //calculating part1
        if (startDiff > ACCEPTABLE_FLOAT_PRESISION_LOSS) {
            float v1 = data[startIdx];
            float v2 = data[startIdx+1];
            float k = v2-v1;
            float valInStart = v2 - k*startDiff;
            part1 = startDiff * (valInStart + v2) / 2;
        }
        int firstIdxInRange = int(ceil(start));
        //calculating part2
        for(int i =firstIdxInRange; i < endIdx; i++) {
            part2+=data[i];
        }
        //calculating part3
        if (endDiff > ACCEPTABLE_FLOAT_PRESISION_LOSS && endIdx+1 < (int)data.size()) {
            float v1 = data[endIdx];
            float v2 = data[endIdx+1];
            float k = v2-v1;
            float valInEnd= v1+k*endDiff;
            part3 = endDiff * (v1 + valInEnd) / 2;
        }
        //sum
        result = (part1 + part2 + part3 ) / range;
    } else {
        //result constructed from 1 part: ave[start, end], no data points between
        int startIdx = int(start);
        float startDiff = start - float(startIdx);
        float endDiff = startDiff + range;
        assert(endDiff < 1);
        float v1 = data[startIdx];
        float v2 = data[startIdx+1];
        float k = v2-v1;
        float valInStart = v1 + k*startDiff;
        float valInEnd = v1 + k*endDiff;
        result = (valInEnd+valInStart)/2;
    }
    return result;
}

void GSequenceGraphUtils::fitToScreen(const QVector<float>& data, int dataStartBase, int dataEndBase, QVector<float>& results, 
                                       int resultStartBase, int resultEndBase, int screenWidth, float unknownVal)
{
    //BUG:422: use intervals and max/min values instead of average!
    float basesPerPixel = (resultEndBase - resultStartBase) / (float) screenWidth;
    float basesInDataPerIndex = (dataEndBase - dataStartBase) / (float) (data.size() - 1);
    float currentBase = resultStartBase;
    results.reserve(results.size() + screenWidth);
    for (int i=0; i < screenWidth; i++, currentBase+=basesPerPixel) {
        float dataStartIdx = (currentBase - basesPerPixel / 2 - dataStartBase) / basesInDataPerIndex;
        float dataEndIdx =  (currentBase  + basesPerPixel / 2 - dataStartBase) / basesInDataPerIndex;
        dataStartIdx = qMax((float)0, dataStartIdx);
        dataEndIdx = qMin((float)data.size()-1, dataEndIdx);
        float nDataPointsToAverage = dataEndIdx - dataStartIdx;
        float val = unknownVal;
        if (nDataPointsToAverage >= ACCEPTABLE_FLOAT_PRESISION_LOSS) {
            val = calculateAverage(data, dataStartIdx, nDataPointsToAverage);
        }
        results.append(val);
    }
}

int GSequenceGraphUtils::getNumSteps(const U2Region& range, int w, int s) { 
    if(range.length < w) return 1;
    int steps = (range.length  - w) / s + 1;
    return steps;
}

//////////////////////////////////////////////////////////////////////////
//drawer

const QString GSequenceGraphDrawer::DEFAULT_COLOR(tr("Default color"));
const int GSequenceGraphDrawer::UNKNOWN_VAL = -1;

GSequenceGraphDrawer::GSequenceGraphDrawer(GSequenceGraphView* v, const GSequenceGraphWindowData& wd, 
                                           QMap<QString,QColor> colors)
: QObject(v), view(v), lineColors(colors), globalMin(0), globalMax(0), wdata(wd)
{
    connect(v, SIGNAL(si_frameRangeChanged(GSequenceGraphData*, const QRect&)),
            this, SLOT(sl_frameRangeChanged(GSequenceGraphData*, const QRect&)));
    connect(v, SIGNAL(si_visibleRangeChanged()),
            this, SLOT(sl_labelAdded()));
    connect(v, SIGNAL(si_labelAdded(GSequenceGraphData*, GraphLabel*, const QRect&)),
            this, SLOT(sl_labelAdded(GSequenceGraphData*, GraphLabel*, const QRect&)));
    connect(v, SIGNAL(si_labelMoved(GSequenceGraphData*, GraphLabel*, const QRect&)),
            this, SLOT(sl_labelMoved(GSequenceGraphData*, GraphLabel*, const QRect&)));
    connect(v, SIGNAL(si_labelsColorChange(GSequenceGraphData*)),
            this, SLOT(sl_labelsColorChange(GSequenceGraphData*)));
    defFont = new QFont("Arial", 8);
    if (colors.isEmpty()) {
        lineColors.insert(DEFAULT_COLOR, Qt::black);
    }
}

GSequenceGraphDrawer::~GSequenceGraphDrawer() {
    delete defFont;
}

void GSequenceGraphDrawer::draw(QPainter& p, QList<GSequenceGraphData*> graphs, const QRect& rect) {
    
    globalMin = 0;
    globalMax = 0;

    foreach (GSequenceGraphData* graph, graphs) {
        drawGraph(p, graph, rect);
    }

    {
         //draw min/max
         QPen minMaxPen(Qt::DashDotDotLine);
         minMaxPen.setWidth(1);
         p.setPen(minMaxPen);
         p.setFont(*defFont);

         //max
         p.drawLine(rect.topLeft(), rect.topRight());
         QRect maxTextRect(rect.x(), rect.y(), rect.width(), 12);
         p.drawText(maxTextRect, Qt::AlignRight, QString::number((double) globalMax, 'g', 4));

         //min
         p.drawLine(rect.bottomLeft(), rect.bottomRight());
         QRect minTextRect(rect.x(), rect.bottom()-12, rect.width(), 12);
         p.drawText(minTextRect, Qt::AlignRight, QString::number((double) globalMin, 'g', 4));
     }
}
void GSequenceGraphDrawer::sl_frameRangeChanged(GSequenceGraphData *graph, const QRect &rect) {
    foreach(GraphLabel *label, graph->graphLabels.getLabels()) {
        int res = updateStaticLabels(graph, label, rect);
		if (res == 0) {
            label->show();
		}
    }
}
void GSequenceGraphDrawer::sl_labelAdded(GSequenceGraphData *graph, GraphLabel *label, const QRect &rect) {
    updateStaticLabels(graph, label, rect);
}

void  GSequenceGraphDrawer::sl_labelMoved(GSequenceGraphData *graph, GraphLabel *label, const QRect &rect) {
    updateMovingLabels(graph, label, rect);
}

void  GSequenceGraphDrawer::sl_labelsColorChange(GSequenceGraphData *graph) {
    QColor color;

	if  (lineColors.contains(graph->graphName)) {
        color = lineColors.value(graph->graphName);
	} else {
        color = lineColors.value(DEFAULT_COLOR);
	}

	foreach(GraphLabel *label, graph->graphLabels.getLabels()) {
        label->setColor(color, color);
	}

    graph->graphLabels.getMovingLabel().setColor(color, Qt::red);
}


void GSequenceGraphDrawer::drawGraph( QPainter& p, GSequenceGraphData* d, const QRect& rect ) {
    float min=0;
    float max=0;
    PairVector points;
    int nPoints = rect.width();
    calculatePoints(d, points, min, max, nPoints);

    assert(points.firstPoints.size() == nPoints);

    double comin = commdata.min, comax = commdata.max;
    if (commdata.enableCuttoff){
        min = comin;
        max = comax;
    }

    globalMin = min;
    globalMax = max;

    QPen graphPen(Qt::SolidLine);
    if (lineColors.contains(d->graphName)) {
        graphPen.setColor(lineColors.value(d->graphName));
    } else {
        graphPen.setColor(lineColors.value(DEFAULT_COLOR));
    }

    graphPen.setWidth(1);
    p.setPen(graphPen);


    int graphHeight = rect.bottom() - rect.top() - 2;
    float kh = (min == max) ? 1 : graphHeight / (max - min);

    int prevY = -1;
    int prevX = -1;

    if (!commdata.enableCuttoff) {
        ////////cutoff off
        for (int i = 0, n = nPoints; i < n; i++) {
            float fy1 = points.firstPoints[i];
            if (isUnknownValue(fy1)) {
                continue;
            }
            int dy1 = qRound((fy1 - min) * kh);
            assert(dy1 <= graphHeight);
            int y1 = rect.bottom() - 1 - dy1;
            int x = rect.left() + i;
            assert(y1 > rect.top() && y1 < rect.bottom());
            if (prevX != -1) {
                p.drawLine(prevX, prevY , x, y1);
            }
            prevY = y1;
            prevX = x;
            if (points.useIntervals) {
                float fy2 = points.secondPoints[i];
                if (isUnknownValue(fy2)) {
                    continue;
                }
                int dy2 = qRound((fy2 - min) * kh);
                assert(dy2 <= graphHeight);
                int y2 = rect.bottom() - 1 - dy2;
                assert(y2 > rect.top() && y2 < rect.bottom());
                if (prevX != -1){
                    p.drawLine(prevX, prevY , x, y2);
                }
                prevY = y2;
                prevX = x;
            }
        }
    } else    {
        ////////cutoff on

        float fymin = comin;
        float fymax = comax;
        float fymid = (comin + comax)/2;
        float fy, fy2;
        int prevFY = -1;
        bool rp = false, lp = false;
        if(!points.useIntervals){
            for (int i=0, n = points.firstPoints.size(); i < n; i++) {
                fy = points.firstPoints[i];
                rp = false;
                lp = false;
                if (isUnknownValue(fy)) {
                    continue;
                }
                if (fy >= fymax) {
                    fy = fymax;
                    if (prevFY == int(fymid)) lp=true;
                }else if(fy < fymax && fy > fymin){
                    fy = fymid;
                    if (prevFY == int(fymax)) rp=true;
                    if (prevFY == int(fymin)) lp=true;
                }else{
                    fy = fymin;
                    if (prevFY == int(fymid)) lp=true;
                }

                int dy = qRound((fy - min) * kh);
                assert(dy <= graphHeight);
                int y = rect.bottom() - 1 - dy;
                int x = rect.left() + i;

                assert(y > rect.top() && y < rect.bottom());
                if (prevX!=-1){
                    p.drawLine(prevX, prevY , x, prevY);
                }

                if (prevY != y && prevX != -1){ // common case for cutoffs
                    p.drawLine(x, prevY , x, y);
                }
                prevY = y;
                prevX = x;
                prevFY = (int) fy;
            }
        } else {
            for (int i=0, n = points.firstPoints.size(); i < n; i++) {
                assert(points.firstPoints.size() == points.secondPoints.size());
                fy = points.firstPoints[i], fy2 = points.secondPoints[i];
                rp = false;
                lp = false;
                if (isUnknownValue(fy)) {
                    continue;
                }
                
                assert(!isUnknownValue(fy2));
                
                if (fy2 >= fymax) {
                    fy2 = fymax;
                }else if(fy2 < fymax && fy2 > fymin){
                    fy2 = fymid;
                }else{
                    fy2 = fymin;
                }

                if (fy >= fymax) {
                    fy = fymax;
                    if (prevFY == int(fymid)) lp=true;
                }else if(fy < fymax && fy > fymin){
                    fy = fymid;
                    if (prevFY == int(fymax)) rp=true;
                    if (prevFY == int(fymin)) lp=true;
                }else{
                    fy = fymin;
                    if (prevFY == int(fymid)) lp=true;
                }

                int dy = qRound((fy - min) * kh);
                assert(dy <= graphHeight);
                int y = rect.bottom() - 1 - dy;
                int x = rect.left() + i;
                if (lp) {
                    p.drawLine(prevX, prevY, x, prevY);
                    prevX = x;
                }
                if (rp) {
                    p.drawLine(prevX,prevY,prevX,y);
                    prevY = y;
                }
                assert(y > rect.top() && y < rect.bottom());
                if (prevX!=-1){
                    p.drawLine(prevX, prevY , x, y);
                }
                if(fy != fy2){
                    int dy2 = qRound((fy2 - min) * kh);
                    int y2 = rect.bottom() - 1 - dy2;
                    p.drawLine(x,y, x, y2);
                }
                prevY = y;
                prevX = x;
                prevFY = (int) fy;
            }

        }
    }
}
const int mLabelCoordY = 20;

void GSequenceGraphDrawer::selectExtremumPoints(GSequenceGraphData *graph, const QRect &graphRect, int windowSize, const U2Region &visibleRange)
{
    qint64 sequenceLength = view->getSequenceLength();
    int startPos = visibleRange.startPos;
    int endPos = visibleRange.endPos();
    int maxValue, minValue;
    int posOfMax, posOfMin;

    PairVector points;
	if(sequenceLength <= 0) {
        return;
	}
    calculatePoints(graph, points, globalMin, globalMax, sequenceLength);

    int pos = startPos;
    QVector<float> &firstPoints = points.firstPoints;
    endPos = qMin(endPos, firstPoints.size() - 1);
    while(pos < endPos) {
        maxValue = firstPoints.at(pos);
        minValue = globalMax * 2;
        posOfMax = pos;
        posOfMin = 0;
        for(int i = 0; i < windowSize; i++, pos++) {
            if (pos >= endPos)
                break;
            if (isUnknownValue(firstPoints.at(pos)))
                continue;
            if (maxValue < firstPoints.at(pos)) {
                maxValue = firstPoints.at(pos);
                posOfMax = pos;
            }
            if (minValue > firstPoints.at(pos)) {
                minValue = firstPoints.at(pos);
                posOfMin = pos;
            }
        }
        GraphLabel* maxLabel = new GraphLabel(posOfMax, view);
        maxLabel->show();
        graph->graphLabels.addLabel(maxLabel);
        updateStaticLabels(graph, maxLabel, graphRect);
        GraphLabel* minLabel = new GraphLabel(posOfMin, view);
        minLabel->show();
        graph->graphLabels.addLabel(minLabel);
        updateStaticLabels(graph, minLabel, graphRect);
    }
}
int GSequenceGraphDrawer::updateStaticLabels(GSequenceGraphData *graph, GraphLabel *label, const QRect &rect)
{
    int nPoints = rect.width();
    PairVector points;
	if (nPoints <= 0) {
        return 1;
	}
    calculatePoints(graph, points, globalMin, globalMax, nPoints);

    QColor color;
	if (lineColors.contains(graph->graphName)) {
        color = lineColors.value(graph->graphName);
	} else {
        color = lineColors.value(DEFAULT_COLOR);
	}
    label->setColor(color, color);

    qint64 sequenceLength = view->getSequenceLength();
    int position = label->getPosition() * nPoints / sequenceLength;
	if (position < 0 || position >= nPoints) {
        return 1;
	}

    int errorCode = calculateLabelData(rect, points, label);
	if (errorCode!=0) {
        return errorCode;
	}

    QRectF boundingRect = label->getHintRect();

    int lineWidth = label->getTextLabel().lineWidth();
    int height = qRound(boundingRect.height()) + 2*lineWidth;
    int width = qRound(boundingRect.width()) + 2*lineWidth;

    int x = label->getCoord().x();
    int y = label->getCoord().y();

	if (x + width/2 - 1 > nPoints - 2) {
        x = nPoints - width - 2;
	} else {
        x = (x > width/2 + 1) ? (x - width/2 - 1) : 2;
	}

	if (rect.top() > y - label->getSize() - height) {
        label->setHintRect(QRect(x, y + label->getSize() + 1, width, height));
	} else {
        label->setHintRect(QRect(x, y - label->getSize() - height, width, height));
	}

	if (label->attachedLabel != NULL) {
        calculatePositionOfLabel(label, nPoints);
	}

    return 0;
}
void GSequenceGraphDrawer::calculatePositionOfLabel(GraphLabel *label, int nPoints)
{
    GraphLabel *attachedLabel = label;

    int groupWidth = 0;
    while(NULL != attachedLabel) {
        groupWidth += attachedLabel->getHintRect().width();
        attachedLabel = attachedLabel->attachedLabel;
    }
    int x = label->getCoord().x();
	if (x + groupWidth/2 - 1 > nPoints) {
        x = nPoints - groupWidth/2;
	} else {
        x = (x > groupWidth/2) ? x : groupWidth/2;
	}

    attachedLabel = label;
    while(NULL != attachedLabel) {
        QRect hintRect = attachedLabel->getHintRect();
        groupWidth -= 2*hintRect.width();
        int shift = x + groupWidth/2 - hintRect.left();
        hintRect.adjust(shift, 0, shift, 0);
        attachedLabel->setHintRect(hintRect);
        attachedLabel = attachedLabel->attachedLabel;
    }
}

void GSequenceGraphDrawer::updateMovingLabels(GSequenceGraphData *graph, GraphLabel *label, const QRect &rect)
{
    int errorCode = updateStaticLabels(graph, label, rect);
	if (errorCode != 0) {
        return;
	}
    QRect textRect = label->getHintRect();
    int height = textRect.height();
    if(mLabelCoordY + height + label->getSize() > label->getCoord().y()) {
        textRect.setTop(label->getCoord().y() - height - label->getSize()/2 - 4);
        textRect.setBottom(label->getCoord().y() - label->getSize()/2 - 4);
    } else {
        textRect.setTop(mLabelCoordY);
        textRect.setBottom(mLabelCoordY + height);
    }

    label->setColor(label->getFillingColor(), Qt::red);
    label->setHintRect(textRect);
}

int GSequenceGraphDrawer::calculateLabelData(const QRect &rect, const PairVector &points, GraphLabel *label) {
    int graphHeight = rect.bottom() - rect.top() - 2;
    const U2Region& visibleRange = view->getVisibleRange();
    int xcoordInRect;
	if (visibleRange.contains(label->getPosition())) {
        xcoordInRect = qRound((label->getPosition() - visibleRange.startPos) * rect.width() / visibleRange.length);
	} else {
        label->hide();
        return 1;
    }
    if (xcoordInRect >= points.firstPoints.size()) {
        label->hide();
        return 1;
    }
    int nPoints = rect.width();
    float value = calculateLabelValue(nPoints, points, label, xcoordInRect);
	if (2 * globalMax == value) {
        return 1;
	}
    int ycoordInRect;
    QString text;
    float heightScalingFactor = (globalMin == globalMax) ? 1 : graphHeight / (globalMax - globalMin);
    if (points.useIntervals) {
        float maxValue = points.firstPoints.at(xcoordInRect);
        float minValue = points.secondPoints.at(xcoordInRect);
        int pos = qRound(label->getPosition());
		if(maxValue != minValue) {
            text = GSequenceGraphView::tr("[%2, max:%3, min:%4]").arg(QString::number(pos)).arg(QString::number(maxValue)).arg(QString::number(minValue));
		} else {
            text = GSequenceGraphView::tr("[%2, %3]").arg(QString::number(pos)).arg(QString::number(maxValue));
		}
    
        float avgValue = (maxValue + minValue) / 2;
        label->setValue(avgValue);
        ycoordInRect = qRound((avgValue - globalMin) * heightScalingFactor);
    }
    else
    {
        int pos = qRound(label->getPosition());
        text = GSequenceGraphView::tr("[%2, %3]").arg(QString::number(pos)).arg(QString::number(value));
        label->setValue(value);
        ycoordInRect = qRound((value - globalMin) * heightScalingFactor);
    }

    QPoint labelCoord(xcoordInRect, rect.bottom() - 1 - ycoordInRect);
    label->setCoord(labelCoord);
    label->setHintText(text);
    label->getTextLabel().adjustSize();
    return 0;
}
float GSequenceGraphDrawer::calculateLabelValue(int nPoints, const PairVector &points, GraphLabel *label, int xcoordInRect)
{
    float value = points.firstPoints.at(xcoordInRect);
    float prevValue = value;
    int prevX;
    for(prevX = xcoordInRect; isUnknownValue(prevValue); prevX--) {
        if (prevX <= 0) {
            label->hide();
            return 2 * globalMax;
        }
        prevValue = points.firstPoints.at(prevX);
    }
    float nextValue = points.firstPoints.at(xcoordInRect);
    int   nextX;
    for(nextX = xcoordInRect; isUnknownValue(nextValue); nextX++) {
        if (nextX >= points.firstPoints.size()) {
            label->hide();
            return 2*globalMax;
        }
        nextValue = points.firstPoints.at(nextX);
    }
	if (prevX != nextX) {
        value = prevValue + (nextValue-prevValue)*(xcoordInRect - prevX)/(nextX-prevX);
	}

    const int comparisonWindowSize = 50;

    int startPos = qMax(xcoordInRect, 0);
    startPos = qMin(startPos, nPoints - comparisonWindowSize/2 - 1);
    U2Region comparisonWindow(startPos, comparisonWindowSize/2);
    bool isExtremum = isExtremumPoint(nPoints, points, value, comparisonWindow);
	if(true == isExtremum) {
        label->mark();
	} else {
        label->unmark();
	}

    return value;
}
bool GSequenceGraphDrawer::isExtremumPoint(int npoints, const PairVector& points, float value, U2Region& comparisonWindow)
{
    int compareRes = -2;
    int length = comparisonWindow.length;
    int counter = 0;
    const QVector<float> &firstPoints = points.firstPoints;
    for(int x = comparisonWindow.startPos; counter <= length; x++)
    {
		if (x >= npoints - 2) {
            break;
		}
		if (isUnknownValue(firstPoints.at(x))) {
            continue;
		}
        counter++;
        float comparedValue = firstPoints.at(x);
        if(true == qFuzzyCompare(value, comparedValue)) {
			if(0 == compareRes) {
                return false;
			}
            compareRes = 0;
        } else {
            bool condition = value > comparedValue;
			if((true == condition && compareRes == -1) || (false == condition && compareRes == 1)) {
                return false;
			}
            compareRes = condition ? 1 : -1;
        }
    }
    counter = 0;
    for(int x = comparisonWindow.startPos; counter <= length; x--) {
		if (x <= 0) {
            break;
		}
		if (isUnknownValue(firstPoints.at(x))) {
            continue;
		}
        counter++;
        float comparedValue = firstPoints.at(x);
        if (qFuzzyCompare(value, comparedValue)) {
			if (compareRes == 0) {
                return false;
			}
            compareRes = 0;
        } else {
            bool condition = value > comparedValue;
			if ((condition && compareRes == -1) || (!condition && compareRes == 1)) {
                return false;
			}
            compareRes = condition ? 1 : -1;
        }
    }
	if (compareRes == 0|| compareRes == -2) {
        return false;
	}
    return true;
}

static void align(int start, int end, int win, int step, int seqLen, int& alignedFirst, int& alignedLast) {
    int win2 = (win + 1) / 2;
    int notAlignedFirst = start - win2;
    alignedFirst = qMax(0, notAlignedFirst - notAlignedFirst % step);

    int notAlignedLast = end + win + step;
    alignedLast = notAlignedLast - notAlignedLast % step;
    while (alignedLast + win2 >= end + step) {
        alignedLast-=step;
    }
    while (alignedLast > seqLen - win) {
        alignedLast-=step;
    }
    assert(alignedLast % step == 0);
    assert(alignedFirst % step == 0);
    assert(alignedLast < end);
}




void GSequenceGraphDrawer::calculatePoints(GSequenceGraphData* d, PairVector& points, float& min, float& max, int numPoints) {
    const U2Region& vr = view->getVisibleRange();

    int step = wdata.step;
    int win = wdata.window, win2 = (win+1)/2;
    qint64 seqLen = view->getSequenceLength();

    points.firstPoints.resize(numPoints);
    points.firstPoints.fill(UNKNOWN_VAL);
    points.secondPoints.resize(numPoints);
    points.secondPoints.fill(UNKNOWN_VAL);
    /*
    points.cutoffPoints.resize(seqLen - win);
    points.cutoffPoints.fill(UNKNOWN_VAL);
    */
    min = UNKNOWN_VAL;
    max = UNKNOWN_VAL;
    int alignedFirst = 0; //start point for the first window
    int alignedLast = 0; //start point for the last window
    align(vr.startPos, vr.endPos(), win, step, seqLen, alignedFirst, alignedLast);
    int nSteps = (alignedLast - alignedFirst) / step;
    
    bool winStepNotChanged = win == d->cachedW && step == d->cachedS ;
    bool numPointsNotChanged = numPoints == d->cachedData.firstPoints.size();

    bool useCached = vr.length == d->cachedLen && vr.startPos == d->cachedFrom 
        && winStepNotChanged && numPointsNotChanged;
    
    if (useCached) {
        points = d->cachedData;
    } else if (nSteps > numPoints) {
        points.useIntervals = true;
        int stepsPerPoint = nSteps / points.firstPoints.size();
        int basesPerPoint = stepsPerPoint * step;

        //<=step because of boundary conditions -> number of steps can be changed if alignedLast+w2 == end
        bool offsetIsTooSmall = qAbs((d->alignedLC - d->alignedFC) - (alignedLast - alignedFirst)) <= step
                        && (qAbs(alignedFirst - d->alignedFC) < basesPerPoint);

        if (offsetIsTooSmall && winStepNotChanged && numPointsNotChanged && vr.length == d->cachedLen ) {
            useCached = true;
            points = d->cachedData;
        } else {
            calculateWithFit(d, points, alignedFirst, alignedLast);
            calculateCutoffPoints(d, points, alignedFirst, alignedLast);
        }
    } else {
        points.useIntervals = false;
        if(vr.startPos + win2 <= seqLen){
            calculateWithExpand(d, points, alignedFirst, alignedLast);
            calculateCutoffPoints(d, points, alignedFirst, alignedLast);      
        }
    }

    // Calculate min-max values, ignore unknown values
    bool inited = false; min = 0; max = 0;
    foreach(float p, points.firstPoints) {
        if (isUnknownValue(p)) {
            continue;
        }
        if (!inited) {
            inited = true;
            min = p;
            max = p;
        } else {
            min = qMin(p, min);
            max = qMax(p, max);
        }
    }
    // If interval based graph -> adjust min-max values with second graph data
    if (points.useIntervals)    {
        foreach(float p, points.secondPoints) {
            if (isUnknownValue(p)) {
                continue;
            }
            min = qMin(p, min);
            max = qMax(p, max);

        }
    }
    if (useCached) {
        return;
    }
    
    // cache this result
    d->cachedData = points;
    d->cachedFrom = vr.startPos;
    d->cachedLen = vr.length;
    d->cachedW = win;
    d->cachedS = step;
    d->alignedFC = alignedFirst;
    d->alignedLC = alignedLast;
}

void GSequenceGraphDrawer::calculateCutoffPoints(GSequenceGraphData* d, PairVector& points, int alignedFirst, int alignedLast){
    Q_UNUSED(alignedFirst);
    Q_UNUSED(alignedLast);
    points.cutoffPoints.clear();
   
    int win = wdata.window;
    U2SequenceObject* o = view->getSequenceObject();

    U2Region r(alignedFirst, alignedLast - alignedFirst + win);
    if( r.startPos + win > o->getSequenceLength() ){
        return;
    }

    d->ga->calculate(points.cutoffPoints, view->getSequenceObject(), r, &wdata);
}

void GSequenceGraphDrawer::calculateWithFit(GSequenceGraphData* d, PairVector& points, int alignedFirst, int alignedLast) {
    int nPoints = points.firstPoints.size();
    float basesPerPoint = (alignedLast - alignedFirst) / float(nPoints);
    assert(int(basesPerPoint) >= wdata.step); //ensure that every point is associated with some step data
    QVector<float> pointData;
    U2SequenceObject* o = view->getSequenceObject();
    qint64 len = qMax(qint64(basesPerPoint), wdata.window);
#ifdef DEBUG
    int lastBase = alignedLast + wdata.window;
#endif // DEBUG
    for (int i = 0; i < nPoints; i++) {
        pointData.clear();
        qint64 startPos = alignedFirst + qint64(i * basesPerPoint);
        U2Region r(startPos, len);
#ifdef DEBUG
        assert(r.endPos() <= lastBase);
#endif
        d->ga->calculate(pointData, o, r, &wdata);
        float min, max;
        GSequenceGraphUtils::calculateMinMax(pointData, min, max);

        points.firstPoints[i] = max; //BUG:422: support interval based graph!!!
        points.secondPoints[i] = min;
    }
}

void GSequenceGraphDrawer::calculateWithExpand(GSequenceGraphData* d, PairVector& points, int alignedFirst, int alignedLast) {
    int win = wdata.window;
    int win2 = (win+1)/2;
    int step = wdata.step;
    assert((alignedLast - alignedFirst) % step == 0);

    U2Region r(alignedFirst, alignedLast - alignedFirst + win);
    U2SequenceObject* o = view->getSequenceObject();
    QVector<float> res;

    if( r.startPos + win > o->getSequenceLength() ){
        return;
    }

    d->ga->calculate(res, o, r, &wdata);
    const U2Region& vr = view->getVisibleRange();

    assert(alignedFirst + win2 + step >= vr.startPos); //0 or 1 step is before the visible range
    assert(alignedLast + win2 - step <= vr.endPos()); //0 or 1 step is after the the visible range

    bool hasBeforeStep = alignedFirst + win2 < vr.startPos;
    bool hasAfterStep  = alignedLast + win2 >= vr.endPos();

    int firstBaseOffset = hasBeforeStep ?
        (step - (vr.startPos - (alignedFirst + win2)))
        : (alignedFirst + win2 - vr.startPos);
    int lastBaseOffset = hasAfterStep ?
        (step - (alignedLast + win2 - vr.endPos()))  //extra step on the right is available
        : (vr.endPos() - (alignedLast + win2)); // no extra step available -> end of the sequence

    assert(firstBaseOffset >= 0 && lastBaseOffset >= 0);
    assert(hasBeforeStep ? (firstBaseOffset < step && firstBaseOffset!=0): firstBaseOffset <= win2);
    assert(hasAfterStep ? (lastBaseOffset <= step && lastBaseOffset !=0) : lastBaseOffset < win2 + step);

    float base2point = points.firstPoints.size() / (float)vr.length;

    int ri = hasBeforeStep ? 1 : 0;
    int rn = hasAfterStep ? res.size()-1 : res.size();
    for (int i=0;  ri < rn; ri++, i++) {
        int b = firstBaseOffset + i * step;
        int px = int(b * base2point);
        assert(px < points.firstPoints.size());
        points.firstPoints[px] = res[ri];
    }

    //restore boundary points if possible
    if(res.size() < 2){
        return;
    }

    if (hasBeforeStep && !isUnknownValue(res[0]) && !isUnknownValue(res[1])) {
        assert(firstBaseOffset > 0);
        float k = firstBaseOffset / (float)step;
        float val = res[1] + (res[0]-res[1])*k;
        points.firstPoints[0] = val;
    }

    if (hasAfterStep && !isUnknownValue(res[rn-1]) &&  !isUnknownValue(res[rn])) {
        assert(lastBaseOffset > 0);
        float k = lastBaseOffset / (float)step;
        float val = res[rn-1] + (res[rn]-res[rn-1])*k;
        points.firstPoints[points.firstPoints.size()-1] = val;
    }
}


void GSequenceGraphDrawer::showSettingsDialog() {
    
    GraphSettingsDialog dlg(this, U2Region(1, view->getSequenceLength()-1), view);

    if (dlg.exec() == QDialog::Accepted) {
        wdata.window = dlg.getWindowSelector()->getWindow();
        wdata.step = dlg.getWindowSelector()->getStep();
        commdata.enableCuttoff = dlg.getMinMaxSelector()->getState();
        commdata.min = dlg.getMinMaxSelector()->getMin();
        commdata.max = dlg.getMinMaxSelector()->getMax();
        lineColors = dlg.getColors();
        view->update();
        view->changeLabelsColor();
    }
}
} // namespace
