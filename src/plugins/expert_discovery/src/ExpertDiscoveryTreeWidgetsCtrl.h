#ifndef _U2_EXPERT_DIS_TREEWIDGETSCTR_H_
#define _U2_EXPERT_DIS_TREEWIDGETSCTR_H_

#include "DDisc/Signal.h"
#include "DDisc/MetaInfo.h"
#include "ExpertDiscoveryTreeWidgets.h"
#include "ExpertDiscoveryData.h"

#include <QObject>
#include <QAction>
 #include <QActionGroup>
#include <QMenu>
#include <QTreeWidget>

namespace U2 {

class EDProjectTree : public QTreeWidget{
    Q_OBJECT

public:
    EDProjectTree(QWidget *parent, ExpertDiscoveryData &data);

    CSFolder &getRootItem(){return edData.getRootFolder();}

    EDProjectItem* findEDItem(void* pData);
    CSFolder* findFolder(EDPICSDirectory *pFolder) const;
    Signal* findSignal(const Signal* pSignal) const;

    void addSubitem(EDProjectItem* subItem, EDProjectItem* parent);
    void updateTree(int flag, EDProjectItem* item = NULL);

    void remake();
    void clearTree();
    void internalRemake(EDProjectItem* subItem, EDProjectItem* parent);
    void updateItem(EDProjectItem* pItem);
    void updateItemState(EDProjectItem* pItem);
    void updateChildren(EDProjectItem* pItem);
    void updateMarkup();
    void updateSequenceBase(EItemType type);
    void deleteFolder(EDPICSFolder* folder);
    void deleteSignal(EDPICS* signal);
    void updateSorting();
    void updateSortingRecurs(EDProjectItem* pItem);

    bool isUpdatingItem() {return updatingItem;}

public slots:
    void sl_newFolder();
protected slots:
    void sl_newSignal();
    void sl_deletePI();
    void sl_selAllSig();
    void sl_deselAllSig();
    void sl_setPriorAllSig();
    void sl_clearPriorAllSig();
    void sl_addSignalToMarkup();
    void sl_selDeselSig();
    void sl_setCurPrior();
    void sl_clearCurPrior();
    void sl_markupLetters();
    void sl_loadMarkup();
    void sl_showSequence();
    void sl_addToShown();
    void sl_clearDisplayed();
    void sl_showFirstSequences();
    void sl_setMetainfoBase();
    void sl_generateReport();
    void sl_sortField(QAction* action);
    void sl_sortOrd(QAction* action);

private:
    
    QMenu* chosePopupMen(EDProjectItem* pItem);
    void createPopupsAndActions();
    QIcon getItemIcon(EDProjectItem* pItem);

    void onCSNPropertyChanged(EDProjectItem* pItem, const EDPIProperty* pProperty, QString strNewValue);
    void onDistancePropertyChanged(EDProjectItem* pItem, const EDPIProperty* pProperty, QString strNewValue);
    void onRepetitionPropertyChanged(EDProjectItem* pItem, const EDPIProperty* pProperty, QString strNewValue);
    void onIntervalPropertyChanged(EDProjectItem* pItem, const EDPIProperty* pProperty, QString strNewValue);
    void onWordPropertyChanged(EDProjectItem* pItem, const EDPIProperty* pProperty, QString strNewValue);
    void onMrkItemPropertyChanged(EDProjectItem* pItem, const EDPIProperty* pProperty, QString strNewValue);
    Operation* createCSN(int ValueId) const;


    ExpertDiscoveryData &edData;
    EDPICSRoot root;
    EDPIMrkRoot mrkRoot;
    EDPISequenceRoot seqRoot;

    QMenu*      popupMenuCS;
    QMenu*      popupMenuDir;
    QMenu*      popupMenuDirRoot;
    QMenu*      popupMenuMrkRoot;
    QMenu*      popupMenuSequence;
    QMenu*      popupMenuSequenceBase;

    QAction*    newFolderAction;
    QAction*    newSignalAction;
    QAction*    deletePIAction;
    QAction*    selAllSigAction;
    QAction*    deselAllSigAction;
    QAction*    setPriorAllSigAction;
    QAction*    clearPriorAllSigAction;
    QAction*    addSignalToMarkupAction;
    QAction*    selDeselSigAction;
    QAction*    setCurPriorAction;
    QAction*    clearCurPriorAction;
    QAction*    markupLettersAction;
    QAction*    loadMarkupAction;
    QAction*    showSequenceAction;
    QAction*    addToShownAction;
    QAction*    generateReportAction;
    QAction*    clearDisplayedAction;
    QAction*    showFirstSequencesAction;

    QActionGroup* sortGroup;
    QActionGroup* sortOrdGroup;
    QActionGroup* sortFieldGroup;
    QAction*      sortOrdIncrAction;
    QAction*      sortOrdDecrAction;
    QAction*      sortFieldCoverAction;
    QAction*      sortFieldFisherAction;
    QAction*      sortFieldNameAction;
    QAction*      sortFieldProbAction;

    EDSortParameters sortField;
    EDSortParameters sortOrd;

    bool updatingItem;
protected:
    virtual void mousePressEvent(QMouseEvent *e);
    virtual void mouseDoubleClickEvent(QMouseEvent *event);

public slots:
    void sl_propChanged(EDProjectItem* item, const EDPIProperty* prop, QString newVal);

signals:
    void si_loadMarkup(bool isLetters);
    void si_showSequence();
    void si_addToShown();
    void si_showFirstSequences();
    void si_clearDisplayed();
    void si_changeProp(QTreeWidgetItem* item);
};
    
} //namespace

#endif
