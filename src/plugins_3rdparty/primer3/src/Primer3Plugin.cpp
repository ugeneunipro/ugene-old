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

#include "Primer3Plugin.h"
#include "Primer3Dialog.h"
#include "Primer3Query.h"

#include <U2Core/GAutoDeleteList.h>
#include <U2Gui/GUIUtils.h>

#include <U2View/AnnotatedDNAView.h>
#include <U2View/ADVConstants.h>
#include <U2View/ADVSequenceObjectContext.h>
#include <U2View/ADVUtils.h>

#include <QtGui/QMenu>
#include <QtCore/QMap>
#include <QtGui/QAction>
#include <QtGui/QMessageBox>

#include <U2Core/AppContext.h>

#include <U2Test/XMLTestFormat.h>
#include <U2Test/GTest.h>
#include <U2Test/GTestFrameworkComponents.h>

namespace U2 {


extern "C" Q_DECL_EXPORT Plugin* U2_PLUGIN_INIT_FUNC() {
    Primer3Plugin * plug = new Primer3Plugin();
    return plug;
}

Primer3Plugin::Primer3Plugin() : Plugin(tr("Primer3"), tr("Integrated tool for PCR primers design.")), viewCtx(NULL)
{
    if (AppContext::getMainWindow()) {
        viewCtx = new Primer3ADVContext(this);
        viewCtx->init();
    }

    QDActorPrototypeRegistry* qdpr = AppContext::getQDActorProtoRegistry();
    qdpr->registerProto(new QDPrimerActorPrototype());
    
    //////////////////////////////////////////////////////////////////////////
    //tests
    GTestFormatRegistry* tfr = AppContext::getTestFramework()->getTestFormatRegistry();
    XMLTestFormat *xmlTestFormat = qobject_cast<XMLTestFormat*>(tfr->findFormat("XML"));
    assert(xmlTestFormat!=NULL);

    GAutoDeleteList<XMLTestFactory>* l = new GAutoDeleteList<XMLTestFactory>(this);
    l->qlist = Primer3Tests::createTestFactories();


    foreach(XMLTestFactory* f, l->qlist) { 
        bool res = xmlTestFormat->registerTestFactory(f);
        Q_UNUSED(res);
        assert(res);
    }
}

Primer3Plugin::~Primer3Plugin() {
}

Primer3ADVContext::Primer3ADVContext(QObject* p) : GObjectViewWindowContext(p, ANNOTATED_DNA_VIEW_FACTORY_ID)
{

}

void Primer3ADVContext::initViewContext(GObjectView* v) {
    AnnotatedDNAView* av = qobject_cast<AnnotatedDNAView*>(v);
    ADVGlobalAction* a = new ADVGlobalAction(av, QIcon(":/primer3/images/primer3.png"), tr("Primer3..."), 95);
    a->addAlphabetFilter(DNAAlphabet_NUCL);
    connect(a, SIGNAL(triggered()), SLOT(sl_showDialog()));
}

void Primer3ADVContext::sl_showDialog() {
    QAction* a = (QAction*)sender();
    GObjectViewAction* viewAction = qobject_cast<GObjectViewAction*>(a);
    AnnotatedDNAView* av = qobject_cast<AnnotatedDNAView*>(viewAction->getObjectView());
    assert(av);

    ADVSequenceObjectContext* seqCtx = av->getSequenceInFocus();
    assert(seqCtx->getAlphabet()->isNucleic());
    {
        Primer3TaskSettings defaultSettings;
        {
            QList<QPair<int, int> > sizeRange;
            sizeRange.append(qMakePair(150,250));
            sizeRange.append(qMakePair(100,300));
            sizeRange.append(qMakePair(301,400));
            sizeRange.append(qMakePair(401,500));
            sizeRange.append(qMakePair(501,600));
            sizeRange.append(qMakePair(601,700));
            sizeRange.append(qMakePair(701,850));
            sizeRange.append(qMakePair(851,1000));
            defaultSettings.setProductSizeRange(sizeRange);
        }
        defaultSettings.setDoubleProperty("PRIMER_MAX_END_STABILITY",9.0);
        defaultSettings.setAlignProperty("PRIMER_MAX_TEMPLATE_MISPRIMING",1200);
        defaultSettings.setAlignProperty("PRIMER_PAIR_MAX_TEMPLATE_MISPRIMING",2400);
        defaultSettings.setIntProperty("PRIMER_LIBERAL_BASE",1);
        defaultSettings.setDoubleProperty("PRIMER_WT_POS_PENALTY",0.0);
        defaultSettings.setIntProperty("PRIMER_FIRST_BASE_INDEX",1);
        Primer3Dialog dialog(defaultSettings, seqCtx);
        if(QDialog::Accepted == dialog.exec())
        {
            Primer3TaskSettings settings = dialog.getSettings();
            settings.setSequence(seqCtx->getSequenceObject()->getWholeSequenceData());
            U2Region region=dialog.getRegion();
            settings.setIncludedRegion(qMakePair(
                    (int)region.startPos + settings.getFirstBaseIndex(),
                    (int)region.length));//need refactor primer3 move to U2Region
            QString err = dialog.checkModel();
            if (!err.isEmpty()) {
                QMessageBox::warning(QApplication::activeWindow(), dialog.windowTitle(), err);
                return;
            }
            bool objectPrepared = dialog.prepareAnnotationObject();;
            if (!objectPrepared){
                QMessageBox::warning(QApplication::activeWindow(), tr("Error"), tr("Cannot create an annotation object. Please check settings"));
                return;
            }
            const CreateAnnotationModel &model = dialog.getCreateAnnotationModel();
            AppContext::getTaskScheduler()->registerTopLevelTask(
                new Primer3ToAnnotationsTask(settings, seqCtx->getSequenceObject(), model.getAnnotationObject(), model.groupName, ""));

        }
    }
}

QList<XMLTestFactory*> Primer3Tests::createTestFactories() {
    QList<XMLTestFactory*> res;
    res.append(GTest_Primer3::createFactory());
    return res;
}

}//namespace
