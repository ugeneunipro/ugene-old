#include "ExpertDiscoveryTreeWidgetsCtrl.h"



#include<QMessageBox>
#include <QtGui/QMouseEvent>



namespace U2 {
 
EDProjectTree::EDProjectTree(QWidget *parent, ExpertDiscoveryData &d)
:QTreeWidget(parent)
,edData(d)
,root(edData.getRootFolder())
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
    case ED_UPDATE_ALL				:	remake();			break;
    case ED_ITEM_NAME_CHANGED		:	updateItem(item);	break;
    case ED_ITEM_STATE_CHANGED		:	updateItemState(item);	break;
    case ED_ITEM_ADDED				:	internalRemake(item, dynamic_cast<EDProjectItem*>(dynamic_cast<QTreeWidgetItem*>(item)->parent()));	break;
    case ED_ITEM_DELETED			:	/*DeleteItem(pItem);*/  break;
    case ED_CURRENT_ITEM_CHANGED	:	setCurrentItem(item); break;
    case ED_UPDATE_CHILDREN		    :	updateChildren(item); break;
    case ED_MRK_UPDATE              :   updateMarkup(); break; 
    };
}

void EDProjectTree::remake(){
    //clear();

    addTopLevelItem(&mrkRoot);
    mrkRoot.setText(0,"Markup");
    mrkRoot.updMarkup(edData);
    for (int i = 0; i < mrkRoot.childCount(); i++){
        EDProjectItem* item = dynamic_cast<EDProjectItem*>(mrkRoot.child(i));
        if(item){
            internalRemake(item, &mrkRoot);
        }
    }

    addTopLevelItem(&root);
    root.setText(0,"Complex signals");
    for (int i = 0; i < root.childCount(); i++){
        EDProjectItem* item = dynamic_cast<EDProjectItem*>(root.child(i));
        if(item){
            internalRemake(item, &root);
        }
    }

}

void EDProjectTree::internalRemake(EDProjectItem* subItem, EDProjectItem* parent){
    /*int nImage = TypeToImage(pItem->GetType());
    HTREEITEM hItem = InsertItem(pItem->GetName(), nImage, nImage, hParent);
    SetItemData(hItem, (DWORD_PTR) pItem);
    pItem->SetTreeItem(hItem);
    if (pItem->IsSelected())
        SetItemState(hItem, TVIS_BOLD, TVIS_BOLD);
    else
        SetItemState(hItem, 0, TVIS_BOLD);
    int nSubitemsNumber = pItem->GetSubitemsNumber();
    for (int i=0; i<nSubitemsNumber; i++)
        InternalRemake(hItem, pItem->GetSubitem(i));
    SortChildItems(hItem, m_sortingMode, m_order);   */
    subItem->setText(0, subItem->getName());

    for(int i = 0; i < subItem->childCount(); i++){
        EDProjectItem* item = dynamic_cast<EDProjectItem*>(subItem->child(i));
        if (item){
            connect(item, SIGNAL(si_getMetaInfoBase()), SLOT(sl_setMetainfoBase()));
            internalRemake(item, subItem);
        }
        
    }
}

void EDProjectTree::updateItem(EDProjectItem* pItem){
    pItem->setText(0, pItem->getName());
    /*SetItemText(hItem, pItem->GetName());
    SortChildren(GetParentItem(hItem));
    int nImage = TypeToImage(pItem->GetType());
    SetItemImage(hItem, nImage, nImage);*/
    QFont curFont = pItem->font(0);
    if (edData.isSignalSelected(pItem))
        curFont.setBold(true);
    else
        curFont.setBold(false);

    pItem->setFont(0, curFont);
}
void EDProjectTree::updateItemState(EDProjectItem *pItem)
{
    QFont curFont = pItem->font(0);
    if (edData.isSignalSelected(pItem))
        curFont.setBold(true);
    else
        curFont.setBold(false);

    pItem->setFont(0, curFont);
}


void EDProjectTree::updateChildren(EDProjectItem* pItem){
    //updateItem(pItem);
   
    //SelectItem(NULL);
    //while (hChild = GetChildItem(hParent)) CRGTreeCtrl::DeleteItem(hChild);
    for (int i=0; i<pItem->childCount(); i++){
        EDProjectItem* ch = dynamic_cast<EDProjectItem*>(pItem->child(i));
        internalRemake(ch, pItem);

    }
    //SortChildItems(hParent, m_sortingMode, m_order);*/
}

void EDProjectTree::updateMarkup(){
    mrkRoot.updMarkup(edData);
    for (int i = 0; i < mrkRoot.childCount(); i++){
        EDProjectItem* item = dynamic_cast<EDProjectItem*>(mrkRoot.child(i));
        if(item){
  //          connect(item, SIGNAL(si_getMetaInfoBase()), SLOT(sl_setMetainfoBase()));
            internalRemake(item, &mrkRoot);
        }
    }    
}

