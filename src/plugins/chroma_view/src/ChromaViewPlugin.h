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

#ifndef _U2_CHROMA_VIEW_PLUGIN_H_
#define _U2_CHROMA_VIEW_PLUGIN_H_

#include <U2Core/PluginModel.h>
#include <U2Core/AppContext.h>
#include <U2Gui/ObjectViewModel.h>

#include <QtCore/QMap>
#include <QtGui/QAction>

#include <U2View/ADVSequenceWidget.h>

namespace U2 {

class ChromatogramView;

class ChromaViewPlugin : public Plugin {
    Q_OBJECT
public:
    ChromaViewPlugin();
    ~ChromaViewPlugin();
private:
    GObjectViewWindowContext* viewCtx;
};

class ChromaViewContext: public GObjectViewWindowContext {
    Q_OBJECT
public:
    ChromaViewContext(QObject* p);
protected slots:
    void sl_showChromatogram();
    void sl_sequenceWidgetAdded(ADVSequenceWidget*);
    
protected:
    virtual void initViewContext(GObjectView* view);
};    

class ChromaViewAction : public ADVSequenceWidgetAction {
    Q_OBJECT
public:
    ChromaViewAction();
    ~ChromaViewAction();
    ChromatogramView* view;
};

} //namespace

#endif
