#pragma once

#include "ExpertDiscoveryData.h"
#include "ExpertDiscoveryTreeWidgetsCtrl.h"
#include "ExpertDiscoveryPropTable.h"

#include <U2View/AnnotatedDNAView.h>
#include <U2View/ADVSplitWidget.h>
#include <U2Core/AutoAnnotationsSupport.h>

#include <U2Gui/MainWindow.h>

#include <QtGui/QToolBar>
#include <QTreeWidget>

namespace U2{
class ExpertDiscoverySignalsAutoAnnotationUpdater;


class ExpertDiscoveryView : public GObjectView{
    Q_OBJECT

public:
    ExpertDiscoveryView(GObjectViewFactoryId factoryId, const QString& viewName, QObject* p=NULL);
    virtual ~ExpertDiscoveryView();
    void insertSeqView(QWidget* view);

    QAction* getNewDocAction(){return newDoc;}
    QAction* getOpenDocAction(){return openDoc;}
    QAction* getSaveDocAction(){return saveDoc;}
    QAction* getSetUpRecBoundAction(){return setUpRecBound;}
    QAction* getOptimizeRecBoundAction(){return optimizeRecBound;}
    QAction* getLoadControlSeqAction(){return loadControlSeqAction;}
    QAction* getExtractSignalsAction(){return extractSignalsAction;}
    QAction* getLoadMarkupAction(){return loadMarkupAction;}
    QAction* getLoadControlMarkupAction(){return loadControlMarkupAction;}
    QAction* getGenerateFullReportAction(){return generateFullReportAction;}
    Task*    getExtractTask(){return extrTask;}

    QList<GObject*> getEDObjects(){ return edObjects;}

    bool askForSave();


private slots:
    void sl_showExpertDiscoveryPosNegDialog();
    void sl_loadPosNegTaskStateChanged();

    void sl_showExpertDiscoveryControlDialog();
    void sl_loadControlTaskStateChanged();

    void sl_showExpertDiscoveryPosNegMrkDialog();
    void sl_loadPosNegMrkTaskStateChanged();

    void sl_showExpertDiscoveryControlMrkDialog();
    void sl_loadControlMrkTaskStateChanged();
    void sl_newDoc();

    void clearSequencesView();
    void sl_openDoc();
    void sl_saveDoc();
    void sl_newSignalReady(DDisc::Signal* signal, CSFolder* folder);
    void sl_optimizeRecBound();
    void sl_setRecBound();
    void sl_extractSignals();
    void sl_treeItemSelChanged(QTreeWidgetItem* tItem);
    void sl_sequenceItemSelChanged(ADVSequenceWidget*);
    void sl_treeWidgetMarkup(bool isLetters);
    void sl_generateFullReport();

    void sl_showSequence();
    void sl_addToShown();
    void sl_showFirstSequences();
    void sl_clearDisplayed();

    void sl_testView(); 

    void sl_newViewTask(Task* t);
    void sl_updateAll();

    void sl_autoAnnotationUpdateStarted();
    void sl_autoAnnotationUpdateFinished();

protected:
    virtual void onObjectAdded(GObject* o){
        //add implementation
    }

private:

    virtual QWidget* createWidget();
    void createActions();
    void initADVView(AnnotatedDNAView* adv);
    void updateAnnotations();

    void createEDSequence();
    void updateEDSequenceProperties();
    DNASequenceObject* getSeqObjectFromEDSequence(EDPISequence* sItem);
    Document* createUDocument(SequenceType sType);

    bool wizzard;
    int updatesCount;

    QSplitter*          splitter;

    EDProjectTree*      signalsWidget;         
    EDPropertiesTable*  propWidget;
    AnnotatedDNAView*   currentAdv;

    ExpertDiscoveryData d;

    QAction*            newDoc;
    QAction*            openDoc;
    QAction*            saveDoc;
    QAction*            setUpRecBound;
    QAction*            optimizeRecBound;
    QAction*            loadControlSeqAction;
    QAction*            extractSignalsAction;
    QAction*            loadMarkupAction;
    QAction*            loadControlMarkupAction;
    QAction*            generateFullReportAction;
    
    QList<GObject*>     edObjects;
    EDPISequence*       curEDsequence;

    Document* posUDoc;
    Document* negUDoc;
    Document* conUDoc;

    Task* extrTask;

    ExpertDiscoverySignalsAutoAnnotationUpdater* edAutoAnnotationsUpdater;

    const EDProcessedSignal* curPS;
    bool updatePS;

    QMutex mutex;

signals:
    void si_insertSeqGObjects(const QList<GObject*>& objects);
};

class ExpertDiscoveryViewWindow : public GObjectViewWindow{
    Q_OBJECT
public:
    ExpertDiscoveryViewWindow(GObjectView* view, const QString& viewName, bool persistent = false);
    virtual void setupMDIToolbar(QToolBar* tb);
protected:
    bool onCloseEvent();

};
}//namespace
