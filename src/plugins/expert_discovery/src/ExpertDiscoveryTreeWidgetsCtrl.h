#ifndef _U2_EXPERT_DIS_TREEWIDGETSCTR_H_
#define _U2_EXPERT_DIS_TREEWIDGETSCTR_H_

#include "DDisc/Signal.h"
#include "DDisc/MetaInfo.h"
#include "ExpertDiscoveryTreeWidgets.h"
#include "ExpertDiscoveryData.h"

#include <QObject>
#include <QAction>
#include <QMenu>
#include <QTreeWidget>

namespace U2 {


class EDProjectTree : public QTreeWidget{
    Q_OBJECT

public:
    EDProjectTree(QWidget *parent, ExpertDiscoveryData &data);

    CSFolder &getRootItem(){edData.getRootFolder();};

    EDProjectItem* findEDItem(void* pData);
    CSFolder* findFolder(EDPICSDirectory *pFolder) const;
    Signal* findSignal(const Signal* pSignal) const;

    void addSubitem(EDProjectItem* subItem, EDProjectItem* parent);
    void updateTree(int flag, EDProjectItem* item = NULL);

    void remake();
    void internalRemake(EDProjectItem* subItem, EDProjectItem* parent);
    void updateItem(EDProjectItem* pItem);
    void updateItemState(EDProjectItem* pItem);
    void updateChildren(EDProjectItem* pItem);
    void updateMarkup();
    void deleteFolder(EDPICSFolder* folder);
    void deleteSignal(EDPICS* signal);
protected slots:
    void sl_newFolder();
    void sl_newSignal();
    void sl_deletePI();
    void sl_selAllSig();
    void sl_deselAllSig();
    void sl_setPriorAllSig();
    void sl_clearPriorAllSig();
    void sl_selDeselSig();
    void sl_setCurPrior();
    void sl_clearCurPrior();
    void sl_markupLetters();
    void sl_setMetainfoBase();

private:
    
    QMenu* chosePopupMen(EDProjectItem* pItem);
    void createPopupsAndActions();

    ExpertDiscoveryData &edData;
    EDPICSRoot root;
    EDPIMrkRoot mrkRoot;

    QMenu*      popupMenuCS;
    QMenu*      popupMenuDir;
    QMenu*      popupMenuDirRoot;
    QMenu*      popupMenuMrkRoot;

    QAction*    newFolderAction;
    QAction*    newSignalAction;
    QAction*    deletePIAction;
    QAction*    selAllSigAction;
    QAction*    deselAllSigAction;
    QAction*    setPriorAllSigAction;
    QAction*    clearPriorAllSigAction;
    QAction*    selDeselSigAction;
    QAction*    setCurPriorAction;
    QAction*    clearCurPriorAction;
    QAction*    markupLettersAction;

protected:
    virtual void mousePressEvent(QMouseEvent *e);

public slots:
    void sl_propChanged(EDProjectItem* item, const EDPIProperty* prop, QString newVal);


};
    
} //namespace

#endif
