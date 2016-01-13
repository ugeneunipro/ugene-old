/**
 * UGENE - Integrated Bioinformatics Tools.
 * Copyright (C) 2008-2016 UniPro <ugene@unipro.ru>
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

#include <U2View/GraphMenu.h>
#include <U2View/GSequenceGraphView.h>
#include <U2View/ADVSequenceWidget.h>

#if (QT_VERSION < 0x050000) //Qt 5
#include <QtGui/QMenu>
#include <QtGui/QToolBar>
#else
#include <QtWidgets/QMenu>
#include <QtWidgets/QToolBar>
#endif
#include <QtCore/QPointer>

namespace U2 {

class MWMDIWindow;
class GObjectView;
class GSequenceGraphFactory;
class GSequenceGraphData;
class GraphAction;
class DNAGraphPackViewContext;
class ADVSingleSequenceWidget;


class DNAGraphPackPlugin : public Plugin {
    Q_OBJECT
public:
    DNAGraphPackPlugin();

private:
    DNAGraphPackViewContext* ctx;
};


class DNAGraphPackViewContext : public GObjectViewWindowContext
{
    Q_OBJECT
public:
    DNAGraphPackViewContext(QObject* parent);

private:
    QList<GSequenceGraphFactory*>   graphFactories;

    virtual void initViewContext(GObjectView* view);

private slots:
    void sl_sequenceWidgetAdded(ADVSequenceWidget*);
};



} //namespace

#endif
