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

#include <QMouseEvent>
#include <QStandardItemModel>
#include <QMessageBox>

#include <U2Core/U2SafePoints.h>

#include <U2Core/QObjectScopedPointer.h>

#include "ExpertDiscoveryTreeWidgetsCtrl.h"
#include "ExpertDiscoveryTask.h"

namespace U2 {

EDProjectTree::EDProjectTree(QWidget *parent, ExpertDiscoveryData &d)
:QTreeWidget(parent)
,edData(d)
,root(edData.getRootFolder())
,seqRoot(d)
,sortField(ED_FIELD_PROBABILITY)
,sortOrd(ED_ORDER_DECREASING)
,updatingItem(false)
{
    createPopupsAndActions();
    updateTree(ED_UPDATE_ALL);
}

EDProjectItem* EDProjectTree::findEDItem(void* pData){
    return const_cast<EDProjectItem*>(root.findItemConnectedTo(pData));
}

CSFolder* EDProjectTree::findFolder(EDPICSDirectory *pFolder) const
{
    return const_cast<CSFolder*>(pFolder->getFolder());
}

Signal*EDProjectTree::findSignal(const Signal* pSignal) const
{
    return const_cast<Signal*>(pSignal);
}

void EDProjectTree::addSubitem(EDProjectItem* subItem, EDProjectItem* parent){

    parent->addChild(subItem);
    updateTree(ED_ITEM_ADDED, subItem);
}

void EDProjectTree::updateTree(int flag, EDProjectItem* item){
    switch (flag) {
    case ED_UPDATE_ALL                :    remake();            break;
    case ED_ITEM_NAME_CHANGED        :    updateItem(item);    break;
    case ED_ITEM_STATE_CHANGED        :    updateItemState(item);    break;
    case ED_ITEM_ADDED                :    internalRemake(item, dynamic_cast<EDProjectItem*>(dynamic_cast<QTreeWidgetItem*>(item)->parent()));    break;
    case ED_ITEM_DELETED            :    /*deleteItem(pItem);*/  break;
    case ED_CURRENT_ITEM_CHANGED    :    setCurrentItem(item); emit si_changeProp(item); break;
    case ED_UPDATE_CHILDREN            :    updateChildren(item); break;
    case ED_MRK_UPDATE              :   updateMarkup(); break;
    };
}

void EDProjectTree::remake(){

    addTopLevelItem(&seqRoot);
    seqRoot.setText(0,"Sequences");
    seqRoot.setIcon(0,QIcon(":expert_discovery/images/sequences.png"));
    seqRoot.setExpanded(true);
    seqRoot.update(true);
    for (int i = 0; i < seqRoot.childCount(); i++){
        EDProjectItem* item = dynamic_cast<EDProjectItem*>(seqRoot.child(i));
        if(item){
            internalRemake(item, &seqRoot);
        }
    }

    addTopLevelItem(&mrkRoot);
    mrkRoot.setText(0,"Markup");
    mrkRoot.setIcon(0,QIcon(":expert_discovery/images/mrkroot.png"));
    mrkRoot.updMarkup(edData);
    for (int i = 0; i < mrkRoot.childCount(); i++){
        EDProjectItem* item = dynamic_cast<EDProjectItem*>(mrkRoot.child(i));
        if(item){
            internalRemake(item, &mrkRoot);
        }
    }

    addTopLevelItem(&root);
    root.setText(0,"Complex signals");
    root.setIcon(0,QIcon(":expert_discovery/images/csroot.png"));
    root.update(true);
    for (int i = 0; i < root.childCount(); i++){
        EDProjectItem* item = dynamic_cast<EDProjectItem*>(root.child(i));
        if(item){
            internalRemake(item, &root);
        }
    }
}

void EDProjectTree::clearTree(){
   root.takeChildren();
   mrkRoot.takeChildren();
}

void EDProjectTree::internalRemake(EDProjectItem* subItem, EDProjectItem* parent){

    subItem->setText(0, subItem->getName());
    subItem->setIcon(0, getItemIcon(subItem));
    subItem->setSortOrd(sortOrd);
    subItem->setSortField(sortField);

    for(int i = 0; i < subItem->childCount(); i++){
        EDProjectItem* item = dynamic_cast<EDProjectItem*>(subItem->child(i));
        if (item){
            connect(item, SIGNAL(si_getMetaInfoBase()), SLOT(sl_setMetainfoBase()));
            internalRemake(item, subItem);
        }

    }
    subItem->sortChildren(0, Qt::AscendingOrder);
    updateItemState(subItem);

}

void EDProjectTree::updateItem(EDProjectItem* pItem){
    pItem->setText(0, pItem->getName());
    pItem->setIcon(0, getItemIcon(pItem));

    QFont curFont = pItem->font(0);
    if (edData.isSignalSelected(pItem))
        curFont.setBold(true);
    else
        curFont.setBold(false);

    EDPISequence* seq = dynamic_cast<EDPISequence*>(pItem);
    if(seq){
        if(edData.isSequenceSelected(seq)){
            curFont.setBold(true);
        }
        else{
            curFont.setBold(false);
        }
    }

    pItem->setFont(0, curFont);


}
void EDProjectTree::updateItemState(EDProjectItem *pItem){
    QFont curFont = pItem->font(0);
    if (edData.isSignalSelected(pItem))
        curFont.setBold(true);
    else
        curFont.setBold(false);

    pItem->setFont(0, curFont);
}


void EDProjectTree::updateChildren(EDProjectItem* pItem){

    for (int i=0; i<pItem->childCount(); i++){
        EDProjectItem* ch = dynamic_cast<EDProjectItem*>(pItem->child(i));
        internalRemake(ch, pItem);

    }
    pItem->sortChildren(0, Qt::AscendingOrder);
}

void EDProjectTree::updateMarkup(){
    mrkRoot.updMarkup(edData);
    for (int i = 0; i < mrkRoot.childCount(); i++){
        EDProjectItem* item = dynamic_cast<EDProjectItem*>(mrkRoot.child(i));
        if(item){
            internalRemake(item, &mrkRoot);
        }
    }
}

void EDProjectTree::updateSequenceBase(EItemType type){
    if(type == PIT_SEQUENCEROOT){
        seqRoot.update(true);
        for (int i = 0; i < seqRoot.childCount(); i++){
            EDProjectItem* item = dynamic_cast<EDProjectItem*>(seqRoot.child(i));
            if(item){
                    item->update(true);
                    internalRemake(item, &seqRoot);
            }
        }
    }else{
        for (int i = 0; i < seqRoot.childCount(); i++){
            EDProjectItem* item = dynamic_cast<EDProjectItem*>(seqRoot.child(i));
            if(item){
                if(item->getType() == type){
                    item->update(true);
                    internalRemake(item, &seqRoot);
                }
            }
        }
    }
}

void EDProjectTree::sl_propChanged(EDProjectItem* item, const EDPIProperty* prop, QString newVal){
    switch (item->getType()) {
        case PIT_CS_FOLDER:
            if (prop->getName().compare("Name", Qt::CaseInsensitive) == 0) {
                EDPICSDirectory* pPI = dynamic_cast<EDPICSDirectory*>(item);
                CSFolder* pFolder = findFolder(pPI);
                const CSFolder* pParentFolder = pFolder->getParentFolder();
                int nIndex = pParentFolder->getFolderIndexByName(newVal);
                if (nIndex>=0) {
                    QObjectScopedPointer<QMessageBox> mb = new QMessageBox(QMessageBox::Critical, tr("Rename error"), tr("Folder already exist"));
                    mb->exec();
                    return;
                }
                pFolder->setName(newVal);
                updateTree(ED_ITEM_NAME_CHANGED, qobject_cast<EDProjectItem *>(pPI));
                updateTree(ED_CURRENT_ITEM_CHANGED, qobject_cast<EDProjectItem *>(pPI));
            }
            return;
        case PIT_CS:{
           //updatingItem = true;
           EDPICS *pPI = dynamic_cast<EDPICS*>(item);
           EDPICSDirectory *pParent = dynamic_cast<EDPICSDirectory*>(dynamic_cast<QTreeWidgetItem*>(pPI)->parent());
           assert(pPI != NULL);
            assert(pParent != NULL);

            Signal *pSignal = findSignal(pPI->getSignal());

            QString strName = "Name";
            QString strDescription = "Description";

            QString strPropName = prop->getName();
            if (strPropName.compare(strName, Qt::CaseInsensitive)==0)
            {
                CSFolder* pFolder = findFolder(pParent);
                int nIndex = pFolder->getSignalIndexByName(newVal);
                if (nIndex >= 0) {
                    QObjectScopedPointer<QMessageBox> mb = new QMessageBox(QMessageBox::Question, tr("Signal rename"), tr("Signal with the same name already exist. Replace?"), QMessageBox::Ok|QMessageBox::Cancel);
                    mb->exec();
                    CHECK(!mb.isNull(), );

                    if (mb->result() == QMessageBox::Ok)
                        pFolder->deleteSignal(nIndex);
                    else
                        return;
                }
                pSignal->setName(newVal.toStdString());
            }
            else
                if (strPropName.compare(strDescription, Qt::CaseInsensitive) == 0)
                {
                    pSignal->setDescription(newVal.toStdString());
                }
                else assert(0);
            pParent->update(true);
           // updatingItem = false;
            EDProjectItem* pCurItem = const_cast<EDProjectItem*>(pParent->findItemConnectedTo(pSignal));
            updateTree(ED_UPDATE_CHILDREN, qobject_cast<EDProjectItem *>(pParent));
            updateTree(ED_CURRENT_ITEM_CHANGED, pCurItem);
            return;
                    }
        case PIT_CSN_UNDEFINED:
        case PIT_CSN_DISTANCE:
        case PIT_CSN_REPETITION:
        case PIT_CSN_INTERVAL:
        case PIT_CSN_WORD:
        case PIT_CSN_MRK_ITEM:
            onCSNPropertyChanged(item, prop, newVal);
        return;
    }
}

void EDProjectTree::mousePressEvent(QMouseEvent *e){
    if(e->button() == Qt::RightButton){
        QTreeWidgetItem* curItem = itemAt(e->pos());
        setCurrentItem(curItem, 0);
        EDProjectItem* projItem = dynamic_cast<EDProjectItem*>(curItem);
        if(projItem){
            QMenu* curMenu = chosePopupMen(projItem);
            if(curMenu){
                curMenu->popup(e->globalPos());
            }
        }
    }
    QTreeWidget::mousePressEvent(e);
}

void EDProjectTree::mouseDoubleClickEvent(QMouseEvent *e){

    QTreeWidgetItem* curItem = itemAt(e->pos());
    setCurrentItem(curItem, 0);

    sl_addToShown();

    QTreeWidget::mouseDoubleClickEvent(e);
}

QMenu* EDProjectTree::chosePopupMen(EDProjectItem* pItem){
    if(pItem->getType() == PIT_CS){
        EDPICS* pItem = dynamic_cast<EDPICS*>(currentItem());
        if(pItem){
            if(!edData.isSignalSelected(pItem)){
                selDeselSigAction->setText(tr("Select signal"));
            }else{
                selDeselSigAction->setText(tr("Deselect signal"));
            }
            setCurPriorAction->setEnabled(pItem->getProcessedSignal(edData) != NULL);
            clearCurPriorAction->setEnabled(pItem->getSignal()->isPriorParamsDefined() && !edData.isSignalSelected(pItem));
        }else{
            selDeselSigAction->setText(tr("Select/deselect signal"));
            setCurPriorAction->setEnabled(true);
            clearCurPriorAction->setEnabled(true);
        }
        return popupMenuCS;
    }else if(pItem->getType() == PIT_CS_FOLDER){
        EDPICSDirectory* piDir = dynamic_cast<EDPICSDirectory*>(pItem);
        if(!piDir){
            return NULL;
        }
        return popupMenuDir;

    }else if (pItem->getType() == PIT_CS_ROOT){
        return popupMenuDirRoot;
    }else if(pItem->getType() == PIT_MRK_ROOT){
        markupLettersAction->setEnabled(!edData.isLettersMarkedUp() && (edData.getNegSeqBase().getSize() > 0) && (edData.getPosSeqBase().getSize() > 0) );
        loadMarkupAction->setEnabled(edData.getPosSeqBase().getSize() > 0 && edData.getNegSeqBase().getSize() > 0);
        return popupMenuMrkRoot;
    }else if(pItem->getType() == PIT_SEQUENCE || pItem->getType() == PIT_CONTROLSEQUENCE){
        return popupMenuSequence;
    }else if(pItem->getType() == PIT_POSSEQUENCEBASE || pItem->getType() == PIT_NEGSEQUENCEBASE || pItem->getType() == PIT_CONTROLSEQUENCEBASE){
        EDPISequenceBase* pBaseItem = dynamic_cast<EDPISequenceBase*>(pItem);
        if (!pBaseItem)
        {
            return NULL;
        }
        generateReportAction->setEnabled(pBaseItem->getSequenceBase().getSize() != 0);
        exportToSequencesAction->setEnabled(pBaseItem->getSequenceBase().getSize() != 0);
        showFirstSequencesAction->setEnabled(pBaseItem->getSequenceBase().getSize() != 0);
        return popupMenuSequenceBase;
    }
    return NULL;

}

QIcon EDProjectTree::getItemIcon(EDProjectItem* pItem){
    EItemType iType = pItem->getType();

    switch (iType)
    {
        case PIT_SEQUENCEROOT: return QIcon(":expert_discovery/images/sequences.png");
        case PIT_POSSEQUENCEBASE: return QIcon(":expert_discovery/images/posseq.png");
        case PIT_NEGSEQUENCEBASE: return QIcon(":expert_discovery/images/negseq.png");
        case PIT_CONTROLSEQUENCEBASE: return QIcon(":expert_discovery/images/control.png");
        case PIT_SEQUENCE: return QIcon(":expert_discovery/images/sequence.png");
        case PIT_CONTROLSEQUENCE: return QIcon(":expert_discovery/images/sequence.png");
        case PIT_CS_ROOT: return QIcon(":expert_discovery/images/csroot.png");
        case PIT_CS_FOLDER: return QIcon(":expert_discovery/images/folder.png");
        case PIT_CS: return QIcon(":expert_discovery/images/cs.png");
        case PIT_CSN_UNDEFINED: return QIcon(":expert_discovery/images/undefined.png");
        case PIT_CSN_DISTANCE: return QIcon(":expert_discovery/images/distance.png");
        case PIT_CSN_REPETITION: return QIcon(":expert_discovery/images/repetition.png");
        case PIT_CSN_INTERVAL: return QIcon(":expert_discovery/images/interval.png");
        case PIT_CSN_MRK_ITEM: return QIcon(":expert_discovery/images/ts.png");
        case PIT_MRK_ITEM: return QIcon(":expert_discovery/images/ts.png");
        case PIT_CSN_WORD: return QIcon(":expert_discovery/images/ts.png");
        case PIT_MRK_ROOT: return QIcon(":expert_discovery/images/mrkroot.png");
        case PIT_MRK_FAMILY: return QIcon(":expert_discovery/images/folder.png");
        default: return QIcon();
    }

    return QIcon();
}

void EDProjectTree::createPopupsAndActions(){
    newFolderAction = new QAction(tr("New folder"), this);
    connect(newFolderAction, SIGNAL(triggered(bool)), SLOT(sl_newFolder()));

    newSignalAction= new QAction(tr("New signal"), this);
    connect(newSignalAction, SIGNAL(triggered(bool)), SLOT(sl_newSignal()));

    deletePIAction = new QAction(tr("Delete"), this);
    connect(deletePIAction, SIGNAL(triggered(bool)), SLOT(sl_deletePI()));

    selAllSigAction = new QAction(tr("Select all signals"), this);
    connect(selAllSigAction, SIGNAL(triggered(bool)), SLOT(sl_selAllSig()));

    deselAllSigAction = new QAction(tr("Deselect all signals"), this);
    connect(deselAllSigAction, SIGNAL(triggered(bool)), SLOT(sl_deselAllSig()));

    setPriorAllSigAction = new QAction(tr("Set prior parameters of all signals"), this);
    connect(setPriorAllSigAction, SIGNAL(triggered(bool)), SLOT(sl_setPriorAllSig()));

    clearPriorAllSigAction = new QAction(tr("Clear prior parameters of all signals"), this);
    connect(clearPriorAllSigAction, SIGNAL(triggered(bool)), SLOT(sl_clearPriorAllSig()));

    addSignalToMarkupAction = new QAction(tr("Add signal to markup"), this);
    connect(addSignalToMarkupAction, SIGNAL(triggered(bool)), SLOT(sl_addSignalToMarkup()));

    selDeselSigAction = new QAction(tr("Select/deselect signal"), this);
    connect(selDeselSigAction, SIGNAL(triggered(bool)), SLOT(sl_selDeselSig()));

    setCurPriorAction = new QAction(tr("Set current params as prior"), this);
    connect(setCurPriorAction, SIGNAL(triggered(bool)), SLOT(sl_setCurPrior()));

    clearCurPriorAction = new QAction(tr("Clear prior params"), this);
    connect(clearCurPriorAction, SIGNAL(triggered(bool)), SLOT(sl_clearCurPrior()));

    markupLettersAction = new QAction(tr("Markup letters"), this);
    connect(markupLettersAction, SIGNAL(triggered(bool)), SLOT(sl_markupLetters()));

    loadMarkupAction = new QAction(tr("Load markup"), this);
    connect(loadMarkupAction, SIGNAL(triggered(bool)), SLOT(sl_loadMarkup()));

    showSequenceAction = new QAction(tr("Show one sequence"), this);
    connect(showSequenceAction, SIGNAL(triggered(bool)), SLOT(sl_showSequence()));

    addToShownAction = new QAction(tr("Add to displayed"), this);
    connect(addToShownAction, SIGNAL(triggered(bool)), SLOT(sl_addToShown()));

    generateReportAction = new QAction(tr("Generate report"), this);
    connect(generateReportAction, SIGNAL(triggered(bool)), SLOT(sl_generateReport()));

    exportToSequencesAction = new QAction(tr("Export Sequences"), this);
    connect(exportToSequencesAction, SIGNAL(triggered(bool)), SLOT(sl_exportSequences()));

    clearDisplayedAction = new QAction(tr("Clear displayed sequences area"), this);
    connect(clearDisplayedAction, SIGNAL(triggered(bool)), SLOT(sl_clearDisplayed()));

    showFirstSequencesAction = new QAction(tr("Show sequences"), this);
    connect(showFirstSequencesAction, SIGNAL(triggered(bool)), SLOT(sl_showFirstSequences()));

    sortGroup = new QActionGroup(this);
    sortOrdGroup = new QActionGroup(this);
    sortFieldGroup = new QActionGroup(this);

    connect(sortOrdGroup, SIGNAL(triggered(QAction* )), SLOT(sl_sortOrd(QAction* )));
    connect(sortFieldGroup, SIGNAL(triggered(QAction* )), SLOT(sl_sortField(QAction* )));


    sortOrdIncrAction = new QAction(tr("Increasing"), this);
    sortOrdDecrAction = new QAction(tr("Decreasing"), this);

    sortFieldCoverAction = new QAction(tr("Coverage"), this);
    sortFieldFisherAction = new QAction(tr("Fisher"), this);
    sortFieldNameAction = new QAction(tr("Name"), this);
    sortFieldProbAction = new QAction(tr("Probability"), this);

    sortOrdIncrAction->setCheckable(true);
    sortOrdDecrAction->setCheckable(true);
    sortOrdDecrAction->setChecked(true);

    sortFieldCoverAction->setCheckable(true);
    sortFieldFisherAction->setCheckable(true);
    sortFieldNameAction->setCheckable(true);
    sortFieldProbAction->setCheckable(true);
    sortFieldProbAction->setChecked(true);

    sortOrdGroup->addAction(sortOrdIncrAction);
    sortOrdGroup->addAction(sortOrdDecrAction);

    sortFieldGroup->addAction(sortFieldCoverAction);
    sortFieldGroup->addAction(sortFieldFisherAction);
    sortFieldGroup->addAction(sortFieldNameAction);
    sortFieldGroup->addAction(sortFieldProbAction);



    popupMenuCS = new QMenu(this);
    popupMenuCS->addAction(selDeselSigAction);
    popupMenuCS->addAction(deletePIAction);
    popupMenuCS->addSeparator();
    popupMenuCS->addAction(addSignalToMarkupAction);
    //popupMenuCS->addAction(setCurPriorAction);
    //popupMenuCS->addAction(clearCurPriorAction);


    popupMenuDir = new QMenu(this);
    popupMenuDir->addAction(newFolderAction);
    popupMenuDir->addAction(newSignalAction);
    popupMenuDir->addSeparator();
    popupMenuDir->addAction(deletePIAction);
    popupMenuDir->addSeparator();
    popupMenuDir->addAction(selAllSigAction);
    popupMenuDir->addAction(deselAllSigAction);
    //popupMenuDir->addAction(setPriorAllSigAction);
    //popupMenuDir->addAction(clearPriorAllSigAction);

    popupMenuDirRoot = new QMenu(this);
    popupMenuDirRoot->addAction(newFolderAction);
    popupMenuDirRoot->addAction(newSignalAction);
    popupMenuDirRoot->addSeparator();
    popupMenuDirRoot->addAction(selAllSigAction);
    popupMenuDirRoot->addAction(deselAllSigAction);
    //popupMenuDirRoot->addAction(setPriorAllSigAction);
    //popupMenuDirRoot->addAction(clearPriorAllSigAction);

    QMenu* sortMenu = new QMenu(tr("Sort"), this);
    QMenu* fieldMenu = new QMenu(tr("Field"), this);
    QMenu* ordMenu = new QMenu(tr("Order"), this);
    fieldMenu->addActions(sortFieldGroup->actions());
    ordMenu->addActions(sortOrdGroup->actions());
    sortMenu->addMenu(ordMenu);
    sortMenu->addMenu(fieldMenu);

    popupMenuDirRoot->addMenu(sortMenu);

    popupMenuMrkRoot = new QMenu(this);
    popupMenuMrkRoot->addAction(markupLettersAction);
    popupMenuMrkRoot->addAction(loadMarkupAction);

    popupMenuSequence = new QMenu(this);
    popupMenuSequence->addAction(showSequenceAction);
    popupMenuSequence->addAction(addToShownAction);
    popupMenuSequence->addAction(clearDisplayedAction);

    popupMenuSequenceBase = new QMenu(this);
    popupMenuSequenceBase->addAction(generateReportAction);
    popupMenuSequenceBase->addAction(exportToSequencesAction);
    popupMenuSequenceBase->addAction(showFirstSequencesAction);
}

void EDProjectTree::onCSNPropertyChanged(EDProjectItem* pItem, const EDPIProperty* pProperty, QString strNewValue){
    EDPICSNode* pCSN = dynamic_cast<EDPICSNode*>(pItem);
    Operation* pOp = pCSN->getOperation();


    EDProjectItem *pParent = dynamic_cast<EDProjectItem*>(dynamic_cast<QTreeWidgetItem*>(pItem)->parent());
    QString strType = "Type";


    if (pProperty->getName().compare(strType, Qt::CaseInsensitive) == 0) {

        Operation* pNewOp = createCSN(EDPIPropertyTypeListCSNodeTypes::getInstance()->getValueId(strNewValue));
        assert(pNewOp != NULL);

        if (pParent->getType() == PIT_CS) {
            updatingItem = true;
            EDPICS *pPICS = dynamic_cast<EDPICS*>(pParent);
            connect(pPICS, SIGNAL(si_getMetaInfoBase()), SLOT(sl_setMetainfoBase()));
            Signal *pSignal = findSignal(pPICS->getSignal());
            pSignal->attach(pNewOp);
            Operation* op = pPICS->getOperation();
            //if(op==NULL){
                pPICS->setOperation(pSignal->getSignal());
            //}
            pPICS->update(true);
            updatingItem = false;

        }
        else {
            updatingItem = true;
            EDPICSNode *pPICS = dynamic_cast<EDPICSNode*>(pParent);
            Operation* pParentOp = pPICS->getOperation();
            int id;
            for (id=0; id<pParentOp->getArgumentNumber(); id++) {
                if (pParentOp->getArgument(id) == pOp)
                    break;
            }
            assert( id >=0 && id < pParentOp->getArgumentNumber() );
            pParentOp->setArgument(pNewOp, id);
            connect(pParent, SIGNAL(si_getMetaInfoBase()), SLOT(sl_setMetainfoBase()));
            pParent->update(true);
            updatingItem = false;
        }
        delete pOp;
        const EDProjectItem *pNewItemc = pParent->findItemConnectedTo(pNewOp);
        EDProjectItem *pNewItem = const_cast<EDProjectItem*>(pNewItemc);
        connect(pNewItem, SIGNAL(si_getMetaInfoBase()), SLOT(sl_setMetainfoBase()));
        updateTree(ED_UPDATE_CHILDREN, qobject_cast<EDProjectItem *>(pParent));
        updateTree(ED_CURRENT_ITEM_CHANGED, pNewItem);
    }
    else {
        switch (pCSN->getType()) {
        case PIT_CSN_DISTANCE    : onDistancePropertyChanged(pCSN, pProperty, strNewValue); break;
        case PIT_CSN_REPETITION    : onRepetitionPropertyChanged(pCSN, pProperty, strNewValue); break;
        case PIT_CSN_INTERVAL    : onIntervalPropertyChanged(pCSN, pProperty, strNewValue); break;
        case PIT_CSN_WORD        : onWordPropertyChanged(pCSN, pProperty, strNewValue); break;
        case PIT_CSN_MRK_ITEM    : onMrkItemPropertyChanged(pCSN, pProperty, strNewValue); break;
        default: assert(0);
        }
    }

    while (pParent->getType() != PIT_CS) {
        pParent->update(false);

        //pParent = findEDItem(dynamic_cast<EDProjectItem*>(dynamic_cast<QTreeWidgetItem*>(pParent)->parent()));
        pParent = dynamic_cast<EDProjectItem*>(dynamic_cast<QTreeWidgetItem*>(pParent)->parent());
        connect(pParent, SIGNAL(si_getMetaInfoBase()), SLOT(sl_setMetainfoBase()));
    }
    pParent->update(false);
    if(edData.isSignalSelected(pItem))
        edData.clearScores();
}
void EDProjectTree::onDistancePropertyChanged(EDProjectItem* pItem, const EDPIProperty* pProperty, QString strNewValue){
    EDPICSNDistance* pDist = dynamic_cast<EDPICSNDistance*>(pItem);
    OpDistance* pOp = dynamic_cast<OpDistance*>(pDist->getOperation());
    QString strFrom = "Distance from";
    QString strTo = "Distance to";
    QString strOrder = "Order";
    QString strDistanceType = "Distance type";

    if (pProperty->getName().compare(strFrom, Qt::CaseInsensitive) == 0) {
        DDisc::Interval iDist = pOp->getDistance();
        int nValue =0;
        if (parse(strNewValue.toStdString().c_str(),"%d", &nValue) && nValue>=0 && nValue<=iDist.getTo()) {
            iDist.setFrom(nValue);
            pOp->setDistance(iDist);
        }
        else{
            QObjectScopedPointer<QMessageBox> mb = new QMessageBox(QMessageBox::Critical, tr("Error"), tr("The value must be positive integer which is less than the higher bound"));
            mb->exec();
            CHECK(!mb.isNull(), );
        }
    }
    else
        if (pProperty->getName().compare(strTo, Qt::CaseInsensitive) == 0) {
            DDisc::Interval iDist = pOp->getDistance();
            int nId = EDPIPropertyTypeUnsignedIntWithUnl::getInstance()->getValueId(strNewValue);
            int nValue =0;
            bool bError = false;
            if (nId < 0) {
                if (bError = !parse(strNewValue.toStdString().c_str(),"%d", &nValue) || nValue<0) {
                    QObjectScopedPointer<QMessageBox> mb = new QMessageBox(QMessageBox::Critical, tr("Error"), tr("The value must be positive integer which is less than the higher bound"));
                    mb->exec();
                    CHECK(!mb.isNull(), );
                }
            }
            else
                nValue = PINF;
            if (!bError) {
                if (nValue >= iDist.getFrom()) {
                    iDist.setTo(nValue);
                    pOp->setDistance(iDist);
                }
                else{
                    QObjectScopedPointer<QMessageBox> mb = new QMessageBox(QMessageBox::Critical, tr("Error"), tr("The value must be grater than the lower bound"));
                    mb->exec();
                    CHECK(!mb.isNull(), );
                }
            }
        } else
            if (pProperty->getName().compare(strOrder, Qt::CaseInsensitive) == 0) {
                int nId = EDPIPropertyTypeBool::getInstance()->getValueId(strNewValue);
                if (nId == EDPIPropertyTypeBool::False)
                    pOp->setOrderImportant(false);
                else
                    pOp->setOrderImportant(true);
            } else
                if (pProperty->getName().compare(strDistanceType, Qt::CaseInsensitive) == 0) {
                    int nId = EDPIPropertyTypeDistType::getInstance()->getValueId(strNewValue);
                    pOp->setDistanceType((EDistType)nId);
                }
                else assert(0);
                pDist->update(true);
                EDProjectItem* pIt = dynamic_cast<EDProjectItem*>(dynamic_cast<QTreeWidgetItem*>(pDist)->parent());
                updateTree(ED_UPDATE_CHILDREN, qobject_cast<EDProjectItem *>(pIt));
                updateTree(ED_CURRENT_ITEM_CHANGED, pDist);
}

void EDProjectTree::onRepetitionPropertyChanged(EDProjectItem* pItem, const EDPIProperty* pProperty, QString strNewValue){
    EDPICSNRepetition* pDist = dynamic_cast<EDPICSNRepetition*>(pItem);
    OpReiteration* pOp = dynamic_cast<OpReiteration*>(pDist->getOperation());
    QString strFrom = "Distance from";
    QString strTo = "Distance to";
    QString strDistanceType = "Distance type";
    QString strCountFrom = "Count from";
    QString strCountTo = "Count to";


    if (pProperty->getName().compare(strFrom, Qt::CaseInsensitive) == 0) {
        DDisc::Interval iDist = pOp->getDistance();
        int nValue =0;
        if (parse(strNewValue.toStdString().c_str(),"%d", &nValue) && nValue>=0 && nValue<=iDist.getTo()) {
            iDist.setFrom(nValue);
            pOp->setDistance(iDist);
        }
        else{
            QObjectScopedPointer<QMessageBox> mb = new QMessageBox(QMessageBox::Critical, tr("Error"), tr("The value must be positive integer which is less than the higher bound"));
            mb->exec();
            CHECK(!mb.isNull(), );
        }
    }
    else
        if (pProperty->getName().compare(strTo, Qt::CaseInsensitive) == 0) {
            DDisc::Interval iDist = pOp->getDistance();
            int nId = EDPIPropertyTypeUnsignedIntWithUnl::getInstance()->getValueId(strNewValue);
            int nValue =0;
            bool bError = false;
            if (nId < 0) {
                if (bError = !parse(strNewValue.toStdString().c_str(),"%d", &nValue)) {
                    QObjectScopedPointer<QMessageBox> mb = new QMessageBox(QMessageBox::Critical, tr("Error"), tr("The value must be positive integer which is less than the higher bound"));
                    mb->exec();
                    CHECK(!mb.isNull(), );
                }
            }
            else
                nValue = PINF;
            if (!bError) {
                if (nValue >= iDist.getFrom()) {
                    iDist.setTo(nValue);
                    pOp->setDistance(iDist);
                }
                else{
                    QObjectScopedPointer<QMessageBox> mb = new QMessageBox(QMessageBox::Critical, tr("Error"), tr("The value must be grater than the lower bound"));
                    mb->exec();
                    CHECK(!mb.isNull(), );
                }
            }
        } else
            if (pProperty->getName().compare(strCountFrom, Qt::CaseInsensitive) == 0) {
                DDisc::Interval iCount = pOp->getCount();
                int nValue =0;
                if (parse(strNewValue.toStdString().c_str(),"%d", &nValue) && nValue>=0 && nValue<=iCount.getTo() && nValue<=10) {
                    iCount.setFrom(nValue);
                    pOp->setCount(iCount);
                }
                else {
                    QObjectScopedPointer<QMessageBox> mb = new QMessageBox(QMessageBox::Critical, tr("Error"), tr("The value must be positive integer which is less than the higher bound"));
                    mb->exec();
                    CHECK(!mb.isNull(), );
                }
            }
            else
                if (pProperty->getName().compare(strCountTo, Qt::CaseInsensitive) == 0) {
                    DDisc::Interval iCount = pOp->getCount();
                    int nValue =0;
                    if (!parse(strNewValue.toStdString().c_str(),"%d", &nValue)) {
                        QObjectScopedPointer<QMessageBox> mb = new QMessageBox(QMessageBox::Critical, tr("Error"), tr("The value must be positive integer which is less than the higher bound"));
                        mb->exec();
                        CHECK(!mb.isNull(), );
                    }
                    else {
                        if (nValue >= iCount.getFrom()) {
                            iCount.setTo(nValue);
                            pOp->setCount(iCount);
                        }
                        else{
                            QObjectScopedPointer<QMessageBox> mb = new QMessageBox(QMessageBox::Critical, tr("Error"), tr("The value must be grater than the lower bound"));
                            mb->exec();
                            CHECK(!mb.isNull(), );
                        }
                    }
                } else
                    if (pProperty->getName().compare(strDistanceType, Qt::CaseInsensitive) == 0) {
                        int nId = EDPIPropertyTypeDistType::getInstance()->getValueId(strNewValue);
                        pOp->setDistanceType((EDistType)nId);
                    }
                    else assert(0);
                    pDist->update(true);
                    EDProjectItem* pIt = dynamic_cast<EDProjectItem*>(dynamic_cast<QTreeWidgetItem*>(pDist)->parent());
                    updateTree(ED_UPDATE_CHILDREN, qobject_cast<EDProjectItem *>(pIt));
                    updateTree(ED_CURRENT_ITEM_CHANGED, pDist);
}

void EDProjectTree::onIntervalPropertyChanged(EDProjectItem* pItem, const EDPIProperty* pProperty, QString strNewValue){
    EDPICSNInterval* pInt = dynamic_cast<EDPICSNInterval*>(pItem);
    OpInterval* pOp = dynamic_cast<OpInterval*>(pInt->getOperation());
    QString strFrom = "Distance from";
    QString strTo = "Distance to";

    if (pProperty->getName().compare(strFrom, Qt::CaseInsensitive) == 0) {
        DDisc::Interval iInt = pOp->getInt();
        int nValue =0;
        if (parse(strNewValue.toStdString().c_str(),"%d", &nValue) && nValue>=0 && nValue<=iInt.getTo()) {
            iInt.setFrom(nValue);
            pOp->setInt(iInt);
        }
        else{
            QObjectScopedPointer<QMessageBox> mb = new QMessageBox(QMessageBox::Critical, tr("Error"), tr("The value must be positive integer which is less than the higher bound"));
            mb->exec();
            CHECK(!mb.isNull(), );
        }
    }
    else
        if (pProperty->getName().compare(strTo, Qt::CaseInsensitive) == 0) {
            DDisc::Interval iInt = pOp->getInt();
            int nId = EDPIPropertyTypeUnsignedIntWithUnl::getInstance()->getValueId(strNewValue);
            int nValue =0;
            bool bError = false;
            if (nId < 0) {
                if (bError = !parse(strNewValue.toStdString().c_str(),"%d", &nValue)) {
                    QObjectScopedPointer<QMessageBox> mb = new QMessageBox(QMessageBox::Critical, tr("Error"), tr("The value must be positive integer which is less than the higher bound"));
                    mb->exec();
                    CHECK(!mb.isNull(), );
                }
            }
            else
                nValue = PINF;
            if (!bError) {
                if (nValue >= iInt.getFrom()) {
                    iInt.setTo(nValue);
                    pOp->setInt(iInt);
                }
                else{
                    QObjectScopedPointer<QMessageBox> mb = new QMessageBox(QMessageBox::Critical, tr("Error"), tr("The value must be grater than the lower bound"));
                    mb->exec();
                    CHECK(!mb.isNull(), );
                }
            }
        }
        else assert(0);
        pInt->update(true);
        EDProjectItem* pIt = dynamic_cast<EDProjectItem*>(dynamic_cast<QTreeWidgetItem*>(pInt)->parent());
        updateTree(ED_UPDATE_CHILDREN, qobject_cast<EDProjectItem *>(pIt));
        updateTree(ED_CURRENT_ITEM_CHANGED, pInt);
}

void EDProjectTree::onWordPropertyChanged(EDProjectItem* pItem, const EDPIProperty* pProperty, QString strNewValue){
    EDPICSNTSWord* pPITS = dynamic_cast<EDPICSNTSWord*>(pItem);
    TS* pOp = dynamic_cast<TS*>(pPITS->getOperation());
    QString strWord = "Word";

    if (pProperty->getName().compare(strWord, Qt::CaseInsensitive) == 0) {
        if (isValidWord15(strNewValue.toStdString().c_str()))
            pOp->setWord(strNewValue.toUpper().toStdString());
        else {
            QObjectScopedPointer<QMessageBox> mb = new QMessageBox(QMessageBox::Critical, tr("Error"), tr("The word must be in 15-character code"));
            mb->exec();
            return;
        }

    }
    else assert(0);

    pPITS->update(true);
    EDProjectItem* pIt = dynamic_cast<EDProjectItem*>(dynamic_cast<QTreeWidgetItem*>(pPITS)->parent());
    updateTree(ED_UPDATE_CHILDREN, qobject_cast<EDProjectItem *>(pIt));
    updateTree(ED_CURRENT_ITEM_CHANGED, pPITS);
}
void EDProjectTree::onMrkItemPropertyChanged(EDProjectItem* pItem, const EDPIProperty* pProperty, QString strNewValue){
    QString strFamily = "Family";
    QString strSignal = "Signal";

    EDPICSNTSMrkItem* pPITS = dynamic_cast<EDPICSNTSMrkItem*>(pItem);
    assert(pPITS!=NULL);
    TS *pTS = dynamic_cast<TS*>(pPITS->getOperation());
    assert(pTS!=NULL);

    if (pProperty->getName().compare(strFamily, Qt::CaseInsensitive) == 0) {
        pTS->setFamily(strNewValue.toStdString().c_str());
    }
    else
        if (pProperty->getName().compare(strSignal, Qt::CaseInsensitive) == 0) {
            pTS->setName(strNewValue.toStdString().c_str());
        }
        else
            assert(0);

    pPITS->update(true);
    EDProjectItem* pIt = dynamic_cast<EDProjectItem*>(dynamic_cast<QTreeWidgetItem*>(pPITS)->parent());
    updateTree(ED_UPDATE_CHILDREN, qobject_cast<EDProjectItem *>(pIt));
    updateTree(ED_CURRENT_ITEM_CHANGED, pPITS);
    emit si_changeProp(pPITS);
}

Operation* EDProjectTree::createCSN(int ValueId) const{
    switch (ValueId) {
    case EDPIPropertyTypeListCSNodeTypes::DISTANCE    : return new OpDistance;
    case EDPIPropertyTypeListCSNodeTypes::REPETITION    : return new OpReiteration;
    case EDPIPropertyTypeListCSNodeTypes::INTERVAL    : return new OpInterval;
    case EDPIPropertyTypeListCSNodeTypes::WORD        :
        {
            TS* pTS = new TS;
            pTS->setFromMarking(false);
            return pTS;
        }
    case EDPIPropertyTypeListCSNodeTypes::MRK_ITEM    :
        {
            TS* pTS = new TS;
            pTS->setFromMarking(true);
            return pTS;
        }
    }
    return NULL;
}

void EDProjectTree::sl_newFolder(){
    QTreeWidgetItem* curItem = currentItem();
    EDPICSDirectory* pItem = dynamic_cast<EDPICSDirectory*>(curItem);
    assert(pItem);
    CSFolder* pFolder = findFolder(pItem);
    CSFolder* pNewFolder = new CSFolder();
    pNewFolder->setName(pFolder->makeUniqueFolderName());
    if (pFolder->addFolder( pNewFolder ) < 0) {
        delete pNewFolder;
        return;
    }
    EDPICSFolder* newFol = new EDPICSFolder(pNewFolder);
    pItem->addChild(dynamic_cast<EDProjectItem*>(newFol));
    updateTree(ED_UPDATE_CHILDREN, pItem);
    updateTree(ED_CURRENT_ITEM_CHANGED, newFol);
}
void EDProjectTree::sl_newSignal(){

    QTreeWidgetItem* curItem = currentItem();
    EDPICSDirectory* pItem = dynamic_cast<EDPICSDirectory*>(curItem);
    assert(pItem);
    CSFolder* pFolder = findFolder(pItem);
    Signal *pNewSignal = new Signal(NULL);
    pNewSignal->setName(pFolder->makeUniqueSignalName().toStdString());
    if (pFolder->addSignal( pNewSignal ) < 0) {
                delete pNewSignal;
                return;
    }
    EDPICS* pNewItem =  new EDPICS(pNewSignal);
    pItem->addChild(dynamic_cast<EDProjectItem*>(pNewItem));
    updateTree(ED_UPDATE_CHILDREN, pItem);
    updateTree(ED_CURRENT_ITEM_CHANGED, pNewItem);
}
void EDProjectTree::sl_deletePI(){

    QTreeWidgetItem* curItem = currentItem();
    EDProjectItem* pItem= dynamic_cast<EDProjectItem*>(curItem);
    assert(pItem);
    switch (pItem->getType()){
        case PIT_CS_FOLDER: deleteFolder(dynamic_cast<EDPICSFolder*>(pItem)); break;
        case PIT_CS: deleteSignal(dynamic_cast<EDPICS*>(pItem)); break;
        default: assert(0);
    }
}

void EDProjectTree::deleteFolder(EDPICSFolder* pPI){
    EDProjectItem *pItem = dynamic_cast<EDProjectItem*>(dynamic_cast<QTreeWidgetItem*>(pPI)->parent());
    CSFolder* pFolder = findFolder(dynamic_cast<EDPICSDirectory*>(pItem));
    assert(pFolder != NULL);
    int id = pFolder->getFolderIndexByName(pPI->getName());
    assert(id>=0);
    pFolder->deleteFolder(id);

    updateTree(ED_CURRENT_ITEM_CHANGED, NULL);
    pItem->update(true);
    updateTree(ED_UPDATE_CHILDREN, pItem);
    edData.clearScores();
}
void EDProjectTree::deleteSignal(EDPICS* pPI){

    if(edData.isSignalSelected(pPI)){
        edData.clearScores();
    }
    EDProjectItem *pItem = dynamic_cast<EDProjectItem*>(dynamic_cast<QTreeWidgetItem*>(pPI)->parent());
    CSFolder* pFolder = findFolder(dynamic_cast<EDPICSDirectory*>(pItem));
    assert(pFolder != NULL);
    int id = pFolder->getSignalIndexByName(pPI->getName());
    assert(id>=0);
    pFolder->deleteSignal(id);

    updateTree(ED_CURRENT_ITEM_CHANGED, NULL);
    pItem->update(true);
    updateTree(ED_UPDATE_CHILDREN, pItem);

}

void EDProjectTree::updateSorting(){


//     for (int i = 0; i < seqRoot.childCount(); i++){
//         EDProjectItem* item = dynamic_cast<EDProjectItem*>(seqRoot.child(i));
//         if(item){
//            item->setSortField(sortField);
//            item->setSortOrd(sortOrd);
//            updateSortingRecurs(item);
//         }
//     }
//
//     for (int i = 0; i < mrkRoot.childCount(); i++){
//         EDProjectItem* item = dynamic_cast<EDProjectItem*>(mrkRoot.child(i));
//         if(item){
//             item->setSortField(sortField);
//             item->setSortOrd(sortOrd);
//             updateSortingRecurs(item);
//         }
//     }

    for (int i = 0; i < root.childCount(); i++){
        EDProjectItem* item = dynamic_cast<EDProjectItem*>(root.child(i));
        if(item){
            item->setSortField(sortField);
            item->setSortOrd(sortOrd);
            updateSortingRecurs(item);
        }
    }
//     seqRoot.sortChildren(0, Qt::AscendingOrder);
//     mrkRoot.sortChildren(0, Qt::AscendingOrder);
    root.sortChildren(0, Qt::AscendingOrder);

}

void EDProjectTree::updateSortingRecurs(EDProjectItem* pItem){

    for (int i = 0; i < pItem->childCount(); i++){
        EDProjectItem* item = dynamic_cast<EDProjectItem*>(pItem->child(i));
        if(item){
            item->setSortField(sortField);
            item->setSortOrd(sortOrd);
            updateSortingRecurs(item);
        }
    }
    pItem->sortChildren(0, Qt::AscendingOrder);

}
void EDProjectTree::sl_selAllSig(){
    EDProjectItem* pItem = dynamic_cast<EDProjectItem*>(currentItem());
    if (!pItem)
        return;
    for (int i=0; i<pItem->childCount(); i++)
    {
        EDProjectItem* pSI = dynamic_cast<EDProjectItem*>(pItem->child(i));
        if(!pSI){
            continue;
        }
        if (!edData.isSignalSelected(pSI))
            edData.switchSelection(const_cast<EDProjectItem*>(pSI), true);

        updateTree(ED_ITEM_STATE_CHANGED, pSI);
    }
}
void EDProjectTree::sl_deselAllSig(){
    EDProjectItem* pItem = dynamic_cast<EDProjectItem*>(currentItem());
    if (!pItem)
        return;
    for (int i=0; i<pItem->childCount(); i++)
    {
        EDProjectItem* pSI = dynamic_cast<EDProjectItem*>(pItem->child(i));
        if(!pSI){
            continue;
        }
        if (edData.isSignalSelected(pSI))
               edData.switchSelection(const_cast<EDProjectItem*>(pSI), true);
        updateTree(ED_ITEM_STATE_CHANGED, pSI);
    }
}
void EDProjectTree::sl_setPriorAllSig(){
    EDProjectItem* pItem = dynamic_cast<EDProjectItem*>(currentItem());
    if (!pItem)
        return;
    for (int i=0; i<pItem->childCount(); i++)
    {
        EDProjectItem* pSI = dynamic_cast<EDProjectItem*>(pItem->child(i));
        if(!pSI){
            continue;
        }
        EDPICS* pCS = dynamic_cast<EDPICS*>(pSI);
        if (pCS){
            edData.onSetCurrentSignalParamsAsPrior(pCS, false);
            updateTree(ED_CURRENT_ITEM_CHANGED, pCS);
        }
    }
}
void EDProjectTree::sl_clearPriorAllSig(){
    EDProjectItem* pItem = dynamic_cast<EDProjectItem*>(currentItem());
    if (!pItem)
        return;
    for (int i=0; i<pItem->childCount(); i++)
    {
        EDProjectItem* pSI = dynamic_cast<EDProjectItem*>(pItem->child(i));
        if(!pSI){
            continue;
        }
        EDPICS* pCS = dynamic_cast<EDPICS*>(pSI);
        if (pCS){
            edData.onClearSignalPriorParams(pCS);
            updateTree(ED_CURRENT_ITEM_CHANGED, pCS);
        }
    }
}
void EDProjectTree::sl_addSignalToMarkup(){
    emit si_addToMarkup();
}
void EDProjectTree::sl_selDeselSig(){
    EDProjectItem* pItem = dynamic_cast<EDProjectItem*>(currentItem());
    if (pItem){
         edData.switchSelection(pItem, true);
         updateTree(ED_ITEM_STATE_CHANGED, pItem);
    }
}
void EDProjectTree::sl_setCurPrior(){
    EDPICS* pItem = dynamic_cast<EDPICS*>(currentItem());
    if (!pItem)
    {
        assert(0);
        return;
    }
    edData.onSetCurrentSignalParamsAsPrior(pItem, true);
    updateTree(ED_CURRENT_ITEM_CHANGED, pItem);
}
void EDProjectTree::sl_clearCurPrior(){
     EDPICS* pItem = dynamic_cast<EDPICS*>(currentItem());
    if (!pItem)
    {
        assert(0);
        return;
    }
    edData.onClearSignalPriorParams(pItem);
    updateTree(ED_CURRENT_ITEM_CHANGED, pItem);
}

void EDProjectTree::sl_markupLetters(){
    edData.markupLetters();
    updateMarkup();
    emit si_loadMarkup(true);
}

void EDProjectTree::sl_loadMarkup(){
    emit si_loadMarkup(false);
}
void EDProjectTree::sl_showSequence(){
    emit si_showSequence();
}
void EDProjectTree::sl_addToShown(){
    emit si_addToShown();
}

void EDProjectTree::sl_clearDisplayed(){
    emit si_clearDisplayed();
}
void EDProjectTree::sl_showFirstSequences(){
    emit si_showFirstSequences();
}

void EDProjectTree::sl_setMetainfoBase(){
    EDProjectItem* item = dynamic_cast<EDProjectItem*>(sender());

    if(!item){
        return;
    }

    item->setMetainfoBase(&edData.getDescriptionBase());

}

void EDProjectTree::sl_generateReport(){
    EDPISequenceBase* pItem = dynamic_cast<EDPISequenceBase*>(currentItem());
    if (!pItem)
    {
        return;
    }
    edData.generateRecognizationReport(pItem);
}

void EDProjectTree::sl_exportSequences(){
    EDPISequenceBase* pItem = dynamic_cast<EDPISequenceBase*>(currentItem());
    if (!pItem)
    {
        return;
    }
    //edData.generateRecognizationReport(pItem);

    ExpertDiscoveryExportSequences *t = new ExpertDiscoveryExportSequences(pItem->getSequenceBase());

    AppContext::getTaskScheduler()->registerTopLevelTask(t);
//     QFileDialog saveRepDialog;
//     saveRepDialog.setFileMode(QFileDialog::AnyFile);
//     saveRepDialog.setNameFilter(tr("Fasta Files (*.fa *.fasta)"));
//     saveRepDialog.setViewMode(QFileDialog::Detail);
//     saveRepDialog.setAcceptMode(QFileDialog::AcceptSave);
//
//     if(saveRepDialog.exec()){
//         QStringList fileNames = saveRepDialog.selectedFiles();
//         if(fileNames.isEmpty()) return;
//
//         QString fileName = fileNames.first();
//
//         GUrl URL(strNegName);
//         IOAdapterFactory* iof = AppContext::getIOAdapterRegistry()->getIOAdapterFactoryById(IOAdapterUtils::url2io(URL));
//         DocumentFormat* f = AppContext::getDocumentFormatRegistry()->getFormatById(BaseDocumentFormats::PLAIN_GENBANK);
//
//         negDoc = f->createNewUnloadedDocument(iof, URL, stateInfo);
//         CHECK_OP(stateInfo,);
//         addSubTask(new LoadUnloadedDocumentTask(negDoc));
}

void EDProjectTree::sl_sortField(QAction* action){
    EDSortParameters cursortField = ED_FIELD_UNDEFINED;
    QString actName = action->text();
    if(actName == tr("Coverage")){
        cursortField = ED_FIELD_COVERAGE;
    }else if(actName == tr("Fisher")){
        cursortField = ED_FIELD_FISHER;
    }else if(actName == tr("Name")){
        cursortField = ED_FIELD_NAME;
    }else if(actName == tr("Probability")){
        cursortField = ED_FIELD_PROBABILITY;
    }

    if(cursortField != ED_FIELD_UNDEFINED){
       sortField = cursortField;
       updateSorting();
    }
}
void EDProjectTree::sl_sortOrd(QAction* action){
    EDSortParameters cursortField = ED_FIELD_UNDEFINED;
    QString actName = action->text();
    if(actName == tr("Increasing")){
        cursortField = ED_ORDER_INCREASING;
    }else if(actName == tr("Decreasing")){
        cursortField = ED_ORDER_DECREASING;
    }

    if(cursortField != ED_FIELD_UNDEFINED){
        sortOrd = cursortField;
        updateSorting();
    }
}
}//namespace
