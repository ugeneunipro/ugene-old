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

#ifndef _U2_DOT_PLOT_CLASSES_H_
#define _U2_DOT_PLOT_CLASSES_H_

#include "DotPlotFilterDialog.h"

#include <U2Algorithm/RepeatFinderSettings.h>
#include <U2Core/U2Region.h>

#include <QtGui/QMessageBox>
#include <QtGui/QPainter>
#include <QtCore/QMutex>

namespace U2 {

// mini map on the DotPlotWidget
class DotPlotMiniMap {
public:
    DotPlotMiniMap (int bigMapW, int bigMapH, float ratio);
    void draw(QPainter &p, int shiftX, int shiftY, const QPointF &zoom) const;

    QRectF getBoundary() const;
    QPointF fromMiniMap(const QPointF &p, const QPointF &zoom) const;

private:
    int x, y, w, h;
    float ratio;
};

// error messages and dialogs
class DotPlotDialogs: QObject {
    Q_OBJECT
public:
    enum Errors {ErrorOpen, ErrorNames, NoErrors};

    static void taskRunning();
    static int saveDotPlot();
    static void fileOpenError(const QString &filename);
    static void filesOpenError();
    static int loadDifferent();
    static void loadWrongFormat();
    static void wrongAlphabetTypes();
    static void tooManyResults();
};



struct DotPlotResults {
    DotPlotResults(): x(0), y(0), len(0){};
    DotPlotResults(int _x, int _y, int _len):x(_x), y(_y), len(_len){};

    int x, y, len;

    inline bool intersectRegion(const U2Region& r, const FilterIntersectionParameter& currentIntersParam){
        qint64 sd = - r.startPos;
        if(currentIntersParam == SequenceY){
            sd += y;
        }else{
            sd += x;
        }
        return (sd >= 0) ? (sd < r.length) : (-sd < len);
        
    }
};

static bool DPResultLessThenX(DotPlotResults& r1, DotPlotResults& r2){
    return r1.x < r2.x;
}

static bool DPResultLessThenY(DotPlotResults& r1, DotPlotResults& r2){
    return r1.y < r2.y;
}

// Listener which collect results from an algorithm
class DotPlotResultsListener : public RFResultsListener {
    friend class DotPlotWidget;

public:
    DotPlotResultsListener();
    ~DotPlotResultsListener();

    void setTask(Task *);

    virtual void onResult(const RFResult& r);
    virtual void onResults(const QVector<RFResult>& v);

private:
    QList<DotPlotResults> *dotPlotList;
    QMutex mutex;

    bool stateOk;

    static const int maxResults = 8*1024*1024;
    Task *rfTask;
};

// apply rev-compl transformation for X sequence results
class DotPlotRevComplResultsListener : public DotPlotResultsListener {
public:
    DotPlotRevComplResultsListener() : xLen(0) {}
    virtual void onResult(const RFResult& r);
    virtual void onResults(const QVector<RFResult>& v);
   
    int xLen;
};


} // namespace

#endif // _U2_DOT_PLOT_CLASSES_H_
