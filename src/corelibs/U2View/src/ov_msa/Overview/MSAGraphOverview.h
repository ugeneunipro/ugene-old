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

#ifndef _U2_MSA_GRAPH_OVERVIEW_H_
#define _U2_MSA_GRAPH_OVERVIEW_H_

#include <U2Core/global.h>
#include <U2Core/BackgroundTaskRunner.h>

#include "MSAOverview.h"

namespace U2 {

class MSAEditorConsensusCache;

class CalcConsensusOverviewPolygonTask : public BackgroundTask<QPolygonF> {
    Q_OBJECT
public:
    CalcConsensusOverviewPolygonTask(QSharedPointer<MSAEditorConsensusCache> _consensus, int _width)
        : BackgroundTask<QPolygonF>(tr("Render consensus overview"), TaskFlag_None),
          consensus(_consensus),
          width(_width){}
    void run();
signals:
    void si_calculationStarted();
    void si_calculationStoped();
private:
    void constructConsensusPolygon(QPolygonF &polygon);
    QSharedPointer<MSAEditorConsensusCache>    consensus;
    int                         width;
};


class MSAGraphOverviewDisplaySettings {
public:
    enum GraphType {
        Hystogram,
        Line,
        Area
    };

    enum OrientationMode {
        FromTopToBottom,
        FromBottomToTop
    };

    MSAGraphOverviewDisplaySettings()
        : color(Qt::blue),
          type(Area),
          orientation(FromBottomToTop) {}

    QColor      color;
    GraphType   type;
    OrientationMode orientation;
};


class U2VIEW_EXPORT MSAGraphOverview : public MSAOverview {
    Q_OBJECT
public:
    MSAGraphOverview(MSAEditorUI* ui);
    bool isValid() { return true; }
    const static int FIXED_HEIGHT = 70;

    void cancelRendering();
    bool isReadyToClose() { return !isRendering; }

    QColor getCurrentColor() const { return displaySettings->color; }
    MSAGraphOverviewDisplaySettings::GraphType getCurrentGraphType() const { return displaySettings->type; }
    MSAGraphOverviewDisplaySettings::OrientationMode getCurrentOrientationMode() const
    { return displaySettings->orientation; }

public slots:
    void sl_visibleRangeChanged();
    void sl_redraw();
    void sl_drawGraph();

    void sl_graphOrientationChanged(MSAGraphOverviewDisplaySettings::OrientationMode orientation);
    void sl_graphTypeChanged(MSAGraphOverviewDisplaySettings::GraphType type);
    void sl_graphColorChanged(QColor color);

    void sl_startRendering();
    void sl_stopRendering();

    void sl_blockRendering();
    void sl_unblockRendering(bool update);

protected:
    void paintEvent(QPaintEvent* e);
    void resizeEvent(QResizeEvent* e);

private:
    void drawVisibleRange(QPainter& p);
    void drawOverview(QPainter& p);
    void moveVisibleRange(QPoint pos);

    QPixmap             cachedConsensus;

    bool redrawGraph;
    bool isRendering;
    bool isBlocked;

    BackgroundTaskRunner<QPolygonF> overviewPixmapTaskRunner;
    CalcConsensusOverviewPolygonTask * task;

    MSAGraphOverviewDisplaySettings*    displaySettings;
};

} // namespace

#endif // _U2_MSA_GRAPH_OVERVIEW_H_
