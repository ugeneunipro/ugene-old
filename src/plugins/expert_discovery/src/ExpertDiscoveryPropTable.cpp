#include "ExpertDiscoveryPropTable.h"

#include <QHeaderView>
#include <QtGui/QMessageBox>

namespace U2 {

EDPropertiesTable::EDPropertiesTable(QWidget* parent)
:QTableWidget(parent)
,curPItem(NULL)
,isSeq(false)
,seqOffset(0)
{
    setColumnCount(2);   

    verticalHeader()->hide();
    horizontalHeader()->hide();
    //connect(this, SIGNAL(cellChanged ( int , int  )), SLOT(sl_cellDataChanged(int , int )));
    connect(this, SIGNAL(itemChanged ( QTableWidgetItem*  )), SLOT( sl_cellChanged(QTableWidgetItem*) ));
    setWordWrap(true);
}

void EDPropertiesTable::representPIProperties(EDProjectItem* pItem){
    curPItem = pItem;
   
    cleanup();

    if(pItem == NULL){
        return;
    }

    int nGroupNum = pItem->getGroupNumber();
    for (int nGroup=0; nGroup<nGroupNum; nGroup++) {
        const EDPIPropertyGroup& rGroup = pItem->getGroup(nGroup);
        addNewGroup(rGroup.getName());

        int nPropNum = rGroup.getPropertiesNumber();
        for(int nProp=0; nProp<nPropNum; nProp++) {
            const EDPIProperty& rProp = rGroup.getProperty(nProp);
            const EDPIPropertyType* pType = rProp.getType();
            assert( pType != NULL );
            EDPropertyItem *pPropertyItem = NULL;
            EDPropertyItemList *pPropertyItemList = NULL;
            addNewField(rProp.getName());
            if(!pType->hasPredefinedValues()){
                pPropertyItem = new EDPropertyItem(rProp.getValue(), nGroup, nProp, pType->hasEdit());
            }
            else{
                 const EDPIPropertyTypeList* pType = dynamic_cast<const EDPIPropertyTypeList*>(rProp.getType());
                 pPropertyItemList = new EDPropertyItemList(rProp.getValue(), nGroup, nProp, pType, pType->hasEdit());  
                 connect (pPropertyItemList, SIGNAL(currentIndexChanged ( const QString& )), this, SLOT(sl_comboEditTextChangerd(const QString& )));
            }

            int itemToSetPos = isSeq? rowCount() - 1 : seqOffset - 1;

            if(!pType->hasPredefinedValues()){
                setItem(itemToSetPos, 1, pPropertyItem);
            }else{
                setCellWidget(itemToSetPos, 1, pPropertyItemList);
            }
        }
    }
    resizeColumnsToContents();
}

void EDPropertiesTable::updateCurrentProperties(){
    representPIProperties(curPItem);
}

void EDPropertiesTable::addNewGroup(const QString& name){
    int rowPos = isSeq ? rowCount() : seqOffset;
    insertRow(rowPos);
    setSpan(rowPos, 0, 1, 2);

    QTableWidgetItem* item = new QTableWidgetItem();
    item->setFlags(Qt::ItemIsSelectable | Qt::ItemIsEditable);
    item->setText(name);
    item->setBackgroundColor(Qt::lightGray);
    item->setTextColor(Qt::black);
    QFont font = item->font();
    font.setBold(true);
    item->setFont(font);

    setItem(rowPos, 0, item);

    if(!isSeq){
        seqOffset++;
    }
}

void EDPropertiesTable::addNewField(const QString& name){
    int rowPos = isSeq ? rowCount() : seqOffset;
    insertRow(rowPos);   

    QTableWidgetItem* item = new QTableWidgetItem();
    item->setFlags(Qt::ItemIsSelectable | Qt::ItemIsEditable);
    item->setText(name);
    item->setTextColor(Qt::black);

    setItem(rowPos, 0, item);

    if(!isSeq){
        seqOffset++;
    }
}


EDPropertyItem::EDPropertyItem(const QString& val, int nGroup, int nProp, bool editable)
:QTableWidgetItem()
{
    this->nGroup = nGroup;
    this->nProp = nProp;
    if(!editable){
        setFlags(Qt::ItemIsEditable);
    }
    setData(val);
    setText(val);
}

// EDPropertyItem::EDPropertyItem(const QString& val, int nGroup, int nProp, const EDPIPropertyTypeList* pType,  bool editable)
// :QTableWidgetItem()
// {
//     this->nGroup = nGroup;
//     this->nProp = nProp;
//     if(!editable){
//         setFlags(Qt::ItemIsSelectable |Qt::ItemIsEditable);
//     }
//     setData(val);
//     setText(val);
// }

// EDPropertyItem::EDPropertyItem(const QString& val, int nGroup, int nProp, bool editable)
// :QTableWidgetItem()
// {
//     this->nGroup = nGroup;
//     this->nProp = nProp;
//     if(!editable){
//         setFlags(Qt::ItemIsEditable);
//     }
//     setData(val);
//     setText(val);
// }

EDPropertyItemList::EDPropertyItemList(const QString& val, int nGroup, int nProp, const EDPIPropertyTypeList* pType,  bool editable)
:QComboBox()
{
    this->nGroup = nGroup;
    this->nProp = nProp;
    if(!editable){
        //setFlags(Qt::ItemIsSelectable |Qt::ItemIsEditable);
    }
    for (int i = 0; i < pType->getValueNumber(); i ++){
        insertItem(i, pType->getValue(i));
    }
    setData(val);
    int curInd = pType->getValueId(val);
    if(curInd == -1){
        insertItem(0, val);
        setCurrentIndex(0);
    }else{
        setCurrentIndex(curInd);
    }
    setEditable(editable);
    //setFrame(false);
    //setInsertPolicy(NoInsert);
}

void EDPropertiesTable::sl_treeSelChanged(QTreeWidgetItem * tItem){
    EDProjectItem* pItem = dynamic_cast<EDProjectItem*>(tItem);

//     if(pItem && (pItem->getType() == PIT_SEQUENCE || pItem->getType() ==PIT_CONTROLSEQUENCE)){
//         isSeq = true;
//     }else{
//         isSeq = false;
//     }
    representPIProperties(pItem);
}

void EDPropertiesTable::cleanup(){
    if(isSeq){
        int rowC = rowCount();
        for(int i = seqOffset; i < rowC; i++){
            removeRow(seqOffset);
        }
        seqOffset = rowCount();
    }else{
        for(int i = 0; i < seqOffset; i++){
            removeRow(0);
        }
        seqOffset = 0;
    }
    
}

void EDPropertiesTable::clearAll(){
    isSeq = false;
    seqOffset = 0;
    curPItem = NULL;
    clear();
    setRowCount(0);
}

void  EDPropertiesTable::sl_cellChanged(QTableWidgetItem* tItem){
    /*QTableWidgetItem* tItem = item(row, column);
    if(!tItem){
        return;
    }*/
    EDPropertyItem* edPropItem = dynamic_cast<EDPropertyItem*>(tItem);
    if(!edPropItem){
        return;
    }
    
    int nGroup = edPropItem->getGroup();
    int nProp  = edPropItem->getProp();
    const EDPIPropertyGroup& rGroup = curPItem->getGroup(nGroup);
    const EDPIProperty& rProp = rGroup.getProperty(nProp);

    QString strNewValue = edPropItem->text();
    if(strNewValue!=edPropItem->getData()){
        edPropItem->setData(strNewValue);
        emit si_propChanged(curPItem, &rProp, strNewValue);
    }
    
}

void EDPropertiesTable::sl_cellDataChanged(int row, int column){
    
    QWidget* w =  cellWidget(row, column);
    if(w == NULL){
        return;
    }

    EDPropertyItemList* edPropItem = dynamic_cast<EDPropertyItemList*>(w);
    if(!edPropItem){
        return;
    }

    int nGroup = edPropItem->getGroup();
    int nProp  = edPropItem->getProp();
    const EDPIPropertyGroup& rGroup = curPItem->getGroup(nGroup);
    const EDPIProperty& rProp = rGroup.getProperty(nProp);

    QString strNewValue = edPropItem->currentText();
    if(strNewValue!=edPropItem->getData()){
        edPropItem->setData(strNewValue);
        emit si_propChanged(curPItem, &rProp, strNewValue);
    }
}

void EDPropertiesTable::sl_comboEditTextChangerd(const QString& t){
    EDPropertyItemList* edPropItem = dynamic_cast<EDPropertyItemList*>(sender());
    if(!edPropItem){
        return;
    }

    int nGroup = edPropItem->getGroup();
    int nProp  = edPropItem->getProp();
    const EDPIPropertyGroup& rGroup = curPItem->getGroup(nGroup);
    const EDPIProperty& rProp = rGroup.getProperty(nProp);

    QString strNewValue = edPropItem->currentText();
    if(strNewValue!=edPropItem->getData()){
        edPropItem->setData(strNewValue);
        emit si_propChanged(curPItem, &rProp, strNewValue);
    } 
}


}//namespace

