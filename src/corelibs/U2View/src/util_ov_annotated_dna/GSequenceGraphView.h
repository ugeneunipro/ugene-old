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

#ifndef _U2_GSEQUENCE_GRAPH_VIEW_H_
#define _U2_GSEQUENCE_GRAPH_VIEW_H_

#include "GSequenceLineView.h"
#include "ADVGraphModel.h"

#include <QtGui/QMenu>

namespace U2 {

class GSequenceGraphWindowData;
class GSequenceGraphView;
class GObjectView;

//////////////////////////////////////////////////////////////////////////
/// View
class U2VIEW_EXPORT GSequenceGraphView : public GSequenceLineView {
    Q_OBJECT
public:
    GSequenceGraphView(QWidget* p, ADVSequenceObjectContext* ctx, GSequenceLineView* baseView, const QString& vName);
    ~GSequenceGraphView();

    const QString& getGraphViewName() const {return vName;}

    void addGraphData(GSequenceGraphData* g);
    
    void setGraphDrawer(GSequenceGraphDrawer* gd);

    const QList<GSequenceGraphData*>& getGraphs() const {return graphs;}
    
    GSequenceGraphDrawer* getGSequenceGraphDrawer() const {return graphDrawer;}

    void buildPopupMenu(QMenu& m);

protected:
    virtual void pack();

private slots:
    void sl_onShowVisualProperties(bool);

private:
    GSequenceLineView*          baseView;
    QString                     vName;
    QList<GSequenceGraphData*>  graphs;
    GSequenceGraphDrawer*       graphDrawer;
    QAction*                    visualPropertiesAction;
};


class U2VIEW_EXPORT GSequenceGraphViewRA : public GSequenceLineViewRenderArea {
public:
    GSequenceGraphViewRA(GSequenceGraphView* g);
    ~GSequenceGraphViewRA();
    virtual GSequenceGraphView* getGraphView() const {return static_cast<GSequenceGraphView*>(view);}
    
    float getCurrentScale() const;

protected:
    virtual void drawAll(QPaintDevice* pd);
    virtual void drawHeader(QPainter& p);
    void drawSelection(QPainter& p);
private:

    QFont *headerFont;
    int headerHeight;
    QRect graphRect;
    GSequenceGraphDrawer* gd;

};

} // namespace

#endif
