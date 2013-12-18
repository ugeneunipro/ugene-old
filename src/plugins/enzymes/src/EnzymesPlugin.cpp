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

#include "EnzymesPlugin.h"
#include "FindEnzymesTask.h"
#include "FindEnzymesDialog.h"
#include "ConstructMoleculeDialog.h"
#include "DigestSequenceDialog.h"
#include "CreateFragmentDialog.h"
#include "EnzymesTests.h"
#include "EnzymesQuery.h"

#include <U2Core/AppContext.h>
#include <U2Gui/MainWindow.h>
#include <U2Core/DNAAlphabet.h>
#include <U2Core/GAutoDeleteList.h>
#include <U2Core/AutoAnnotationsSupport.h>
#include <U2Core/AnnotationSelection.h>

#include <U2Gui/GUIUtils.h>
#include <U2Gui/DialogUtils.h>

#include <U2View/ADVConstants.h>
#include <U2View/AnnotatedDNAView.h>
#include <U2View/ADVSequenceObjectContext.h>
#include <U2View/ADVSequenceWidget.h>
#include <U2View/ADVUtils.h>


#include <U2Test/GTestFrameworkComponents.h>

#include <QtCore/QDir>
#include <QtGui/QMenu>
#include <QtGui/QMessageBox>

namespace U2 {

extern "C" Q_DECL_EXPORT Plugin * U2_PLUGIN_INIT_FUNC() {
    EnzymesPlugin * plug = new EnzymesPlugin();
    return plug;
}

EnzymesPlugin::EnzymesPlugin() : Plugin( tr("Restriction analysis"), 
                                        tr("Finds and annotates restriction sites on a DNA sequence.") ),
                                        ctxADV(NULL)
{
    if (AppContext::getMainWindow()) {
        createToolsMenu();     
        
        QList<QAction*> actions;
        actions.append(openDigestSequenceDialog);
        actions.append(openConstructMoleculeDialog);
        actions.append(openCreateFragmentDialog);
        
        ctxADV = new EnzymesADVContext(this,actions);
        ctxADV->init();

        AppContext::getAutoAnnotationsSupport()->registerAutoAnnotationsUpdater(new FindEnzymesAutoAnnotationUpdater() );

    }

    EnzymesSelectorWidget::setupSettings();
    FindEnzymesDialog::initDefaultSettings();

    GTestFormatRegistry* tfr = AppContext::getTestFramework()->getTestFormatRegistry();
    XMLTestFormat *xmlTestFormat = qobject_cast<XMLTestFormat*>(tfr->findFormat("XML"));
    assert(xmlTestFormat!=NULL);

    QDActorPrototypeRegistry* qdpr = AppContext::getQDActorProtoRegistry();
    qdpr->registerProto(new QDEnzymesActorPrototype());

    GAutoDeleteList<XMLTestFactory>* l = new GAutoDeleteList<XMLTestFactory>(this);
    l->qlist = EnzymeTests::createTestFactories();

    foreach(XMLTestFactory* f, l->qlist) { 
        bool res = xmlTestFormat->registerTestFactory(f);
        assert(res);
        Q_UNUSED(res);
    }
}

void EnzymesPlugin::createToolsMenu()
{
    openDigestSequenceDialog = new QAction(tr("Digest into Fragments..."), this);
    openDigestSequenceDialog->setObjectName("Digest into Fragments");
    openConstructMoleculeDialog = new QAction(tr("Construct Molecule..."), this);
    openConstructMoleculeDialog->setObjectName("Construct Molecule");
    openCreateFragmentDialog = new QAction(tr("Create Fragment..."), this);
    openCreateFragmentDialog->setObjectName("Create Fragment");
    
    QMenu* tools = AppContext::getMainWindow()->getTopLevelMenu(MWMENU_TOOLS);
    
    QMenu* cloningMenu = tools->addMenu(QIcon(":core/images/dna_helix.png"), tr("Cloning"));
    cloningMenu->menuAction()->setObjectName("Cloning");
    
    cloningMenu->addAction(openDigestSequenceDialog);
    connect(openDigestSequenceDialog, SIGNAL(triggered()), SLOT(sl_onOpenDigestSequenceDialog()));   
    
    cloningMenu->addAction(openConstructMoleculeDialog);
    connect(openConstructMoleculeDialog, SIGNAL(triggered()), SLOT(sl_onOpenConstructMoleculeDialog()));

    connect(openCreateFragmentDialog, SIGNAL(triggered()), SLOT(sl_onOpenCreateFragmentDialog()));

}

void EnzymesPlugin::sl_onOpenDigestSequenceDialog()
{
    GObjectViewWindow* w = GObjectViewUtils::getActiveObjectViewWindow();
    
    if (w == NULL) {
        QMessageBox::information(QApplication::activeWindow(), openDigestSequenceDialog->text(),
            tr("There is no active sequence object.\nTo start partition open sequence document.") );
        return;
    }

    AnnotatedDNAView* view = qobject_cast<AnnotatedDNAView*>(w->getObjectView());
    if (view == NULL) {
        QMessageBox::information(QApplication::activeWindow(), openDigestSequenceDialog->text(),
            tr("There is no active sequence object.\nTo start partition open sequence document.") );
        return;

    }

    if (!view->getSequenceInFocus()->getSequenceObject()->getAlphabet()->isNucleic()) {
        QMessageBox::information(QApplication::activeWindow(), openDigestSequenceDialog->text(),
            tr("Can not digest into fragments non-nucleic sequence.") );
        return;

    }

    DigestSequenceDialog dlg(view->getSequenceInFocus(), QApplication::activeWindow());
    dlg.exec();
        
    
}


void EnzymesPlugin::sl_onOpenCreateFragmentDialog()
{
    GObjectViewWindow* w = GObjectViewUtils::getActiveObjectViewWindow();

    if (w == NULL) {
        QMessageBox::information(QApplication::activeWindow(), openCreateFragmentDialog->text(),
            tr("There is no active sequence object.\nTo create fragment open sequence document.") );
        return;
    }

    AnnotatedDNAView* view = qobject_cast<AnnotatedDNAView*>(w->getObjectView());
    if (view == NULL) {
        QMessageBox::information(QApplication::activeWindow(), openCreateFragmentDialog->text(),
            tr("There is no active sequence object.\nTo create fragment open sequence document.") );
        return;

    }

    U2SequenceObject* dnaObj = view->getSequenceInFocus()->getSequenceObject();
    assert(dnaObj != NULL);
    if (!dnaObj->getAlphabet()->isNucleic()) {
        QMessageBox::information(QApplication::activeWindow(), openCreateFragmentDialog->text(),
            tr("The sequence doesn't have nucleic alphabet, it can not be used in cloning.") );
        return;
    }

    CreateFragmentDialog dlg(view->getSequenceInFocus(), QApplication::activeWindow());
    dlg.exec();
}



void EnzymesPlugin::sl_onOpenConstructMoleculeDialog()
{
    Project* p = AppContext::getProject();
    if (p == NULL) {
        QMessageBox::information(QApplication::activeWindow(), openConstructMoleculeDialog->text(),
            tr("There is no active project.\nTo start ligation create a project or open an existing.") );
        return;
    }

    QList<DNAFragment> fragments = DNAFragment::findAvailableFragments();

    ConstructMoleculeDialog dlg(fragments, QApplication::activeWindow());
    dlg.exec();
}

EnzymesPlugin::~EnzymesPlugin()
{

}

//////////////////////////////////////////////////////////////////////////


EnzymesADVContext::EnzymesADVContext(QObject* p,const QList<QAction*>& actions) : GObjectViewWindowContext(p, ANNOTATED_DNA_VIEW_FACTORY_ID), cloningActions(actions),createPCRProductAction(NULL)
{
}

void EnzymesADVContext::initViewContext(GObjectView* view) {
    AnnotatedDNAView* av = qobject_cast<AnnotatedDNAView*>(view);
    ADVGlobalAction* a = new ADVGlobalAction(av, QIcon(":enzymes/images/enzymes.png"), tr("Find restriction sites..."), 50);
    a->addAlphabetFilter(DNAAlphabet_NUCL);
    connect(a, SIGNAL(triggered()), SLOT(sl_search()));
    
    createPCRProductAction = new GObjectViewAction(av, av, "Create PCR product...");
    connect(createPCRProductAction, SIGNAL(triggered()), SLOT(sl_createPCRProduct()));

}


void EnzymesADVContext::sl_search() {
    GObjectViewAction* action = qobject_cast<GObjectViewAction*>(sender());
    assert(action!=NULL);
    AnnotatedDNAView* av = qobject_cast<AnnotatedDNAView*>(action->getObjectView());
    assert(av!=NULL);

    ADVSequenceObjectContext* seqCtx = av->getSequenceInFocus();
    assert(seqCtx->getAlphabet()->isNucleic());
    FindEnzymesDialog d(seqCtx);
    d.exec();
}

// TODO: move definitions to core
#define PRIMER_ANNOTATION_GROUP_NAME    "pair"
#define PRIMER_ANNOTATION_NAME			"primer"


void EnzymesADVContext::buildMenu( GObjectView* v, QMenu* m )
{
    
    AnnotatedDNAView* av = qobject_cast<AnnotatedDNAView*>(v);
    assert(av!=NULL);
    if (!av->getSequenceInFocus()->getAlphabet()->isNucleic()) {
        return;
    }
        
    QMenu* cloningMenu = new QMenu(tr("Cloning"), m);
    cloningMenu->menuAction()->setObjectName("Cloning");
    cloningMenu->addActions(cloningActions);
    
    QAction* exportMenuAction = GUIUtils::findAction(m->actions(), ADV_MENU_EXPORT);
    m->insertMenu(exportMenuAction, cloningMenu);

    if(!av->getAnnotationsSelection()->getSelection().isEmpty()) {
        QString annName = av->getAnnotationsSelection()->getSelection().first().annotation.getName();
        
        if (annName == PRIMER_ANNOTATION_NAME) {
            cloningMenu->addAction( createPCRProductAction );
        }
    }
}

void EnzymesADVContext::sl_createPCRProduct()
{
    GObjectViewAction* action = qobject_cast<GObjectViewAction*>(sender());
    SAFE_POINT( action != NULL, "Invalid action object!", );
    AnnotatedDNAView* av = qobject_cast<AnnotatedDNAView*>(action->getObjectView());
    SAFE_POINT( av != NULL, "Invalid DNA view!", );

    const Annotation a = av->getAnnotationsSelection()->getSelection().first().annotation;
    const AnnotationGroup group = a.getGroup();
    if (group.getName().startsWith(PRIMER_ANNOTATION_GROUP_NAME) ) {
        SAFE_POINT( group.getAnnotations( ).size( ) == 2, "Invalid selected annotation count!", );

        const Annotation a1 = group.getAnnotations().at(0);
        const Annotation a2 = group.getAnnotations().at(1);
        int startPos = a1.getLocation()->regions.at(0).startPos;
        SAFE_POINT( a2.getLocation( )->strand == U2Strand::Complementary, "Invalid annotation's strand!", );
        int endPos = a2.getLocation()->regions.at(0).endPos();

        U2SequenceObject* seqObj = av->getSequenceInFocus()->getSequenceObject();
        U2Region region(startPos, endPos - startPos );
        CreateFragmentDialog dlg(seqObj, region, av->getSequenceWidgetInFocus() );
        dlg.setWindowTitle("Create PCR product");
        dlg.exec();
    }
}

} //namespace
