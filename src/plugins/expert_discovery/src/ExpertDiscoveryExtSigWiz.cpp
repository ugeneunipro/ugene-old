#include "ExpertDiscoveryExtSigWiz.h"

#include "ExpertDiscoveryAdvSetDialog.h"

#include <QtGui/QMessageBox>
#include <U2Gui/HelpButton.h>


namespace U2 {

ExpertDiscoveryExtSigWiz::ExpertDiscoveryExtSigWiz(QWidget *parent, CSFolder* f, int positiveSize, bool isLetters)
: QWizard(parent)
,posSize(positiveSize)
,folder(NULL){

    setupUi(this);
    new U2::HelpButton(this, button(QWizard::HelpButton), "4227131");

//1 page
    connect(advancedButton, SIGNAL(clicked()), SLOT(sl_advButton()));
    state.setDefaultState();

    condProbLevEdit->setText(QString("%1").arg(state.dProbability));
    coverBoundEdit->setText(QString("%1").arg(state.dCoverage));
    fishCritEdit->setText(QString("%1").arg(state.dFisher));
    minimFishCritCheck->setChecked(state.bCheckFisherMinimization);
    storeCheck->setChecked(state.bStoreOnlyDifferent);
    ulCritCheck->setChecked(state.bUmEnabled);
    samplesBoundEdit->setText(QString("%1").arg(state.nUmSamplesBound));
    levelBoundEdit->setText(QString("%1").arg(state.dUmBound));
    minComplexityEdit->setText(QString("%1").arg(state.nMinComplexity));
    maxComplexityEdit->setText(QString("%1").arg(state.nMaxComplexity));

    QDoubleValidator* d0_100Valid = new QDoubleValidator(0,100,5,this);
    QDoubleValidator* d0_1Valid = new QDoubleValidator(0,1,5,this);
    QDoubleValidator* d0_maxValid = new QDoubleValidator(0,LONG_MAX,0,this);

    condProbLevEdit->setValidator(d0_100Valid);
    coverBoundEdit->setValidator(d0_100Valid);
    fishCritEdit->setValidator(d0_1Valid);
    samplesBoundEdit->setValidator(d0_maxValid);
    levelBoundEdit->setValidator(d0_1Valid);

//2 page 
    initSet();

    predicatesTree->setSelectionMode(QAbstractItemView::SingleSelection);

    connect(distanseButton,SIGNAL(clicked()), SLOT(sl_distButton()));
    connect(repetitionButton,SIGNAL(clicked()), SLOT(sl_repetButton()));
    connect(intervalButton,SIGNAL(clicked()), SLOT(sl_intervButton()));
    connect(deleteButton,SIGNAL(clicked()), SLOT(sl_deleteButton()));

    distItem = new QTreeWidgetItem(predicatesTree);
    repetItem = new QTreeWidgetItem(predicatesTree);
    intervItem = new QTreeWidgetItem(predicatesTree);

    distItem->setText(0,"Distance");
    repetItem->setText(0,"Repetition");
    intervItem->setText(0,"Interval");

    connect(predicatesTree, SIGNAL(currentItemChanged (QTreeWidgetItem*, QTreeWidgetItem*)), SLOT(sl_selectionChanged (QTreeWidgetItem*, QTreeWidgetItem*)));

    predicatesByDefault(isLetters);

//page 3
    folder = f;
    updateTree();
    connect(createSubfolderButton, SIGNAL(clicked()), SLOT(sl_createSubfolder()));

    connect(this, SIGNAL(currentIdChanged ( int )), this, SLOT( sl_idChanged(int )));
    
    hideParameters();
}

ExpertDiscoveryExtSigWiz::~ExpertDiscoveryExtSigWiz(){
    delete distItem;
    delete repetItem;
    delete intervItem;

    delete sigSetLayout;
}

void ExpertDiscoveryExtSigWiz::sl_advButton(){
    ExpertDiscoveryAdvSetDialog adv (this,
        state.dIntProbability,
        state.dIntFisher,
        state.nMinComplexity,
        state.nMaxComplexity,
        state.dMinPosCorrelation,
        state.dMaxPosCorrelation,
        state.dMinNegCorrelation,
        state.dMaxNegCorrelation,
        state.bCorrelationImportant);
    if(adv.exec()){

    }

}

void ExpertDiscoveryExtSigWiz::sl_distButton(){
    OpDistance *pOp = new OpDistance;
    QTreeWidgetItem *op = new QTreeWidgetItem(distItem);
    op->setText(0, QString::fromStdString(pOp->getDescription()));
    void* pointer = (void*)pOp;
    QVariant variant = qVariantFromValue<void*>(pointer);
    op->setData(0, Qt::UserRole, variant);

    predicatesTree->setCurrentItem(op);
    distItem->setExpanded(true);
    //op->setSelected(true);
}
void ExpertDiscoveryExtSigWiz::sl_repetButton(){
    OpReiteration *pOp = new OpReiteration;
    QTreeWidgetItem *op = new QTreeWidgetItem(repetItem);
    op->setText(0, QString::fromStdString(pOp->getDescription()));
    void* pointer = (void*)pOp;
    QVariant variant = qVariantFromValue<void*>(pointer);
    op->setData(0, Qt::UserRole, variant);
    //predicatesTree->clearSelection();
    predicatesTree->setCurrentItem(op);
    repetItem->setExpanded(true);
    //op->setSelected(true);
}
void ExpertDiscoveryExtSigWiz::sl_intervButton(){
    OpInterval *pOp = new OpInterval;
    QTreeWidgetItem *op = new QTreeWidgetItem(intervItem);
    op->setText(0, QString::fromStdString(pOp->getDescription()));
    void* pointer = (void*)pOp;
    QVariant variant = qVariantFromValue<void*>(pointer);
    op->setData(0, Qt::UserRole, variant);
    //predicatesTree->clearSelection();
    predicatesTree->setCurrentItem(op);
    intervItem->setExpanded(true);
    //op->setSelected(true);
}
void ExpertDiscoveryExtSigWiz::sl_deleteButton(){
    if(predicatesTree->selectedItems().isEmpty()){
        return ;
    }
    QTreeWidgetItem* item = predicatesTree->selectedItems().first();
    QVariant variant = item->data(0, Qt::UserRole);
    void* pointer = qVariantValue<void*>(variant);
    Operation* pOp = (Operation*) pointer;
    if (pOp == NULL)
        return;
    if (sigSetLayout->currentIndex() != T_UNDEFINED) {
        sigSetLayout->setCurrentIndex(T_UNDEFINED);
    }
    delete pOp;
    predicatesTree->removeItemWidget(item,0);
    delete item;
}


void ExpertDiscoveryExtSigWiz::sl_idChanged(int id){
    switch (id){
        case 2:{
            int minCom = 0;
            int maxCom = 0;
            minCom = minComplexityEdit->text().toInt();
            maxCom = maxComplexityEdit->text().toInt();

            if(minCom>maxCom || minCom<0){
                back();
                QMessageBox mb(QMessageBox::Critical, tr("Wrong parameters"), tr("Minimal complexity must not be grater then maximal complexity and positive"));
                mb.exec();
                break;
            }else{
                if(!(checkD(condProbLevEdit) && checkD(coverBoundEdit)
                    && checkD(fishCritEdit) && checkD(levelBoundEdit)
                    && checkD(samplesBoundEdit))){
                        back();
                }
            }
            break;
        }
        case 3:
            sl_selectionChanged(predicatesTree->currentItem(), predicatesTree->currentItem());
            if((intervItem->childCount() == 0 ) && (repetItem->childCount() == 0 ) && (distItem->childCount() == 0 ) && !alignedCheck->isChecked()){
                back();
                QMessageBox mb(QMessageBox::Critical, tr("No predicates"), tr("Create a predicate to perform signal generation"));
                mb.exec();
            }
    }
}

void ExpertDiscoveryExtSigWiz::sl_createSubfolder(){
    QString folderName = folderNameEdit->text();

    if(folderName.isEmpty()){
        QMessageBox mb(QMessageBox::Critical, tr("Specify folder name"), tr("Please specify a name for your folder"));
        mb.exec();
        folderNameEdit->setFocus();
        return;
    }
    
    QTreeWidgetItem* csItem = treeFoldersWidget->topLevelItem(0);
    assert(csItem!=NULL);

    bool isUniqueName = true;
    for (int i = 0 ; i < csItem->childCount(); i++){
        QTreeWidgetItem* item  = csItem->child(i);
        if(item->text(0) == folderName){
            isUniqueName = false;
            break;
        }
    }

    if(!isUniqueName){
        QMessageBox mb(QMessageBox::Critical, tr("Specify folder name"), tr("Item with the same name already exist. Please enter another name"));
        mb.exec();
        folderNameEdit->setFocus();
        return;
    }

    emit si_newFolder(folderName);
    updateTree();
}
void ExpertDiscoveryExtSigWiz::predicatesByDefault(bool isLetters){
    if(isLetters){
        for(int i = 0; i < 3; i++){ //generate D(0,0), D(1,1), D(2,2)
            OpDistance *pOp = new OpDistance;
            pOp->setDistance(Interval(i,i));
            QTreeWidgetItem *op = new QTreeWidgetItem(distItem);
            op->setText(0, QString::fromStdString(pOp->getDescription()));
            void* pointer = (void*)pOp;
            QVariant variant = qVariantFromValue<void*>(pointer);
            op->setData(0, Qt::UserRole, variant);
            distItem->setExpanded(true);
        }
    }else{  //generate D(0,10), D(10,20)
        for(int i = 0; i < 20; i+=10){
            OpDistance *pOp = new OpDistance;
            pOp->setDistance(Interval(i,i+10));
            QTreeWidgetItem *op = new QTreeWidgetItem(distItem);
            op->setText(0, QString::fromStdString(pOp->getDescription()));
            void* pointer = (void*)pOp;
            QVariant variant = qVariantFromValue<void*>(pointer);
            op->setData(0, Qt::UserRole, variant);
            distItem->setExpanded(true);
        }
    }
}

void ExpertDiscoveryExtSigWiz::accept(){

    //page2
    QVariant variant;
    void* pointer = NULL;
    Operation* pOp = NULL;

    QList<QTreeWidgetItem*> ch = distItem->takeChildren();
    foreach(QTreeWidgetItem* item, ch){
        variant = item->data(0, Qt::UserRole);
        pointer = qVariantValue<void*>(variant);
        pOp = (Operation*) pointer;
        predicates.push_back(pOp);
        delete item;
    }

    ch = repetItem->takeChildren();
    foreach(QTreeWidgetItem* item, ch){
        variant = item->data(0, Qt::UserRole);
        pointer = qVariantValue<void*>(variant);
        pOp = (Operation*) pointer;
        predicates.push_back(pOp);
        delete item;
    }

    ch = intervItem->takeChildren();
    foreach(QTreeWidgetItem* item, ch){
        variant = item->data(0, Qt::UserRole);
        pointer = qVariantValue<void*>(variant);
        pOp = (Operation*) pointer;
        predicates.push_back(pOp);
        delete item;
    }

    if(alignedCheck->isChecked()){
        for(int i = 0; i < posSize; i++){
            OpInterval *pOp = new OpInterval;
            pOp->setInt(Interval(i, i));
            predicates.push_back((Operation*)pOp);
        }
    }
    
    if(predicates.empty()){
        QMessageBox mb(QMessageBox::Critical, tr("No predicates"), tr("Create a predicate to perform signal generation"));
        mb.exec();
    }else{
        //page 1
        state.dProbability = condProbLevEdit->text().toDouble();
        state.dCoverage = coverBoundEdit->text().toDouble();
        state.dFisher =  fishCritEdit->text().toDouble();
        state.bCheckFisherMinimization = minimFishCritCheck->isChecked();
        state.bStoreOnlyDifferent = storeCheck->isChecked();
        state.bUmEnabled = ulCritCheck->isChecked();
        state.nUmSamplesBound = (int)(samplesBoundEdit->text().toDouble());
        state.dUmBound = levelBoundEdit->text().toDouble();
        state.nMinComplexity = minComplexityEdit->text().toInt();
        state.nMaxComplexity = maxComplexityEdit->text().toInt();

        //page 3
        if(!treeFoldersWidget->selectedItems().isEmpty()){
            QTreeWidgetItem* item = treeFoldersWidget->selectedItems().first();    
            QVariant variant = item->data(0, Qt::UserRole);
            void* pointer = qVariantValue<void*>(variant);
            CSFolder* f = (CSFolder*) pointer;
            folder = f;
        }

        QWizard::accept();
    }
    

}

void ExpertDiscoveryExtSigWiz::hideParameters(){
  //  label_4->hide();
 //   fishCritEdit->hide();
 //   minimFishCritCheck->hide();
//    storeCheck->hide();
    ulCritCheck->hide();
    label_5->hide();
    samplesBoundEdit->hide();
    label_6->hide();
    levelBoundEdit->hide();
    advancedButton->hide();
   
}

bool ExpertDiscoveryExtSigWiz::checkD(const QLineEdit* lineE) const{
    if(!lineE->validator()){
        return false;
    }
    const QDoubleValidator* validator = qobject_cast<const QDoubleValidator*>(lineE->validator());
    int pos = 0;
        QString textValue=lineE->text();
        if(validator->validate(textValue,pos)!=QValidator::Acceptable){
        QString msg = QString("Entered value must be from %1 to %2").arg(validator->bottom()).arg(validator->top());
        QMessageBox mb(QMessageBox::Critical, tr("Wrong parameters"), tr(msg.toStdString().c_str()));
        mb.exec();
        return false;
    }
    return true;
}

void ExpertDiscoveryExtSigWiz::updateTree(const CSFolder* pFolder, QTreeWidgetItem* treeItem){
    QString strName;
    QTreeWidgetItem* nTreeItem = NULL;
    if(pFolder == NULL){
        treeFoldersWidget->clear();
        pFolder = folder;
        strName = tr("Complex signals");
        nTreeItem = new QTreeWidgetItem(treeFoldersWidget);
 
    }else{
        strName = pFolder->getName();
        nTreeItem = new QTreeWidgetItem(treeItem);
    }
    nTreeItem->setText(0,strName);
    void* pointer = (void*)pFolder;
    QVariant variant = qVariantFromValue<void*>(pointer);
    nTreeItem->setData(0, Qt::UserRole, variant);
    int nFolderNum = pFolder->getFolderNumber();
    for (int i=0; i<nFolderNum; i++){
        updateTree(pFolder->getSubfolder(i), nTreeItem);
    }

    nTreeItem->setExpanded(true);
}

void ExpertDiscoveryExtSigWiz::sl_selectionChanged (QTreeWidgetItem * current, QTreeWidgetItem * previous){

//saving data to the deselected item

    int curIndex = sigSetLayout->currentIndex();

    if(previous== NULL){
        return ;
    }

    if(previous != distItem && previous != intervItem && previous != repetItem && sigSet[curIndex]->isReadyToClose() && curIndex != T_UNDEFINED){
        QVariant variant = previous->data(0, Qt::UserRole);
        void* pointer = qVariantValue<void*>(variant);
        Operation* pOp = (Operation*) pointer;
        sigSet[curIndex]->saveData(pOp);
        previous->setText(0, QString::fromStdString(pOp->getDescription()));
        variant = qVariantFromValue<void*>(pointer);
        previous->setData(0, Qt::UserRole, variant);
        sigSetLayout->setCurrentIndex(T_UNDEFINED);
    }

//loading data to the form
    if(current == NULL || current == distItem || current == intervItem || current == repetItem){
        sigSetLayout->setCurrentIndex(T_UNDEFINED);
        return ;
    }else if(current->parent()==distItem){
        sigSetLayout->setCurrentIndex(T_DISTANCE);
        QVariant variant = current->data(0,Qt::UserRole);
        void* dataP = qVariantValue<void*>(variant);
        ((DistanceSet*)sigSet[T_DISTANCE])->loadData(dataP);
    }else if(current->parent()==intervItem){
        sigSetLayout->setCurrentIndex(T_INTERVAL);
        QVariant variant = current->data(0,Qt::UserRole);
        void* dataP = qVariantValue<void*>(variant);
        ((IntervalSet*)sigSet[T_INTERVAL])->loadData(dataP);
    }else if(current->parent()==repetItem){
        sigSetLayout->setCurrentIndex(T_REITERATION);
        QVariant variant = current->data(0,Qt::UserRole);
        void* dataP = qVariantValue<void*>(variant);
        ((RepetitionSet*)sigSet[T_REITERATION])->loadData(dataP);
    }
}

void ExpertDiscoveryExtSigWiz::initSet(){
    sigSetLayout = new QStackedLayout(editorBox);

//undef
    sigSet[T_UNDEFINED] = new OperationSet(editorBox);
    sigSet[T_UNDEFINED]->setEnabled(true);
    QGridLayout* gridLayoutUndef = new QGridLayout(sigSet[T_UNDEFINED]);
    QLabel* labelUndef = new QLabel(sigSet[T_UNDEFINED]);
    labelUndef->setText(tr("Select predicate on the tree above or create a new one using buttons."));
    gridLayoutUndef->addWidget(labelUndef, 0, 0, 1, 1);
    QSpacerItem* verticalSpacerUndef = new QSpacerItem(20, 40, QSizePolicy::Minimum, QSizePolicy::Expanding);
    gridLayoutUndef->addItem(verticalSpacerUndef, 1, 0, 1, 1);
    sigSetLayout->insertWidget(T_UNDEFINED,sigSet[T_UNDEFINED]);

    sigSet[T_DISTANCE] = new DistanceSet(editorBox);
    sigSetLayout->insertWidget(T_DISTANCE,sigSet[T_DISTANCE]);

    sigSet[T_REITERATION] = new RepetitionSet(editorBox);
    sigSetLayout->insertWidget(T_REITERATION,sigSet[T_REITERATION]);

    sigSet[T_INTERVAL] = new IntervalSet(editorBox);
    sigSetLayout->insertWidget(T_INTERVAL,sigSet[T_INTERVAL]);

    sigSetLayout->setCurrentIndex(T_UNDEFINED);

    verticalLayout_4->addLayout(sigSetLayout);
}

DistanceSet::DistanceSet (QWidget* parent)
: OperationSet(parent)
    , to(0)
    , from(0)
    , isOrderImportant(false)
    , isMaxUNL(true){

    setEnabled(true);
    QGridLayout* gridLayoutD = new QGridLayout(this);
    QLabel* label1D = new QLabel(this);
    label1D->setText(tr("Distance from"));
    gridLayoutD->addWidget(label1D, 0, 0, 1, 1);
    QLabel* label2D = new QLabel(this);
    label2D->setText(tr("Distance to"));
    gridLayoutD->addWidget(label2D, 1, 0, 1, 1);
    fromEditD = new QLineEdit(this);
    gridLayoutD->addWidget(fromEditD, 0, 2, 1, 1);
    toEditD = new QLineEdit(this);
    gridLayoutD->addWidget(toEditD, 1, 2, 1, 1);
    unlimCheckBoxD = new QCheckBox(this);
    unlimCheckBoxD->setText(tr("Unlimited"));
    gridLayoutD->addWidget(unlimCheckBoxD, 1, 3, 1, 1);
    orderCheckBoxD = new QCheckBox(this);
    orderCheckBoxD->setText(tr("Take order into account"));
    gridLayoutD->addWidget(orderCheckBoxD, 2, 2, 1, 1);
    QSpacerItem* horizontalSpacerD = new QSpacerItem(40, 20, QSizePolicy::Expanding, QSizePolicy::Minimum);
    gridLayoutD->addItem(horizontalSpacerD, 0, 3, 1, 1);

    QIntValidator* validator = new QIntValidator(0, 65535, this);
    fromEditD->setValidator(validator);
    toEditD->setValidator(validator);


    connect(unlimCheckBoxD, SIGNAL(clicked()), SLOT(sl_unlim()));
    connect(orderCheckBoxD, SIGNAL(clicked()), SLOT(sl_order()));
}

void DistanceSet::loadData(void *pData){
    OpDistance *pOp = reinterpret_cast<OpDistance*>(pData);
    Interval i = pOp->getDistance();
    from = i.getFrom();
    to = i.getTo();
    isOrderImportant = pOp->isOrderImportant();
    isMaxUNL = (to == PINF);
    if (isMaxUNL) to = from + 1;
    toEditD->setEnabled(!isMaxUNL);
    updateData(false);
}
void DistanceSet::saveData(void *pData){
    OpDistance *pOp = reinterpret_cast<OpDistance*>(pData);
    updateData();
    if (isMaxUNL) to = PINF;
    pOp->setDistance( Interval(from, to) );
    pOp->setOrderImportant( isOrderImportant == true );
}

bool DistanceSet::isReadyToClose(){
    updateData();
    if (isMaxUNL) to = PINF;
    if ( from > to ) {
        QMessageBox mb(QMessageBox::Critical, tr("Wrong parameters"), tr("Higher bound must be grater then lower bound"));
        mb.exec();
        return false;
    }
    else return true;
}

void DistanceSet::updateData(bool side){
    if(side){
        to = toEditD->text().toInt();
        from = fromEditD->text().toInt();
        isOrderImportant = orderCheckBoxD->isChecked();
        isMaxUNL = unlimCheckBoxD->isChecked();
    }else{
        toEditD->setText(QString("%1").arg(to));
        fromEditD->setText(QString("%1").arg(from));
        orderCheckBoxD->setChecked(isOrderImportant);
        unlimCheckBoxD->setChecked(isMaxUNL);
    }

}
void DistanceSet::sl_unlim(){
    isMaxUNL = unlimCheckBoxD->isChecked();
    toEditD->setEnabled( !isMaxUNL);
}
void DistanceSet::sl_order(){
    isOrderImportant = orderCheckBoxD->isChecked();
}

IntervalSet::IntervalSet (QWidget* parent)
: OperationSet(parent)
    , from(0)
    , to(0)
    , isMaxUNL(true){

    setEnabled(true);
    QGridLayout* gridLayoutI = new QGridLayout(this);
    QLabel* label1I = new QLabel(this);
    label1I->setText(tr("Interval from"));
    gridLayoutI->addWidget(label1I, 0, 0, 1, 1);
    QLabel* label2I = new QLabel(this);
    label2I->setText(tr("Interval to"));
    gridLayoutI->addWidget(label2I, 2, 0, 1, 1);
    fromIEdit = new QLineEdit(this);
    gridLayoutI->addWidget(fromIEdit, 0, 2, 1, 1);
    toIEdit = new QLineEdit(this);
    gridLayoutI->addWidget(toIEdit, 2, 2, 1, 1);
    QSpacerItem* horizontalSpacer_4 = new QSpacerItem(40, 20, QSizePolicy::Expanding, QSizePolicy::Minimum);
    gridLayoutI->addItem(horizontalSpacer_4, 2, 4, 1, 1);
    unlimCheckBoxI = new QCheckBox(this);
    unlimCheckBoxI->setText(tr("Unlimited"));
    gridLayoutI->addWidget(unlimCheckBoxI, 2, 3, 1, 1);

    QIntValidator* validator = new QIntValidator(0, 65535, this);
    fromIEdit->setValidator(validator);
    toIEdit->setValidator(validator);

    connect(unlimCheckBoxI, SIGNAL(clicked()), SLOT(sl_unlim()));

}

void IntervalSet::loadData(void *pData){
    OpInterval *pOp = reinterpret_cast<OpInterval*>(pData);
    Interval i = pOp->getInt();
    from = i.getFrom();
    to = i.getTo();
    isMaxUNL = (to == PINF);
    if (isMaxUNL) to = from+1;
    toIEdit->setEnabled( !isMaxUNL);
    updateData(false);
}
void IntervalSet::saveData(void *pData){
    OpInterval *pOp = reinterpret_cast<OpInterval*>(pData);
    updateData();
    if (isMaxUNL) to = PINF;
    pOp->setInt( Interval(from, to) );
}

bool IntervalSet::isReadyToClose(){
    updateData();
    if (isMaxUNL) to = PINF;
    if ( from > to ) {
        QMessageBox mb(QMessageBox::Critical, tr("Wrong parameters"), tr("Higher bound must be grater then lower bound"));
        mb.exec();
        return false;
    }
    else return true;
}

void IntervalSet::updateData(bool side){
    if(side){
        to = toIEdit->text().toInt();
        from = fromIEdit->text().toInt();
        isMaxUNL = unlimCheckBoxI->isChecked();
    }else{
        toIEdit->setText(QString("%1").arg(to));
        fromIEdit->setText(QString("%1").arg(from));
        unlimCheckBoxI->setChecked(isMaxUNL);
    }
}

void IntervalSet::sl_unlim(){
    isMaxUNL = unlimCheckBoxI->isChecked();
    toIEdit->setEnabled( !isMaxUNL);
}


RepetitionSet::RepetitionSet (QWidget* parent)
: OperationSet(parent)
    , nmin(0)
    , nmax(0)
    , min(0)
    , max(0)
    , isMaxUNL(true){

    setEnabled(true);
    QGridLayout* gridLayoutR = new QGridLayout(this);
    QLabel* label1R = new QLabel(this);
    label1R->setText(tr("Distance from"));
    gridLayoutR->addWidget(label1R, 0, 0, 1, 1);
    QLabel* label2R = new QLabel(this);
    label2R->setText(tr("Distance to"));
    gridLayoutR->addWidget(label2R, 1, 0, 1, 1);
    fromDEdit = new QLineEdit(this);
    gridLayoutR->addWidget(fromDEdit, 0, 2, 1, 1);
    toDEditR = new QLineEdit(this);
    gridLayoutR->addWidget(toDEditR, 1, 2, 1, 1);
    unlimCheckBoxR = new QCheckBox(this);
    unlimCheckBoxR->setText(tr("Unlimited"));
    gridLayoutR->addWidget(unlimCheckBoxR, 2, 2, 1, 1);
    QLabel* label3R = new QLabel(this);
    label3R->setText(tr("Count from"));
    gridLayoutR->addWidget(label3R, 0, 3, 1, 1);
    QLabel* label4R = new QLabel(this);
    label4R->setText(tr("Count to"));
    gridLayoutR->addWidget(label4R, 1, 3, 1, 1);
    fromCEditR = new QLineEdit(this);
    gridLayoutR->addWidget(fromCEditR, 0, 4, 1, 1);
    toCEditR = new QLineEdit(this);
    gridLayoutR->addWidget(toCEditR, 1, 4, 1, 1);

    QIntValidator* validator = new QIntValidator(0, 65535, this);
    fromDEdit->setValidator(validator);
    toDEditR->setValidator(validator);

    QIntValidator* validator1 = new QIntValidator(1, 65535, this);
    fromCEditR->setValidator(validator1);
    toCEditR->setValidator(validator1);

    connect(unlimCheckBoxR, SIGNAL(clicked()), SLOT(sl_unlim()));
}

void RepetitionSet::loadData(void *pData){
    OpReiteration *pOp = reinterpret_cast<OpReiteration*>(pData);
    Interval i = pOp->getCount();
    nmin = i.getFrom();
    nmax = i.getTo();
    i = pOp->getDistance();
    min = i.getFrom();
    max = i.getTo();
    isMaxUNL = (max == PINF);
    if (isMaxUNL) max = min + 1;
    toDEditR->setEnabled( !isMaxUNL);
    updateData(false);
}
void RepetitionSet::saveData(void *pData){
    OpReiteration *pOp = reinterpret_cast<OpReiteration*>(pData);
    updateData();
    if (isMaxUNL) max = PINF;
    pOp->setCount( Interval(nmin, nmax) );
    pOp->setDistance( Interval(min, max) );
}

bool RepetitionSet::isReadyToClose(){
    updateData();
    if ( isMaxUNL ) max = PINF;
    if ( min > max ) {
        QMessageBox mb(QMessageBox::Critical, tr("Wrong parameters"), tr("Higher bound must be grater then lower bound"));
        mb.exec();
        return false;
    }
    else
    if ( nmin > nmax ) {
        QMessageBox mb(QMessageBox::Critical, tr("Wrong parameters"), tr("Higher bound must be grater then lower bound"));
        mb.exec();
        return false;
    }
    else return true;
}

void RepetitionSet::updateData(bool side){
    if(side){
        nmin = fromCEditR->text().toInt();
        nmax = toCEditR->text().toInt();
        min = fromDEdit->text().toInt();
        max = toDEditR->text().toInt();
        isMaxUNL = unlimCheckBoxR->isChecked();
    }else{
        fromDEdit->setText(QString("%1").arg(min));
        toDEditR->setText(QString("%1").arg(max));
        fromCEditR->setText(QString("%1").arg(nmin));
        toCEditR->setText(QString("%1").arg(nmax));
        unlimCheckBoxR->setChecked(isMaxUNL);
    }

}

void RepetitionSet::sl_unlim(){
    isMaxUNL = unlimCheckBoxR->isChecked();
    toDEditR->setEnabled( !isMaxUNL);
}
}//namespace


