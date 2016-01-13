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

#include <QAction>
#include <QMenu>
#include <QMessageBox>

#include <U2Core/AppContext.h>
#include <U2Core/GAutoDeleteList.h>
#include <U2Core/GObjectSelection.h>
#include <U2Core/MAlignmentObject.h>

#include <U2Gui/GUIUtils.h>
#include <U2Gui/MainWindow.h>
#include <U2Gui/ObjectViewModel.h>
#include <U2Gui/ProjectView.h>
#include <U2Gui/ToolsMenu.h>
#include <U2Core/QObjectScopedPointer.h>

#include <U2View/ADVConstants.h>
#include <U2View/ADVSequenceObjectContext.h>
#include <U2View/ADVUtils.h>
#include <U2View/AnnotatedDNAView.h>
#include <U2View/MSAEditor.h>
#include <U2View/MSAEditorFactory.h>

#include <U2Test/GTestFrameworkComponents.h>
#include <U2Test/XMLTestFormat.h>

#include "uHMM3Plugin.h"
#include "build/uHMM3BuildDialogImpl.h"
#include "format/uHMMFormat.h"
#include "phmmer/uHMM3PhmmerDialogImpl.h"
#include "search/uHMM3SearchDialogImpl.h"
#include "search/uhmm3QDActor.h"
#include "tests/uhmmer3Tests.h"
#include "workers/HMM3IOWorker.h"

Q_DECLARE_METATYPE(QMenu*);

