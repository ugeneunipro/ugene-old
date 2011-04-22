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

#ifndef __ASSEMBLY_RULER__
#define __ASSEMBLY_RULER__


#include <QtGui/QWidget>
#include <QtCore/QSharedPointer>

namespace U2 {

class AssemblyBrowserUi;
class AssemblyBrowser;
class AssemblyModel;

/**
 * Ruler widget provides auto-adjustable scale, labeled notches and a 
 * mouse-tracking cursor showing the current coordinate in assembly.
 */
class AssemblyRuler : public QWidget {
    Q_OBJECT
public:
    AssemblyRuler(AssemblyBrowserUi * ui);

    void setShowCoordsOnRuler(bool showCoords);
    bool getShowCoordsOnRuler()const;
    
protected:
    void paintEvent(QPaintEvent * e);
    void resizeEvent(QResizeEvent * e);
    void mouseMoveEvent(QMouseEvent * e);

public slots:
    void sl_handleMoveToPos(const QPoint &);
    
private slots:
    void sl_redraw();

private:
    void connectSlots();
    void drawAll();
    void drawRuler(QPainter & p);
    void drawCursor(QPainter & p);
    
private:
    AssemblyBrowserUi * ui;
    AssemblyBrowser * browser;
    QSharedPointer<AssemblyModel> model;

    bool redraw;
    QPixmap cachedView;

    int cursorPos; //x-coord of the cursor on the widget

    //cached ruler labels. Rendered once in paintEvent,
    //used on each cursor redraw
    QList<QRect> cachedLabelsRects;
    QList<QImage> cachedLabels;
    
    bool showCoords;
};

} //ns

#endif
