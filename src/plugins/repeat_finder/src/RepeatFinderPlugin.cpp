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

#include "RepeatFinderPlugin.h"
#include "RepeatFinderTests.h"
#include "RepeatWorker.h"
#include "FindRepeatsDialog.h"
#include "FindTandemsDialog.h"
#include "RepeatQuery.h"
#include "TandemQuery.h"
#include "RFTaskFactory.h"


#include <U2Core/DNAAlphabet.h>
#include <U2Algorithm/RepeatFinderTaskFactoryRegistry.h>
#include <U2Gui/GUIUtils.h>
#include <U2View/AnnotatedDNAViewFactory.h>
#include <U2View/AnnotatedDNAView.h>
#include <U2View/ADVConstants.h>
#include <U2View/ADVSequenceObjectContext.h>
#include <U2View/ADVUtils.h>
#include <U2Core/GAutoDeleteList.h>
#include <U2Lang/QueryDesignerRegistry.h>

#include <U2Test/XMLTestFormat.h>
#include <U2Test/GTestFrameworkComponents.h>

#if (QT_VERSION < 0x050000) //Qt 5
#include <QtGui/QAction>
#include <QtGui/QMenu>
#else
#include <QtWidgets/QAction>
#include <QtWidgets/QMenu>
#endif

#include <cstdio>

namespace U2 {

extern "C" Q_DECL_EXPORT Plugin* U2_PLUGIN_INIT_FUNC() {
    RepeatFinderPlugin* plug = new RepeatFinderPlugin();
    return plug;
}

RepeatFinderPlugin::RepeatFinderPlugin() : Plugin(tr("Repeats Finder"), tr("Search for repeated elements in genetic sequences")), viewCtx(NULL) {
    if (AppContext::getMainWindow()) {
        viewCtx = new RepeatViewContext(this);
        viewCtx->init();
    }
    LocalWorkflow::RepeatWorkerFactory::init();

    QDActorPrototypeRegistry* pr = AppContext::getQDActorProtoRegistry();
    pr->registerProto(new QDRepeatActorPrototype());
    pr->registerProto(new QDTandemActorPrototype());

    //tests
    GTestFormatRegistry* tfr = AppContext::getTestFramework()->getTestFormatRegistry();
    XMLTestFormat *xmlTestFormat = qobject_cast<XMLTestFormat*>(tfr->findFormat("XML"));
    assert(xmlTestFormat!=NULL);

    GAutoDeleteList<XMLTestFactory>* l = new GAutoDeleteList<XMLTestFactory>(this);
    l->qlist = RepeatFinderTests::createTestFactories();

    foreach(XMLTestFactory* f, l->qlist) {
        bool res = xmlTestFormat->registerTestFactory(f);
        assert(res); Q_UNUSED(res);
    }

    RepeatFinderTaskFactoryRegistry *rfTfr = AppContext::getRepeatFinderTaskFactoryRegistry();
    Q_ASSERT(rfTfr);
    rfTfr->registerFactory(new RFTaskFactory(), "");
}

RepeatViewContext::RepeatViewContext(QObject* p) :
GObjectViewWindowContext(p, AnnotatedDNAViewFactory::ID)
{
}

void RepeatViewContext::initViewContext(GObjectView* v) {
    AnnotatedDNAView* av = qobject_cast<AnnotatedDNAView*>(v);
    ADVGlobalAction* a = new ADVGlobalAction(av, QIcon(":repeat_finder/images/repeats.png"), tr("Find repeats..."), 40);
    a->addAlphabetFilter(DNAAlphabet_NUCL);
    a->setObjectName("find_repeats_action");
    connect(a, SIGNAL(triggered()), SLOT(sl_showDialog()));
    ADVGlobalAction* a2 = new ADVGlobalAction(av, QIcon(":repeat_finder/images/repeats_tandem.png"), tr("Find tandems..."), 41);
    a2->addAlphabetFilter(DNAAlphabet_NUCL);
    a2->setObjectName("find_tandems_action");
    connect(a2, SIGNAL(triggered()), SLOT(sl_showTandemDialog()));
}

void RepeatViewContext::sl_showDialog() {
    QAction* a = (QAction*)sender();
    GObjectViewAction* viewAction = qobject_cast<GObjectViewAction*>(a);
    AnnotatedDNAView* av = qobject_cast<AnnotatedDNAView*>(viewAction->getObjectView());
    ADVSequenceObjectContext* sctx = av->getSequenceInFocus();
    assert(sctx!=NULL && sctx->getAlphabet()->isNucleic());
    FindRepeatsDialog d(sctx);
    d.exec();
}

void RepeatViewContext::sl_showTandemDialog() {
    QAction* a = (QAction*)sender();
    GObjectViewAction* viewAction = qobject_cast<GObjectViewAction*>(a);
    AnnotatedDNAView* av = qobject_cast<AnnotatedDNAView*>(viewAction->getObjectView());
    ADVSequenceObjectContext* sctx = av->getSequenceInFocus();
    assert(sctx!=NULL && sctx->getAlphabet()->isNucleic());
    FindTandemsDialog d(sctx);
    d.exec();
}

}//namespace
