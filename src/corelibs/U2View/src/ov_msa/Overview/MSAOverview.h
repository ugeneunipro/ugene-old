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

#ifndef _U2_MSA_OVERVIEW_H_
#define _U2_MSA_OVERVIEW_H_


#include <U2Core/global.h>

#if (QT_VERSION < 0x050000) //Qt 5
#include <QtGui/QWidget>
#else
#include <QtWidgets/QWidget>
#endif


namespace U2 {

class MSAEditor;
class MSAEditorUI;
class MSAEditorSequenceArea;

#define VISIBLE_RANGE_COLOR QColor(230, 230, 230, 180)
#define SELECTION_COLOR QColor(80, 160, 200, 180)
#define VISIBLE_RANGE_CRITICAL_SIZE 5


class U2VIEW_EXPORT MSAOverview : public QWidget {
    Q_OBJECT
public:
    MSAOverview(MSAEditorUI *_ui);
    virtual bool isValid() { return true; }

public slots:
    virtual void sl_visibleRangeChanged(){}
    virtual void sl_selectionChanged() {}
    virtual void sl_redraw(){}

protected:
    void mousePressEvent(QMouseEvent* );
    void mouseMoveEvent(QMouseEvent* );
    void mouseReleaseEvent(QMouseEvent* );

    virtual void drawOverview(QPainter&){}
    virtual void drawVisibleRange(QPainter&){}
    virtual void drawSelection(QPainter&){}

    virtual void moveVisibleRange(QPoint){}

protected:
    MSAEditor*      editor;
    MSAEditorUI*    ui;
    MSAEditorSequenceArea*  sequenceArea;

    QPixmap cachedView;
    QRect   cachedSelection;
    QRect   cachedVisibleRange;

    bool visibleRangeIsMoving;

    double  stepX;
    double  stepY;
};

} // namespace

#endif // _U2_MSA_OVERVIEW_H_