namespace U2 {

/****************************************
 * UHMM3Plugin
 ****************************************/

extern "C" Q_DECL_EXPORT Plugin* U2_PLUGIN_INIT_FUNC() {
    UHMM3Plugin * plug = new UHMM3Plugin();
    return plug;
}

UHMM3Plugin::UHMM3Plugin() : Plugin( tr( "HMM3" ), tr( "HMM profile tools. Plugin is based on HMMER 3.0b3 package: freely distributable implementation of profile HMM software for protein sequence analysis. Home page of project: http://hmmer.janelia.org/" ) ) {
    // UHMMFormat
    DocumentFormatRegistry* dfRegistry = AppContext::getDocumentFormatRegistry();
    assert( NULL != dfRegistry );
    bool ok = dfRegistry->registerFormat( new UHMMFormat( dfRegistry ) );
    assert( ok ); Q_UNUSED( ok );

    LocalWorkflow::HMM3Lib::init();

    QDActorPrototypeRegistry* qdpr = AppContext::getQDActorProtoRegistry();
    qdpr->registerProto(new UHMM3QDActorPrototype());
    
    // Tests
    GTestFormatRegistry* tfr = AppContext::getTestFramework()->getTestFormatRegistry();
    assert( NULL != tfr );
    XMLTestFormat *xmlTestFormat = qobject_cast<XMLTestFormat*>(tfr->findFormat("XML"));
    assert( NULL != xmlTestFormat );
    
    GAutoDeleteList<XMLTestFactory>* l = new GAutoDeleteList<XMLTestFactory>(this);
    l->qlist = UHMMER3Tests::createTestFactories();
    
    foreach( XMLTestFactory* f, l->qlist ) { 
        bool res = xmlTestFormat->registerTestFactory( f );
        assert(res);
        Q_UNUSED(res);
    }
    
    // HMMER3 menu
    MainWindow * mainWnd = AppContext::getMainWindow();
    if( mainWnd ) {  
        QAction * buildAction = new QAction( tr( "Build HMM3 profile..." ), this );
        buildAction->setObjectName(ToolsMenu::HMMER_BUILD3);
        connect( buildAction, SIGNAL( triggered() ), SLOT( sl_buildProfile() ) );
        ToolsMenu::addAction(ToolsMenu::HMMER_MENU, buildAction);
        
        QAction * searchAction = new QAction( tr( "Search with HMMER3..." ), this );
        searchAction->setObjectName(ToolsMenu::HMMER_SEARCH3);
        connect( searchAction, SIGNAL( triggered() ), SLOT( sl_searchHMMSignals() ) );
        ToolsMenu::addAction(ToolsMenu::HMMER_MENU, searchAction);
        
        QAction * phmmerAction = new QAction( tr( "Search with HMMER3 phmmer..." ), this );
        phmmerAction->setObjectName(ToolsMenu::HMMER_SEARCH3P);
        connect( phmmerAction, SIGNAL( triggered() ), SLOT( sl_phmmerSearch() ) );
        ToolsMenu::addAction(ToolsMenu::HMMER_MENU, phmmerAction);
        
        // contexts
        msaEditorCtx = new UHMM3MSAEditorContext( this );
        msaEditorCtx->init();
        
        advCtx = new UHMM3ADVContext( this );
        advCtx->init();
    }    
}

void UHMM3Plugin::sl_buildProfile() {
    MAlignment ma;
    MWMDIWindow* w = AppContext::getMainWindow()->getMDIManager()->getActiveWindow();
    if( NULL != w ) {
        GObjectViewWindow* ow = qobject_cast< GObjectViewWindow* >( w );
        if( NULL != ow ) {
            GObjectView* ov = ow->getObjectView();
            MSAEditor * av = qobject_cast< MSAEditor* >( ov );
            if( NULL != av ) {
                MAlignmentObject* maObj = av->getMSAObject();
                if (maObj != NULL)
                    ma = maObj->getMAlignment();
            }
        }
    }
    QWidget *p = (QWidget*)AppContext::getMainWindow()->getQMainWindow();
    
    QObjectScopedPointer<UHMM3BuildDialogImpl> buildDlg = new UHMM3BuildDialogImpl( ma, p );
    buildDlg->exec();
}

/* TODO: same as in function sl_search in uHMMPlugin.cpp */
U2SequenceObject * UHMM3Plugin::getDnaSequenceObject() const {
    U2SequenceObject * seqObj = NULL;
    GObjectViewWindow * activeWnd = qobject_cast< GObjectViewWindow* >( AppContext::getMainWindow()->getMDIManager()->getActiveWindow() );
    if( NULL != activeWnd ) {
        AnnotatedDNAView * dnaView = qobject_cast< AnnotatedDNAView* >( activeWnd->getObjectView() );
        seqObj = NULL != dnaView ? dnaView->getSequenceInFocus()->getSequenceObject() : NULL;
    }
    if( NULL == seqObj ) {
        ProjectView * projView = AppContext::getProjectView();
        if( NULL != projView ) {
            const GObjectSelection * objSelection = projView->getGObjectSelection();
            GObject* obj = objSelection->getSelectedObjects().size() == 1 ? objSelection->getSelectedObjects().first() : NULL;
            seqObj = qobject_cast< U2SequenceObject* >( obj );
        }
    }
    return seqObj;
}

void UHMM3Plugin::sl_searchHMMSignals() {
    U2SequenceObject * seqObj = getDnaSequenceObject();
    if( NULL == seqObj ) {
        QMessageBox::critical( NULL, tr( "Error!" ), tr( "Target sequence not selected: no opened annotated dna view" ) );
        return;
    }
    QWidget *p = (QWidget*)AppContext::getMainWindow()->getQMainWindow();
    QObjectScopedPointer<UHMM3SearchDialogImpl> searchDlg = new UHMM3SearchDialogImpl( seqObj, p );
    searchDlg->exec();
}

void UHMM3Plugin::sl_phmmerSearch() {
    U2SequenceObject * seqObj = getDnaSequenceObject();
    if( NULL == seqObj ) {
        QMessageBox::critical( NULL, tr( "Error!" ), tr( "Target sequence not selected: no opened annotated dna view" ) );
        return;
    }
    QWidget *p = (QWidget*)AppContext::getMainWindow()->getQMainWindow();
    QObjectScopedPointer<UHMM3PhmmerDialogImpl> phmmerDlg = new UHMM3PhmmerDialogImpl( seqObj, p );
    phmmerDlg->exec();
}

UHMM3Plugin::~UHMM3Plugin() {
}

/****************************************
* UHMM3MSAEditorContext
****************************************/

UHMM3MSAEditorContext::UHMM3MSAEditorContext( QObject * p ) : GObjectViewWindowContext( p, MSAEditorFactory::ID ) {
}

void UHMM3MSAEditorContext::initViewContext( GObjectView * view ) {
    MSAEditor* msaed = qobject_cast<MSAEditor*>(view);
    assert(msaed!=NULL);
    if (msaed->getMSAObject() == NULL)
        return;

    GObjectViewAction * action = new GObjectViewAction( this, view, tr("Build HMMER3 profile") );
    action->setObjectName("Build HMMER3 profile");
    action->setIcon( QIcon( ":/hmm3/images/hmmer_16.png" ) );
    connect( action, SIGNAL( triggered() ), SLOT( sl_build() ) );
    addViewAction( action );
}

void UHMM3MSAEditorContext::buildMenu( GObjectView * v, QMenu * m ) {
    MSAEditor* msaed = qobject_cast<MSAEditor*>(v);
    assert( NULL != msaed && NULL != m );
    if (msaed->getMSAObject() == NULL)
        return;

    QList< GObjectViewAction* > list = getViewActions( v );
    assert( 1 == list.size() );
    QMenu* aMenu = GUIUtils::findSubMenu( m, MSAE_MENU_ADVANCED );
    SAFE_POINT(aMenu != NULL, "aMenu", );
    aMenu->addAction( list.first() );
}

void UHMM3MSAEditorContext::sl_build() {
    GObjectViewAction * action = qobject_cast< GObjectViewAction* >( sender() );
    assert( NULL != action );
    MSAEditor * ed = qobject_cast< MSAEditor* >( action->getObjectView() );
    assert( NULL != ed );
    MAlignmentObject * obj = ed->getMSAObject(); 
    if (obj != NULL) {
        QObjectScopedPointer<UHMM3BuildDialogImpl> buildDlg = new UHMM3BuildDialogImpl( obj->getMAlignment() );
        buildDlg->exec();
        CHECK(!buildDlg.isNull(), );
    }
}

/****************************************
* UHMM3ADVContext
****************************************/

UHMM3ADVContext::UHMM3ADVContext( QObject * p ) : GObjectViewWindowContext( p, ANNOTATED_DNA_VIEW_FACTORY_ID ) {
}

void UHMM3ADVContext::initViewContext( GObjectView * view ) {
    assert( NULL != view );
    AnnotatedDNAView * av = qobject_cast< AnnotatedDNAView* >( view );
    
    ADVGlobalAction * searchAction = new ADVGlobalAction( av, QIcon( ":/hmm3/images/hmmer_16.png" ), 
                                                          tr( "Find HMM signals with HMMER3..." ), 70 );
    searchAction->setObjectName("Find HMM signals with HMMER3");
    connect( searchAction, SIGNAL( triggered() ), SLOT( sl_search() ) );
}

void UHMM3ADVContext::sl_search() {
    QWidget * parent = getParentWidget( sender() );
    assert( NULL != parent );
    U2SequenceObject * seqObj = getSequenceInFocus( sender() );
    if( NULL == seqObj ) {
        QMessageBox::critical( parent, tr( "error" ), tr( "No sequence in focus found" ) );
        return;
    }
    
    QObjectScopedPointer<UHMM3SearchDialogImpl> searchDlg = new UHMM3SearchDialogImpl( seqObj, parent );
    searchDlg->exec();
}

QWidget * UHMM3ADVContext::getParentWidget( QObject * sender ) {
    assert( NULL != sender );
    GObjectViewAction * action = qobject_cast< GObjectViewAction* >( sender );
    assert( NULL != action );
    AnnotatedDNAView * av = qobject_cast< AnnotatedDNAView* >( action->getObjectView() );
    assert( NULL != av );
    
    if( av->getWidget() ){
        return av->getWidget();
    } else {
        return ( QWidget* )AppContext::getMainWindow()->getQMainWindow();
    }
    return NULL; // unreachable code
}

U2SequenceObject * UHMM3ADVContext::getSequenceInFocus( QObject * sender ) {
    assert( NULL != sender );
    GObjectViewAction * action = qobject_cast< GObjectViewAction* >( sender );
    assert( NULL != action );
    AnnotatedDNAView * av = qobject_cast< AnnotatedDNAView* >( action->getObjectView() );
    assert( NULL != av );
    ADVSequenceObjectContext* seqCtx = av->getSequenceInFocus();
    if( NULL == seqCtx ) {
        return NULL;
    }
    return seqCtx->getSequenceObject();
}

} // GB2
