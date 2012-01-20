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

#include "ADVGraphModel.h"
#include "GSequenceGraphView.h"
#include "GraphSettingsDialog.h"
#include "WindowStepSelectorWidget.h"

#include <math.h>

namespace U2 {

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
    assert(range.length >= w);
    int steps = (range.length  - w) / s + 1;
    return steps;
}

//////////////////////////////////////////////////////////////////////////
//drawer

const QString GSequenceGraphDrawer::DEFAULT_COLOR(tr("Default color"));

GSequenceGraphDrawer::GSequenceGraphDrawer(GSequenceGraphView* v, const GSequenceGraphWindowData& wd, 
										   QMap<QString,QColor> colors) 
: QObject(v), view(v), lineColors(colors), wdata(wd)
{
    defFont = new QFont("Arial", 8);
	if (colors.isEmpty()) {
		lineColors.insert(DEFAULT_COLOR, Qt::black);
	}
}

GSequenceGraphDrawer::~GSequenceGraphDrawer() {
    delete defFont;
}

//TODO:
#define UNKNOWN_VAL -1 

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


void GSequenceGraphDrawer::drawGraph( QPainter& p, GSequenceGraphData* d, const QRect& rect )
{
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
	if  (lineColors.contains(d->graphName)) {
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
			if (fy1 == UNKNOWN_VAL) {
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
				if (fy2 == UNKNOWN_VAL) {
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
		int ymid = rect.bottom() - 1 - qRound((fymid - min) * kh);
        if(!points.useIntervals){
			for (int i=0, n = points.firstPoints.size(); i < n; i++) {
				fy = points.firstPoints[i];
				rp = false;
				lp = false;
				if (fy == UNKNOWN_VAL) {
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
				prevY = y;
				prevX = x;
				prevFY = (int) fy;
			}
        }else{
            for (int i=0, n = points.firstPoints.size(); i < n; i++) {
                assert(points.firstPoints.size() == points.secondPoints.size());
                fy = points.firstPoints[i], fy2 = points.secondPoints[i];
                rp = false;
                lp = false;
                if (fy == UNKNOWN_VAL) {
                    continue;
                }
                
                assert(fy2 != UNKNOWN_VAL);
                
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
    int win = wdata.window;
    qint64 seqLen = view->getSequenceLength();

    points.firstPoints.resize(numPoints);
    points.firstPoints.fill(UNKNOWN_VAL);
    points.secondPoints.resize(numPoints);
    points.secondPoints.fill(UNKNOWN_VAL);
    min = UNKNOWN_VAL;
    max = UNKNOWN_VAL;
    if (vr.length < win) {
        return;
    }
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
        }
    } else {
        points.useIntervals = false;
        calculateWithExpand(d, points, alignedFirst, alignedLast);
    }

    // Calculate min-max values, ignore unknown values
    bool inited = false; min = 0; max = 0;
    foreach(float p, points.firstPoints) {
        if (p == UNKNOWN_VAL) {
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
            if (p == UNKNOWN_VAL) {
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

void GSequenceGraphDrawer::calculateWithFit(GSequenceGraphData* d, PairVector& points, int alignedFirst, int alignedLast) {
    int nPoints = points.firstPoints.size();
    float basesPerPoint = (alignedLast - alignedFirst) / float(nPoints);
    assert(int(basesPerPoint) >= wdata.step); //ensure that every point is associated with some step data
    QVector<float> pointData;
    U2SequenceObject* o = view->getSequenceObject();
    int len = qMax(int(basesPerPoint), wdata.window);
#ifdef DEBUG
    int lastBase = alignedLast + wdata.window;
#endif // DEBUG
    for (int i = 0; i < nPoints; i++) {
        pointData.clear();
        int startPos = alignedFirst + int(i * basesPerPoint);
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
    if (hasBeforeStep && res[0]!=UNKNOWN_VAL && res[1]!=UNKNOWN_VAL) {
        assert(firstBaseOffset > 0);
        float k = firstBaseOffset / (float)step;
        float val = res[1] + (res[0]-res[1])*k;
        points.firstPoints[0] = val;
    }

    if (hasAfterStep && res[rn-1]!=UNKNOWN_VAL && res[rn]!=UNKNOWN_VAL) {
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
	}
}




} // namespace
