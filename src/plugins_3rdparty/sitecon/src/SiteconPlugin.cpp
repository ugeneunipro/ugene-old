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

#include "SiteconPlugin.h"
#include "SiteconBuildDialogController.h"
#include "SiteconSearchDialogController.h"
#include "SiteconWorkers.h"
#include "SiteconAlgorithmTests.h"
#include "DIPropertiesTests.h"
#include "SiteconIO.h"
#include "SiteconQuery.h"

#include <U2Core/DNAAlphabet.h>
#include <U2Core/DNASequenceObject.h>

#include <U2Core/GAutoDeleteList.h>

#include <U2View/AnnotatedDNAView.h>
#include <U2View/ADVSequenceObjectContext.h>
#include <U2View/ADVConstants.h>
#include <U2View/ADVUtils.h>

#include <U2Gui/LastUsedDirHelper.h>
#include <U2Gui/GUIUtils.h>

#include <U2Test/XMLTestFormat.h>
#include <U2Test/GTest.h>
#include <U2Test/GTestFrameworkComponents.h>

#include <U2Core/AppContext.h>

namespace U2 {

DinucleotitePropertyRegistry SiteconPlugin::dp;

extern "C" Q_DECL_EXPORT Plugin* U2_PLUGIN_INIT_FUNC() {
    SiteconPlugin* plug = new SiteconPlugin();
    return plug;
}

    
SiteconPlugin::SiteconPlugin() : Plugin(tr("sitecon_plugin"), tr("sitecon_plugin_desc")), ctxADV(NULL)
{
    if (AppContext::getMainWindow()) {
        ctxADV = new SiteconADVContext(this);
        ctxADV->init();

        QAction* buildAction = new QAction(tr("sitecon_build"), this);
        connect(buildAction, SIGNAL(triggered()), SLOT(sl_build()));
        
        QMenu* tools = AppContext::getMainWindow()->getTopLevelMenu(MWMENU_TOOLS);
        QMenu* toolsSubmenu = tools->addMenu(QIcon(":/sitecon/images/sitecon.png"), tr("sitecon_menu"));

        toolsSubmenu->addAction(buildAction);
    }

    LocalWorkflow::SiteconWorkerFactory::init();
    GTestFormatRegistry* tfr = AppContext::getTestFramework()->getTestFormatRegistry();
    XMLTestFormat *xmlTestFormat = qobject_cast<XMLTestFormat*>(tfr->findFormat("XML"));
    assert(xmlTestFormat!=NULL);

    QString defaultDir = QDir::searchPaths( PATH_PREFIX_DATA ).first() + "/sitecon_models";

    if (LastUsedDirHelper::getLastUsedDir(SiteconIO::SITECON_ID).isEmpty()) {
        LastUsedDirHelper::setLastUsedDir(defaultDir, SiteconIO::SITECON_ID);
    }

    QDActorPrototypeRegistry* qpfr = AppContext::getQDActorProtoRegistry();
    assert(qpfr);
    qpfr->registerProto(new QDSiteconActorPrototype());
    
    GAutoDeleteList<XMLTestFactory>* l = new GAutoDeleteList<XMLTestFactory>(this);
    l->qlist = SiteconAlgorithmTests::createTestFactories();

    foreach(XMLTestFactory* f, l->qlist) { 
        bool res = xmlTestFormat->registerTestFactory(f);
        Q_UNUSED(res);
        assert(res);
    }
}

SiteconPlugin::~SiteconPlugin() {
}


void SiteconPlugin::sl_build() {
    QWidget *p = (QWidget*)(AppContext::getMainWindow()->getQMainWindow());
    SiteconBuildDialogController d(this, p);
    d.exec();
}

void SiteconPlugin::sl_search() {
    assert(NULL);
}



SiteconADVContext::SiteconADVContext(QObject* p) : GObjectViewWindowContext(p, ANNOTATED_DNA_VIEW_FACTORY_ID)
{

}

void SiteconADVContext::initViewContext(GObjectView* view) {
    AnnotatedDNAView* av = qobject_cast<AnnotatedDNAView*>(view);
    ADVGlobalAction* a = new ADVGlobalAction(av, QIcon(":sitecon/images/sitecon.png"), tr("Search TFBS with SITECON..."), 80);
    a->addAlphabetFilter(DNAAlphabet_NUCL);
    connect(a, SIGNAL(triggered()), SLOT(sl_search()));
}

void SiteconADVContext::sl_search() {
    GObjectViewAction* action = qobject_cast<GObjectViewAction*>(sender());
    AnnotatedDNAView* av = qobject_cast<AnnotatedDNAView*>(action->getObjectView());
    
    ADVSequenceObjectContext* seqCtx = av->getSequenceInFocus();
    assert(seqCtx->getAlphabet()->isNucleic());
    SiteconSearchDialogController d(seqCtx, av->getWidget());
    d.exec();
}

QList<XMLTestFactory*> SiteconAlgorithmTests::createTestFactories() {
    QList<XMLTestFactory*> res;
    res.append(GTest_CalculateACGTContent::createFactory());
    res.append(GTest_CalculateDispersionAndAverage::createFactory());
    res.append(GTest_DiPropertySiteconCheckAttribs::createFactory());
    res.append(GTest_CalculateFirstTypeError::createFactory());
    res.append(GTest_CalculateSecondTypeError::createFactory());
    res.append(GTest_SiteconSearchTask::createFactory());
    res.append(GTest_CompareSiteconModels::createFactory());
    return res;
}


}//namespace

