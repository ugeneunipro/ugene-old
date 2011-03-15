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

#ifndef __ASSEMBLY_OVERVIEW_H__
#define __ASSEMBLY_OVERVIEW_H__

#include <QtCore/QSharedPointer>
#include <QtGui/QWidget>
#include <QtGui/QPixmap>

#include <U2Core/Task.h>

namespace U2 {

class AssemblyModel;

class AssemblyOverviewRenderTask: public Task {
    Q_OBJECT
public:
    AssemblyOverviewRenderTask(QSharedPointer<AssemblyModel> model, QSize imageSize);
    virtual void run();
    inline QImage getResult() const {return result;};
private:
    QSharedPointer<AssemblyModel> model;
    QImage result;
};

class BackgroundRenderer: public QObject {
    Q_OBJECT
public:
    BackgroundRenderer(QSharedPointer<AssemblyModel> model_);
    void render(const QSize & size_);
    QImage getImage() const;
signals:
    void si_rendered();
private slots:
    void sl_redrawFinished();
private:
    AssemblyOverviewRenderTask * renderTask;
    QImage result;
    QSharedPointer<AssemblyModel> model;
    QSize size;
    bool redrawRunning;
    bool redrawNeeded;
};

class AssemblyBrowserUi;
class AssemblyBrowserWindow;
class AssemblyOverviewRenderTask;

class AssemblyOverview: public QWidget {
    Q_OBJECT
public:
    AssemblyOverview(AssemblyBrowserUi * ui);

protected:
    void paintEvent(QPaintEvent * e);
    void resizeEvent(QResizeEvent * e);
    void mousePressEvent(QMouseEvent * me);
    void mouseMoveEvent(QMouseEvent * me);
    void mouseReleaseEvent(QMouseEvent * me);

private slots:
    void sl_visibleAreaChanged();
    void sl_redraw();

private:
    qint64 calcXAssemblyCoord(int x);
    qint64 calcYAssemblyCoord(int y);

    QRect calcCurrentSelection() const;
    void moveSelectionToPos(QPoint pos, bool moveModel = true);

    void connectSlots();
    void initSelectionRedraw();

    void drawAll();
    void drawSelection(QPainter & p);
    void drawCoordLabels(QPainter & p);

private:
    AssemblyBrowserUi * ui;
    AssemblyBrowserWindow * window;
    QSharedPointer<AssemblyModel> model;

    QRect cachedSelection;

    QPixmap cachedView;
    bool redrawSelection;

    QPixmap cachedBackground;
    BackgroundRenderer bgrRenderer;

    bool scribbling;

    const static int FIXED_HEIGHT = 100;
};

} //ns

#endif 
