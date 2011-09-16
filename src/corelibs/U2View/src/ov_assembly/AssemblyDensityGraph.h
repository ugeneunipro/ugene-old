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

#ifndef __ASSEMBLY_DENSITY_GRAPH_H__
#define __ASSEMBLY_DENSITY_GRAPH_H__

#include <QtGui/QWidget>
#include <QtCore/QSharedPointer>

#include "AssemblyModel.h"
#include "CoverageInfo.h"
#include "BackgroundTaskRunner.h"

namespace U2 {

class AssemblyBrowserUi;
class AssemblyBrowser;

class AssemblyDensityGraph: public QWidget {
    Q_OBJECT
public:
    AssemblyDensityGraph(AssemblyBrowserUi * ui);
protected:
    void paintEvent(QPaintEvent * e);
    void mouseMoveEvent(QMouseEvent * e);

signals:
    void si_mouseMovedToPos(const QPoint &);

private slots:
    void sl_redraw();
    void sl_launchCoverageCalculation();
    void sl_onOffsetsChanged();

private:
    void connectSlots();
    void drawAll();
    void drawGraph(QPainter & p);


    AssemblyBrowserUi * ui;
    AssemblyBrowser * browser;
    QSharedPointer<AssemblyModel> model;

    QPixmap cachedView;
    bool redraw;
    const static int FIXED_HEIGHT = 25;

    qint64 previousXOffset;

    BackgroundTaskRunner<CoverageInfo> coverageTaskRunner;
};

} //ns

#endif 
