#ifndef _U2_ADV_SYNC_VIEW_MANAGER_H_
#define _U2_ADV_SYNC_VIEW_MANAGER_H_

#include <U2Core/global.h>
#include <U2Core/U2Region.h>

#include <QtGui/QAction>
#include <QtGui/QMenu>
#include <QtGui/QToolBar>
#include <QtGui/QToolButton>

namespace U2 {

class AnnotatedDNAView;
class ADVSequenceWidget;
class ADVSingleSequenceWidget;
class LRegionsSelection;  

//Support for synchronous operation for ADVSingleSequenceViews: sync& lock scale, toggle visibility
class ADVSyncViewManager : public QObject {
    Q_OBJECT
public:
    ADVSyncViewManager(AnnotatedDNAView* v);
    ~ADVSyncViewManager();

    void updateToolbar1(QToolBar* tb);
    void updateToolbar2(QToolBar* tb);
private slots:
    void sl_rangeChanged();
    void sl_onSelectionChanged(LRegionsSelection* thiz, const QVector<U2Region>& added, const QVector<U2Region>& removed);
    void sl_sequenceWidgetAdded(ADVSequenceWidget* w);
    void sl_sequenceWidgetRemoved(ADVSequenceWidget* w);

    void sl_lock();
    void sl_sync();

    void sl_toggleVisualMode();
    void updateVisualMode();

private:
    enum SyncMode {
        SyncMode_Start,
        SyncMode_SeqSel,
        SyncMode_AnnSel
    };

    void sync(bool lock, SyncMode mode);

    int offsetBySeqSel(ADVSingleSequenceWidget* w) const;
    int offsetByAnnSel(ADVSingleSequenceWidget* w) const;

    SyncMode detectSyncMode() const;
    int findSelectedAnnotationPos(ADVSingleSequenceWidget* w) const;

    void unlock();
    void updateEnabledState();
    void toggleCheckedAction(SyncMode mode);
    

    QList<ADVSingleSequenceWidget*> getViewsFromADV() const;

    AnnotatedDNAView*                   adv;

    QAction*                            lockByStartPosAction;
    QAction*                            lockBySeqSelAction;
    QAction*                            lockByAnnSelAction;
    QAction*                            syncByStartPosAction;
    QAction*                            syncBySeqSelAction;
    QAction*                            syncByAnnSelAction;
    QActionGroup*                       lockActionGroup;

    QToolButton*                        syncButton;
    QToolButton*                        lockButton;
    QMenu*                              syncMenu;
    QMenu*                              lockMenu;

    QAction*                            syncButtonTBAction;
    QAction*                            lockButtonTBAction;

    bool                                selectionRecursion;
    bool                                recursion;
    QList<ADVSingleSequenceWidget*>     views;
    
    // visual mode vars
    QAction*                            toggleViewButtonAction;
    QToolButton*                        toggleViewButton;
    QMenu*                              toggleViewButtonMenu;
    QAction*                            toggleAllAction;
    QAction*                            toggleOveAction;
    QAction*                            togglePanAction;
    QAction*                            toggleDetAction;
};

} //namespace
#endif
