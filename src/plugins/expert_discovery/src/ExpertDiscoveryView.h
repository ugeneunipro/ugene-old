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
class ExpertDiscoveryUpdateSelectionTask;

class ExpertDiscoveryView : public GObjectView{
    Q_OBJECT

public:
    ExpertDiscoveryView(GObjectViewFactoryId factoryId, const QString& viewName, QObject* p=NULL);
    virtual ~ExpertDiscoveryView();
    void insertSeqView(QWidget* view);

    //view content
    QAction* getNewDocAction(){return newDoc;}
    QAction* getOpenDocAction(){return openDoc;}
    QAction* getSaveDocAction(){return saveDoc;}
    QAction* getSetUpRecBoundAction(){return setUpRecBound;}
    QAction* getOptimizeRecBoundAction(){return optimizeRecBound;}
    QAction* getLoadControlSeqAction(){return loadControlSeqAction;}
    QAction* getExtractSignalsAction(){return extractSignalsAction;}
    QAction* getLoadControlMarkupAction(){return loadControlMarkupAction;}
    QAction* getGenerateFullReportAction(){return generateFullReportAction;}
    Task*    getExtractTask(){return extrTask;}
    
    QAction* getLoadMarkupAction(){return loadMarkupAction;}
    QList<GObject*> getEDObjects() {return edObjects;}
    
    EDProjectTree* getProjectTree() {return signalsWidget;}
    EDPropertiesTable* getPropertiesWidget() {return  propWidget;}
    AnnotatedDNAView* getCurrentAdv() {return currentAdv;}
    ExpertDiscoveryData& getExpertDiscoveryData(){return d;}
    ExpertDiscoverySignalsAutoAnnotationUpdater* getAutoAnnotationUpdater() {return edAutoAnnotationsUpdater;}
    const EDProcessedSignal* getCurrentProcessedSignals(){return curPS;}
    void setProcessedSignals(const EDProcessedSignal* PS){curPS = PS;}

    bool askForSave();


private slots:
    //     void sl_showExpertDiscoveryControlMrkDialog();
    //     void sl_loadControlMrkTaskStateChanged();

    void sl_newDoc();

    void sl_openDoc();
    void sl_saveDoc();
    void sl_showExpertDiscoveryPosNegDialog();
    void sl_loadPosNegTaskStateChanged();

    void sl_showExpertDiscoveryPosNegMrkDialog();
    void sl_loadPosNegMrkTaskStateChanged();

    void sl_showExpertDiscoveryControlDialog();
    void sl_loadControlTaskStateChanged();

    void sl_setRecBound();
    void sl_extractSignals();
    void sl_optimizeRecBound();
    void sl_generateFullReport();
    void sl_newSignalReady(DDisc::Signal* signal, CSFolder* folder);
    void sl_newFolder();
    void sl_treeItemSelChanged(QTreeWidgetItem* tItem);
    void sl_updateTaskFinished();
    void sl_treeWidgetMarkup(bool isLetters);
    void sl_treeWidgetAddMarkup();
    void sl_updateMarking();

    void sl_showSequence();
    void sl_addToShown();
    void sl_showFirstSequences();
    void sl_clearDisplayed();

    void clearSequencesView();
    void sl_updateAll();

    void sl_autoAnnotationUpdateStarted();
    void sl_autoAnnotationUpdateFinished();

    void sl_sequenceItemSelChanged(ADVSequenceWidget*);
    void sl_newViewTask(Task* t);
    void sl_testView(); 

    //void sl_propChanged(QTreeWidgetItem* item);

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
    U2SequenceObject* getSeqObjectFromEDSequence(EDPISequence* sItem);
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

    Document*           posUDoc;
    Document*           negUDoc;
    Document*           conUDoc;

    Task*               extrTask;

    ExpertDiscoverySignalsAutoAnnotationUpdater* edAutoAnnotationsUpdater;

    const EDProcessedSignal*    curPS;
    bool                        updatePS;

    QMutex                      mutex;
    ExpertDiscoveryUpdateSelectionTask* curTask;

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
