#pragma once

#include "ExpertDiscoveryData.h"
#include "ExpertDiscoveryTreeWidgetsCtrl.h"
#include "ExpertDiscoveryPropTable.h"

#include <U2View/AnnotatedDNAView.h>
#include <U2View/ADVSplitWidget.h>

#include <U2Gui/MainWindow.h>

#include <QtGui/QToolBar>
#include <QTreeWidget>

namespace U2{

class ExpertDiscoveryADVSplitWidget : public ADVSplitWidget{
    Q_OBJECT
public:
    ExpertDiscoveryADVSplitWidget(AnnotatedDNAView* view);
    virtual bool acceptsGObject(GObject* objects){Q_UNUSED(objects); return false;}
    virtual void updateState(const QVariantMap& m) {}
    virtual void saveState(QVariantMap& m) {}
};



class ExpertDiscoveryView : public GObjectView{
	Q_OBJECT

public:
    ExpertDiscoveryView(GObjectViewFactoryId factoryId, const QString& viewName, QObject* p=NULL);
    //ExpertDiscoveryView();
    //virtual void setupMDIToolbar(QToolBar* tb);
    void insertSeqView(QWidget* view);

    QAction* getNewDocAction(){return newDoc;}
    QAction* getSetUpRecBoundAction(){return setUpRecBound;}
    QAction* getOptimizeRecBoundAction(){return optimizeRecBound;}
    QAction* getLoadControlSeqAction(){return loadControlSeqAction;}
    QAction* getExtractSignalsAction(){return extractSignalsAction;}

    QList<GObject*> getEDObjects(){ return edObjects;}


private slots:
    void sl_newDoc();
	void sl_showExpertDiscoveryPosNegDialog();
	void sl_loadPosNegTaskStateChanged();

	void sl_showExpertDiscoveryControlDialog();
	void sl_loadControlTaskStateChanged();
    void sl_newSignalReady(DDisc::Signal* signal, CSFolder* folder);
    void sl_optimizeRecBound();
    void sl_setRecBound();
    void sl_extractSignals();
    void sl_treeItemSelChanged(QTreeWidgetItem* tItem);

    void sl_testView(); 

    void sl_newViewTask(Task* t);

protected:
    virtual void onObjectAdded(GObject* o){

    }

private:

    virtual QWidget* createWidget();
    void createActions();
    void initADVView(AnnotatedDNAView* adv);

    bool wizzard;

    QSplitter*          splitter;

    EDProjectTree*      signalsWidget;         
    EDPropertiesTable*  propWidget;
    AnnotatedDNAView*   currentAdv;

    ExpertDiscoveryADVSplitWidget*   seqWidget;

	ExpertDiscoveryData d;

    QAction*            newDoc;
    QAction*            setUpRecBound;
    QAction*            optimizeRecBound;
    QAction*            loadControlSeqAction;
    QAction*            extractSignalsAction;

    QList<GObject*>		edObjects;

signals:
    void si_insertSeqGObjects(const QList<GObject*>& objects);
};

class ExpertDiscoveryViewWindow : public GObjectViewWindow{
	Q_OBJECT
public:
    ExpertDiscoveryViewWindow(GObjectView* view, const QString& viewName, bool persistent = false);
    virtual void setupMDIToolbar(QToolBar* tb);

};

class ExpertDiscoveryView1 : public GObjectView{
    Q_OBJECT

public:
    ExpertDiscoveryView1(GObjectViewFactoryId factoryId, const QString& viewName, QObject* p=NULL);


private:
    virtual QWidget* createWidget();
};








}//namespace