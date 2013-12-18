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

#include <U2Core/AppContext.h>
#include <U2Core/DNAAlphabet.h>
#include <U2Core/DataBaseRegistry.h>

#include <U2Core/GAutoDeleteList.h>
#include <U2View/AnnotatedDNAView.h>
#include <U2View/ADVSequenceObjectContext.h>
#include <U2View/ADVConstants.h>
#include <U2View/ADVUtils.h>

#include <U2Core/DNASequenceSelection.h>
#include <U2Core/DNASequenceObject.h>
#include <U2Gui/GUIUtils.h>

#include <U2Test/XMLTestFormat.h>
#include <U2Test/GTest.h>
#include <U2Test/GTestFrameworkComponents.h>

#include <QtGui/QMenu>
#include <QtCore/QCoreApplication>
#include <QtCore/QDir>
#include <QtCore/QDirIterator>
#include <QtGui/QMessageBox>

#include "RemoteBLASTPlugin.h"
#include "BlastQuery.h"
#include "RemoteBLASTTask.h"
#include <U2Algorithm/CDSearchTaskFactoryRegistry.h>


namespace U2 {

extern "C" Q_DECL_EXPORT Plugin * U2_PLUGIN_INIT_FUNC() {
    RemoteBLASTPlugin * plug = new RemoteBLASTPlugin();
    return plug;
}

RemoteBLASTPlugin::RemoteBLASTPlugin():Plugin(tr("Remote BLAST"),tr("Performs remote database queries: BLAST, CDD, etc...")), ctx(NULL) {
    if(AppContext::getMainWindow()) {
        ctx = new RemoteBLASTViewContext(this);
        ctx->init();
    }

    DataBaseRegistry *reg = AppContext::getDataBaseRegistry();
    reg->registerDataBase(new BLASTFactory(),"blastn");
    reg->registerDataBase(new BLASTFactory(),"blastp");
    reg->registerDataBase(new CDDFactory(),"cdd");

    LocalWorkflow::RemoteBLASTWorkerFactory::init();

    QDActorPrototypeRegistry* qdpr = AppContext::getQDActorProtoRegistry();
    qdpr->registerProto(new QDCDDActorPrototype());

    AppContext::getCDSFactoryRegistry()->registerFactory(new RemoteCDSearchFactory(), CDSearchFactoryRegistry::RemoteSearch);

    GTestFormatRegistry *tfr = AppContext::getTestFramework()->getTestFormatRegistry();
    XMLTestFormat *xmlTestFormat = qobject_cast<XMLTestFormat*>(tfr->findFormat("XML"));
    assert(xmlTestFormat!=NULL);

    GAutoDeleteList<XMLTestFactory> *l = new GAutoDeleteList<XMLTestFactory>(this);
    l->qlist = RemoteBLASTPluginTests::createTestFactories();

    foreach(XMLTestFactory *f,l->qlist) {
        bool res = xmlTestFormat->registerTestFactory(f);
        Q_UNUSED(res);
        assert(res);
    }
}

RemoteBLASTViewContext::RemoteBLASTViewContext(QObject *p):GObjectViewWindowContext(p,ANNOTATED_DNA_VIEW_FACTORY_ID)
{
}

void RemoteBLASTViewContext::initViewContext(GObjectView * view) {
    AnnotatedDNAView* av = qobject_cast<AnnotatedDNAView*>(view);
    ADVGlobalAction * a = new ADVGlobalAction(av, QIcon(":/remote_blast/images/remote_db_request.png"), tr("Query NCBI BLAST database..."), 60);
    connect( a, SIGNAL(triggered()), SLOT(sl_showDialog()) );
}

void RemoteBLASTViewContext::sl_showDialog() {
    QAction* a = (QAction*)sender();
    GObjectViewAction* viewAction = qobject_cast<GObjectViewAction*>(a);
    AnnotatedDNAView* av = qobject_cast<AnnotatedDNAView*>(viewAction->getObjectView());
    assert(av);

    ADVSequenceObjectContext* seqCtx = av->getSequenceInFocus();

    bool isAminoSeq = seqCtx->getAlphabet()->isAmino();
    SendSelectionDialog dlg( seqCtx->getSequenceObject(), isAminoSeq, av->getWidget() );
    if( QDialog::Accepted == dlg.exec() ) {
        //prepare query
        DNASequenceSelection* s = seqCtx->getSequenceSelection();
        QVector<U2Region> regions;
        if (s->isEmpty()) {
            regions.append(U2Region(0, seqCtx->getSequenceLength()));
        } else {
            regions =  s->getSelectedRegions();
        }
        foreach(const U2Region& r, regions) {
            QByteArray query = seqCtx->getSequenceData(r);

            DNATranslation * aminoT = (dlg.translateToAmino ? seqCtx->getAminoTT() : 0);
            DNATranslation * complT = (dlg.translateToAmino ? seqCtx->getComplementTT() : 0);

            RemoteBLASTTaskSettings cfg = dlg.cfg;
            cfg.query = query;
            cfg.aminoT = aminoT;
            cfg.complT = complT;

            FeaturesTableObject *aobject = dlg.getAnnotationObject();
            if (aobject == NULL){
                return;
            }
            Task * t = new RemoteBLASTToAnnotationsTask(cfg, r.startPos, aobject, dlg.getUrl(),dlg.getGroupName());
            AppContext::getTaskScheduler()->registerTopLevelTask( t );
        }
    }
}

QList<XMLTestFactory*> RemoteBLASTPluginTests::createTestFactories() {
    QList<XMLTestFactory*> res;
    res.append(GTest_RemoteBLAST::createFactory());
    return res;
}

}