void EDProjectTree::sl_propChanged(EDProjectItem* item, const EDPIProperty* prop, QString newVal){
    switch (item->getType()) {
        case PIT_CS_FOLDER: 
            if (prop->getName().compare("Name", Qt::CaseInsensitive) == NULL) {
                EDPICSDirectory* pPI = dynamic_cast<EDPICSDirectory*>(item);
                CSFolder* pFolder = findFolder(pPI);
                const CSFolder* pParentFolder = pFolder->getParentFolder();
                int nIndex = pParentFolder->getFolderIndexByName(newVal);
                if (nIndex>=0) {
                    QMessageBox mb(QMessageBox::Critical, tr("Rename error"), tr("Folder already exist"));
                    mb.exec();
                    return;
                }
                pFolder->setName(newVal);
                updateTree(ED_ITEM_NAME_CHANGED, (EDProjectItem *)pPI);
                updateTree(ED_CURRENT_ITEM_CHANGED, (EDProjectItem *) pPI);
            }
            return;
        case PIT_CS:{
            
           //EDPICSDirectory *pParent = dynamic_cast<EDPICSDirectory*>(dynamic_cast<EDProjectItem*>(item->parent()));
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
                    QMessageBox mb(QMessageBox::Question, tr("Signal rename"), tr("Signal with the same name already exist. Replace?"), QMessageBox::Ok|QMessageBox::Cancel);
                    if(mb.exec()==QMessageBox::Ok)
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
            EDProjectItem* pCurItem = const_cast<EDProjectItem*>(pParent->findItemConnectedTo(pSignal));
            updateTree(ED_UPDATE_CHILDREN, (EDProjectItem* ) pParent);
            updateTree(ED_CURRENT_ITEM_CHANGED, pCurItem);
            return;
                    }
        case PIT_CSN_UNDEFINED:
        case PIT_CSN_DISTANCE:
        case PIT_CSN_REPETITION:
        case PIT_CSN_INTERVAL:
        case PIT_CSN_WORD:
        case PIT_CSN_MRK_ITEM:
            //OnCSNPropertyChanged(theEvent.GetProjectItem(), theEvent.GetProperty(), theEvent.GetNewValue());
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

QMenu* EDProjectTree::chosePopupMen(EDProjectItem* pItem){
    if(pItem->getType() == PIT_CS){
        EDPICS* pItem = dynamic_cast<EDPICS*>(currentItem());
        if(pItem){
            if(!edData.isSignalSelected(pItem)){
                selDeselSigAction->setText("Select signal");
            }else{
                selDeselSigAction->setText("Deselect signal");
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
        return popupMenuMrkRoot;  
    }
    return NULL;
      
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

    selDeselSigAction = new QAction(tr("Select/deselect signal"), this);
    connect(selDeselSigAction, SIGNAL(triggered(bool)), SLOT(sl_selDeselSig()));

    setCurPriorAction = new QAction(tr("Set current params as prior"), this);
    connect(setCurPriorAction, SIGNAL(triggered(bool)), SLOT(sl_setCurPrior()));

    clearCurPriorAction = new QAction(tr("Clear prior params"), this);
    connect(clearCurPriorAction, SIGNAL(triggered(bool)), SLOT(sl_clearCurPrior()));

    markupLettersAction = new QAction(tr("Markup letters"), this);
    connect(markupLettersAction, SIGNAL(triggered(bool)), SLOT(sl_markupLetters()));
    
   
 
    popupMenuCS = new QMenu(this);
    popupMenuCS->addAction(selDeselSigAction);
    popupMenuCS->addAction(deletePIAction);
    popupMenuCS->addSeparator();
    popupMenuCS->addAction(setCurPriorAction);
    popupMenuCS->addAction(clearCurPriorAction);

    popupMenuDir = new QMenu(this);
    popupMenuDir->addAction(newFolderAction);
    popupMenuDir->addAction(newSignalAction);
    popupMenuDir->addSeparator();
    popupMenuDir->addAction(deletePIAction);
    popupMenuDir->addSeparator();
    popupMenuDir->addAction(selAllSigAction);
    popupMenuDir->addAction(deselAllSigAction);
    popupMenuDir->addAction(setPriorAllSigAction);
    popupMenuDir->addAction(clearPriorAllSigAction);

    popupMenuDirRoot = new QMenu(this);
    popupMenuDirRoot->addAction(newFolderAction);
    popupMenuDirRoot->addAction(newSignalAction);
    popupMenuDirRoot->addSeparator();
    popupMenuDirRoot->addAction(selAllSigAction);
    popupMenuDirRoot->addAction(deselAllSigAction);
    popupMenuDirRoot->addAction(setPriorAllSigAction);
    popupMenuDirRoot->addAction(clearPriorAllSigAction);

    popupMenuMrkRoot = new QMenu(this);
    popupMenuMrkRoot->addAction(markupLettersAction);
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
    //updateTree(ED_CURRENT_ITEM_CHANGED, pNewItem);
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
}

void EDProjectTree::sl_setMetainfoBase(){
    EDProjectItem* item = dynamic_cast<EDProjectItem*>(sender());

    if(!item){
        return;
    }

    item->setMetainfoBase(&edData.getDescriptionBase());
    
}
}//namespace