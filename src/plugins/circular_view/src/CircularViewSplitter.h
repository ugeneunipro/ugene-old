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

#ifndef _U2_CIRCULAR_VIEW_SPLITTER_H_
#define _U2_CIRCULAR_VIEW_SPLITTER_H_

#include "CircularView.h"
#include "CircularItems.h"

#include <U2View/ADVSplitWidget.h>

#include <QToolBar>
#include <QToolButton>
#include <QAction>


class QScrollBar;

namespace U2 {

class CircularViewHeaderWidget;
class HBar;

class CircularViewSplitter : public ADVSplitWidget{
    Q_OBJECT
public:
    CircularViewSplitter( AnnotatedDNAView* view);
    // there are no special object handling with this view
    // it only shows existing AO only
    virtual bool acceptsGObject(GObject*) {return false;}
    virtual void updateState(const QVariantMap& m);
    virtual void saveState(QVariantMap& m);
    void addView(CircularView* view);
    void adaptSize();
    void removeView(CircularView* view);
    bool isEmpty();
    const QList<CircularView*>& getViewList() const { return circularViewList; }
protected slots:
    void sl_export();
    void sl_horSliderMoved(int);
    void sl_moveSlider(int);
    
    void sl_updateZoomInAction(bool);
    void sl_updateZoomOutAction(bool);
    void sl_updateFitInViewAction(bool);
private:
    QSplitter* splitter;
    HBar* toolBar;    
    QScrollBar* horScroll;

    QToolButton* tbZoomIn;
    QToolButton* tbZoomOut;
    QToolButton* tbFitInView;
    QToolButton* tbExport;

    QList<CircularView*> circularViewList;
};

} //namespace U2

#endif //_U2_CIRCULAR_VIEW_SPLITTER_H_
