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

#ifndef _U2_DNA_GRAPHPACK_PLUGIN_H_
#define _U2_DNA_GRAPHPACK_PLUGIN_H_

#include <U2Core/PluginModel.h>
#include <U2Gui/ObjectViewModel.h>

#include <U2View/GSequenceGraphView.h>
#include <U2View/ADVSequenceWidget.h>

#include <QtGui/QMenu>
#include <QtGui/QToolBar>
#include <QtCore/QPointer>

namespace U2 {

class MWMDIWindow;
class GObjectView;
class GSequenceGraphFactory;
class GSequenceGraphViewWithFactory;
class GSequenceGraphData;
class GraphAction;
class DNAGraphPackViewContext;
class ADVSingleSequenceWidget;

class DNAGraphPackPlugin : public Plugin {
    Q_OBJECT
public:
    DNAGraphPackPlugin();

private:
    DNAGraphPackViewContext*        ctx;
};

class DNAGraphPackViewContext : public GObjectViewWindowContext {
    Q_OBJECT
public:
    DNAGraphPackViewContext(QObject* p);

protected slots:
    void sl_sequenceWidgetAdded(ADVSequenceWidget*);
    void sl_handleGraphAction();

protected:
    virtual void initViewContext(GObjectView* view);

    QList<GSequenceGraphFactory*>   graphFactories;
};

class GraphMenuAction : public ADVSequenceWidgetAction {
    Q_OBJECT
public:
    GraphMenuAction();
};

class GraphAction : public QAction {
    Q_OBJECT
public:
    GraphAction(GSequenceGraphFactory* f);
    GSequenceGraphFactory*  factory;
    GSequenceGraphView*     view;
};


class GSequenceGraphFactory : public QObject{
public:
    GSequenceGraphFactory(const QString& _name, QObject* p) : QObject(p), graphName(_name){}

    virtual QList<GSequenceGraphData*> createGraphs(GSequenceGraphView* v) = 0;
    virtual GSequenceGraphDrawer* getDrawer(GSequenceGraphView* v) = 0;

    virtual bool isEnabled(DNASequenceObject* o) const  = 0;

    const QString& getGraphName() const {return graphName;}

protected:
    QString graphName;
};

class GSequenceGraphViewWithFactory : public GSequenceGraphView {
    Q_OBJECT
public:
    GSequenceGraphViewWithFactory(ADVSingleSequenceWidget* sw, GSequenceGraphFactory* f);
    GSequenceGraphFactory* getFactory() const {return f;}
private: 
    GSequenceGraphFactory* f;
};




} //namespace

#endif
