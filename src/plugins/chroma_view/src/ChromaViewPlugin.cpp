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

#include "ChromaViewPlugin.h"
#include "ChromatogramView.h"

#include <U2Core/GObject.h>
#include <U2Core/DocumentModel.h>
#include <U2Core/GObjectTypes.h>
#include <U2Core/DNASequenceObject.h>
#include <U2Core/DNAChromatogramObject.h>
#include <U2Gui/MainWindow.h>
#include <U2Core/DocumentSelection.h>

#include <U2View/AnnotatedDNAView.h>
#include <U2View/ADVSingleSequenceWidget.h>
#include <U2View/ADVConstants.h>
#include <U2View/ADVSequenceObjectContext.h>

#include <U2Gui/GUIUtils.h>

#include <QtGui/QMessageBox>
#include <QtGui/QMenu>

namespace U2 {

extern "C" Q_DECL_EXPORT Plugin* U2_PLUGIN_INIT_FUNC() {
    if (AppContext::getMainWindow()) {
        ChromaViewPlugin* plug = new ChromaViewPlugin();
        return plug;
    }
    return NULL;
}


ChromaViewPlugin::ChromaViewPlugin() : Plugin(tr("ChromaView"), tr("chroma_view_plugin_desc")) {
    viewCtx = new ChromaViewContext(this);
    viewCtx->init();
}

ChromaViewPlugin::~ChromaViewPlugin() {
}

#define CHROMA_ACTION_NAME   "CHROMA_ACTION"
#define CHROMA_VIEW_NAME     "CHROMA_VIEW"

ChromaViewContext::ChromaViewContext(QObject* p) : GObjectViewWindowContext(p, ANNOTATED_DNA_VIEW_FACTORY_ID) {
}

void ChromaViewContext::initViewContext(GObjectView* v) {
    AnnotatedDNAView* av = qobject_cast<AnnotatedDNAView*>(v);
    foreach(ADVSequenceWidget* w, av->getSequenceWidgets()) {
        sl_sequenceWidgetAdded(w);
    }
    connect(av, SIGNAL(si_sequenceWidgetAdded(ADVSequenceWidget*)), SLOT(sl_sequenceWidgetAdded(ADVSequenceWidget*)));
    
}

static DNAChromatogramObject* findChromaObj(ADVSingleSequenceWidget* sw) {
    QList<GObject*> chromaObjs = sw->getSequenceObject()->getDocument()->findGObjectByType(GObjectTypes::CHROMATOGRAM);
    if (chromaObjs.isEmpty()) {
        return NULL;
    }
    DNAChromatogramObject* chromaObj = qobject_cast<DNAChromatogramObject*>(chromaObjs.first());
    assert(chromaObj->getChromatogram().seqLength == sw->getSequenceLength());
    return chromaObj;
}

void ChromaViewContext::sl_sequenceWidgetAdded(ADVSequenceWidget* w) {
    ADVSingleSequenceWidget* sw = qobject_cast<ADVSingleSequenceWidget*>(w);
    if (sw == NULL || sw->getSequenceObject() == NULL || findChromaObj(sw) == NULL) {
        return;
    }
    
    ChromaViewAction* action = new ChromaViewAction();
    action->setIcon(QIcon(":chroma_view/images/cv.png"));
    action->setCheckable(true);
    action->setChecked(false);
    action->addToMenu = true;
    action->addToBar = true;
    connect(action, SIGNAL(triggered()), SLOT(sl_showChromatogram()));
    
    sw->addADVSequenceWidgetAction(action);

    // if chromatogram is enabled detailed sequence view is hidden
    sw->setDetViewCollapsed(true);
    
    action->trigger();
}

void ChromaViewContext::sl_showChromatogram() {
    ChromaViewAction* a = qobject_cast<ChromaViewAction*>(sender());
    assert(a!=NULL);
    if (a->isChecked()) {
        assert(a->view == NULL);
        ADVSingleSequenceWidget* sw = qobject_cast<ADVSingleSequenceWidget*>(a->seqWidget);
        DNAChromatogramObject* chromaObj = findChromaObj(sw);
        assert(chromaObj!=NULL);
        a->view = new ChromatogramView(sw, sw->getSequenceContext(), sw->getPanGSLView(), chromaObj->getChromatogram());
        sw->addSequenceView(a->view);
    } else {
        assert(a->view!=NULL);
        GObject* editSeq = a->view->getEditedSequence();
        if (editSeq!=NULL) {
            a->view->getSequenceContext()->getAnnotatedDNAView()->removeObject(editSeq);
        }
        delete a->view;
        a->view = NULL;
    } 
}

ChromaViewAction::ChromaViewAction() : ADVSequenceWidgetAction(CHROMA_ACTION_NAME, tr("show_chromatogram")), view(NULL) 
{
}

ChromaViewAction::~ChromaViewAction() {
    delete view;
}


}//namespace
