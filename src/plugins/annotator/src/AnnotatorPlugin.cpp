/**
 * UGENE - Integrated Bioinformatics Tools.
 * Copyright (C) 2008-2013 UniPro <ugene@unipro.ru>
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

#include "AnnotatorPlugin.h"
#include "CollocationsDialogController.h"

#include "AnnotatorTests.h"

#include <U2Core/GAutoDeleteList.h>
#include <U2Gui/GUIUtils.h>
#include <U2View/AnnotatedDNAView.h>
#include <U2View/ADVConstants.h>
#include <U2View/ADVUtils.h>
#include <U2Core/AnnotationTableObject.h>

#include <QtGui/QMenu>
#include <QtGui/QMessageBox>

#include <U2Test/XMLTestFormat.h>
#include <U2Test/GTest.h>
#include <U2Test/GTestFrameworkComponents.h>

#include <U2Core/AppContext.h>

#include "CollocationWorker.h"
#include "GeneByGeneReportWorker.h"
#include "CustomPatternAnnotationTask.h"

namespace U2 {


extern "C" Q_DECL_EXPORT Plugin* U2_PLUGIN_INIT_FUNC() {
    AnnotatorPlugin * plug = new AnnotatorPlugin();
    return plug;
}

#define PLASMID_FEATURES_GROUP_NAME "plasmid_features"

AnnotatorPlugin::AnnotatorPlugin() : Plugin(tr("dna_annotator_plugin"), tr("dna_annotator_plugin_desc")), viewCtx(NULL)
{
    if (AppContext::getMainWindow()) {
        viewCtx = new AnnotatorViewContext(this);
        viewCtx->init();

        QString customAnnotationDir = QDir::searchPaths( PATH_PREFIX_DATA ).first() + "/custom_annotations";
        QString plasmidFeaturesPath = customAnnotationDir + "/plasmid_features.txt";
        SharedFeatureStore store( new FeatureStore(PLASMID_FEATURES_GROUP_NAME, plasmidFeaturesPath));
        if (store->load()) {
            CustomPatternAutoAnnotationUpdater* aaUpdater = new CustomPatternAutoAnnotationUpdater(store);
            AppContext::getAutoAnnotationsSupport()->registerAutoAnnotationsUpdater(aaUpdater);
        }

    }
    LocalWorkflow::CollocationWorkerFactory::init();
    LocalWorkflow::GeneByGeneReportWorkerFactory::init();

    //Annotator test
    GTestFormatRegistry* tfr = AppContext::getTestFramework()->getTestFormatRegistry();
    XMLTestFormat *xmlTestFormat = qobject_cast<XMLTestFormat*>(tfr->findFormat("XML"));
    assert(xmlTestFormat!=NULL);

    GAutoDeleteList<XMLTestFactory>* l = new GAutoDeleteList<XMLTestFactory>(this);
    l->qlist = AnnotatorTests::createTestFactories();


    foreach(XMLTestFactory* f, l->qlist) {
        bool res = xmlTestFormat->registerTestFactory(f);
        Q_UNUSED(res);
        assert(res);
    }
}

AnnotatorViewContext::AnnotatorViewContext(QObject* p) : GObjectViewWindowContext(p, ANNOTATED_DNA_VIEW_FACTORY_ID) {

}

void AnnotatorViewContext::initViewContext(GObjectView* v) {
    AnnotatedDNAView* av = qobject_cast<AnnotatedDNAView*>(v);
    ADVGlobalAction* a = new ADVGlobalAction(av, QIcon(":annotator/images/regions.png"), tr("Find annotated regions..."), 30);
    connect(a, SIGNAL(triggered()), SLOT(sl_showCollocationDialog()));
}

void AnnotatorViewContext::sl_showCollocationDialog() {
    QAction* a = (QAction*)sender();
    GObjectViewAction* viewAction = qobject_cast<GObjectViewAction*>(a);
    AnnotatedDNAView* av = qobject_cast<AnnotatedDNAView*>(viewAction->getObjectView());
    assert(av);

    QSet<QString> allNames;

    foreach ( AnnotationTableObject *ao, av->getAnnotationObjects()) {
        foreach ( const Annotation &a, ao->getAnnotations( ) ) {
            allNames.insert( a.getName( ) );
        }
    }
    if (allNames.isEmpty()) {
        QMessageBox::warning(av->getWidget(), tr("warning"),tr("no_annotations_found"));
        return;
    }
        
    ADVSequenceObjectContext* seqCtx = av->getSequenceInFocus();
    if (seqCtx == NULL) { 
        return;
    }
    CollocationsDialogController d(allNames.toList(), seqCtx);
    d.exec();
}

QList<XMLTestFactory*> AnnotatorTests::createTestFactories() {
    QList<XMLTestFactory*> res;
    res.append(GTest_AnnotatorSearch::createFactory());
    res.append(GTest_GeneByGeneApproach::createFactory());
    return res;
}

}//namespace
