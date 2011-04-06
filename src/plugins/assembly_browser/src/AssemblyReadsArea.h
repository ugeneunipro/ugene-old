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

#ifndef __ASSEMBLY_READS_AREA_H__
#define __ASSEMBLY_READS_AREA_H__

#include <assert.h>

#include <QtGui/QWidget>
#include <QtGui/QScrollBar>
#include <QtGui/QLabel>
#include <QtCore/QSharedPointer>

#include <U2Core/U2Assembly.h>
#include "AssemblyCellRenderer.h"

namespace U2 {

class AssemblyBrowser;
class AssemblyBrowserUi;
class AssemblyModel;
class AssemblyReadsArea;

// hint that is shown under cursor for current read
class ReadsHint : public QFrame {
    Q_OBJECT
public:
    static const QPoint OFFSET_FROM_CURSOR;
    
public:
    ReadsHint(QWidget * p);

    void setLength(qint64 len);
    void setFromTo(qint64 from, qint64 to);
    void setCigar(const QString & ci);
    void setStrand(bool onCompl);
    
protected:
    bool eventFilter(QObject *, QEvent *);
    void leaveEvent(QEvent * e);
    
private:
    QLabel * fromToLabel;
    QLabel * lengthLabel;
    QLabel * cigarLabel;
    QLabel * strandLabel;
};

class AssemblyReadsArea: public QWidget {
    Q_OBJECT
public:
    AssemblyReadsArea(AssemblyBrowserUi * ui, QScrollBar * hBar, QScrollBar * vBar);

protected:
    void paintEvent(QPaintEvent * e);
    void resizeEvent(QResizeEvent * e);
    void wheelEvent(QWheelEvent * e);
    void mousePressEvent(QMouseEvent * e);
    void mouseReleaseEvent(QMouseEvent * e);
    void mouseMoveEvent(QMouseEvent * e);
    void leaveEvent(QEvent * e);
    void hideEvent(QHideEvent * e);
    bool event(QEvent * e);
    void keyPressEvent(QKeyEvent * e);
    
private:
    void initRedraw();
    void connectSlots();
    void setupHScrollBar();
    void setupVScrollBar();

    void drawAll();
    void drawDensityGraph(QPainter & p);
    void drawReads(QPainter & p);
    void drawHint(QPainter & p);
    
    int calcFontPointSize() const;

signals:
    void si_heightChanged();
    void si_mouseMovedToPos(const QPoint &);
    
public slots:
    void sl_hideHint();    
    
private slots:
    void sl_onHScrollMoved(int pos);
    void sl_onVScrollMoved(int pos);
    void sl_zoomOperationPerformed();
    void sl_redraw();
    
private:
    AssemblyBrowserUi * ui;
    AssemblyBrowser * browser;
    QSharedPointer<AssemblyModel> model;

    bool redraw;
    QPixmap cachedView;
    AssemblyCellRenderer cellRenderer;
    
    QScrollBar * hBar;
    QScrollBar * vBar;
    
    // caches reads that are visible on a screen
    class ReadsCache {
    public:
        bool isEmpty() const {
            return data.isEmpty();
        }
        QList<U2AssemblyRead> data;
        U2Region visibleBases;
        U2Region visibleRows;
        int letterWidth;
        qint64 xOffsetInAssembly;
        qint64 yOffsetInAssembly;
    };
    ReadsCache cachedReads;
    QPoint curPos;
    bool redrawHint;
    ReadsHint hint;
    
    // needed to move by-letter when scribbling
    class ReadsMover {
    public:
        ReadsMover() : cellWidth(0) {};
        ReadsMover(int cellWidth_, QPoint initPos) : lastPos(initPos), cellWidth(cellWidth_ ? cellWidth_ : 1){}
        void handleEvent(QPoint newPos) {
            assert(cellWidth);
            QPoint diff_ = newPos - lastPos;
            lastPos = newPos;
            diff += diff_;
        }
        int getXunits() {
            assert(cellWidth);
            int result = diff.x() / cellWidth;
            diff.setX(diff.x() % cellWidth);
            return result;
        }
        int getYunits() {
            assert(cellWidth);
            int result = diff.y() / cellWidth;
            diff.setY(diff.y() % cellWidth);
            return result;
        }
    private:
        QPoint lastPos;
        QPoint diff;
        int cellWidth;
    };

    bool scribbling;
    ReadsMover mover;
};

} //ns

#endif 
