#include "ExternalToolSupportSettingsController.h"
#include "ExternalToolSupportSettings.h"
#include "utils/ExternalToolValidateTask.h"

#include <U2Core/AppContext.h>
#include <U2Misc/DialogUtils.h>
#include <QtGui>
#include <blast/BlastAllSupport.h>
#include <blast/FormatDBSupport.h>
#include <blast_plus/BlastPlusSupport.h>



namespace U2 {

ExternalToolSupportSettingsPageController::ExternalToolSupportSettingsPageController(QObject* p)
: AppSettingsGUIPageController(tr("External Tools"), ExternalToolSupportSettingsPageId, p) {}


AppSettingsGUIPageState* ExternalToolSupportSettingsPageController::getSavedState() {
    ExternalToolSupportSettingsPageState* state = new ExternalToolSupportSettingsPageState();
    state->externalTools=AppContext::getExternalToolRegistry()->getAllEntries();
    return state;
}

void ExternalToolSupportSettingsPageController::saveState(AppSettingsGUIPageState* s) {

    ExternalToolSupportSettingsPageState* state = qobject_cast<ExternalToolSupportSettingsPageState*>(s);
    //Add new external tools to registry
    foreach(ExternalTool* externalTool, state->externalTools){
        AppContext::getExternalToolRegistry()->getByName(externalTool->getName())->setValid(externalTool->isValid());
        AppContext::getExternalToolRegistry()->getByName(externalTool->getName())->setPath(externalTool->getPath());
        AppContext::getExternalToolRegistry()->getByName(externalTool->getName())->setVersion(externalTool->getVersion());
    }
    ExternalToolSupportSettings::setExternalTools();
}

AppSettingsGUIPageWidget* ExternalToolSupportSettingsPageController::createWidget(AppSettingsGUIPageState* state) {
    ExternalToolSupportSettingsPageWidget* r = new ExternalToolSupportSettingsPageWidget(this);
    r->setState(state);
    return r;
}


ExternalToolSupportSettingsPageWidget::ExternalToolSupportSettingsPageWidget(ExternalToolSupportSettingsPageController* ) {
    setupUi(this);
    versionLabel->hide();
    binaryPathLabel->hide();
}

QWidget* ExternalToolSupportSettingsPageWidget::createPathEditor(QWidget *parent, const QString& path) const
{
    QWidget * widget = new QWidget(parent);
    PathLineEdit* toolPathEdit= new PathLineEdit("","executable", false, widget);
    toolPathEdit->setObjectName("PathLineEdit");
    toolPathEdit->setFrame(false);
    toolPathEdit->setSizePolicy(QSizePolicy(QSizePolicy::Preferred,QSizePolicy::Preferred));
    toolPathEdit->setText(QDir::toNativeSeparators(path));
    widget->setFocusProxy(toolPathEdit);
    connect(toolPathEdit, SIGNAL(cursorPositionChanged(int,int)), this, SLOT(sl_onPathEditWidgetClick()));
    connect(toolPathEdit, SIGNAL(selectionChanged()), this, SLOT(sl_onPathEditWidgetClick()));
    connect(toolPathEdit, SIGNAL(editingFinished()), this, SLOT(sl_toolPathCanged()));
    QToolButton * selectToolPathButton = new QToolButton(widget);
    selectToolPathButton->setVisible(true);
    selectToolPathButton->setText("...");
    selectToolPathButton->setSizePolicy(QSizePolicy(QSizePolicy::Fixed,QSizePolicy::Preferred));
    connect(selectToolPathButton, SIGNAL(clicked()), this, SLOT(sl_onPathEditWidgetClick()));
    connect(selectToolPathButton, SIGNAL(clicked()), toolPathEdit, SLOT(sl_onBrowse()));

    QToolButton * clearToolPathButton = new QToolButton(widget);
    clearToolPathButton->setObjectName("ClearToolPathButton");
    clearToolPathButton->setVisible(true);
    clearToolPathButton->setIcon(QIcon(":external_tool_support/images/cancel.png"));
    clearToolPathButton->setSizePolicy(QSizePolicy(QSizePolicy::Fixed,QSizePolicy::Preferred));
    clearToolPathButton->setEnabled(!path.isEmpty());
    connect(clearToolPathButton, SIGNAL(clicked()), this, SLOT(sl_onPathEditWidgetClick()));
    connect(clearToolPathButton,SIGNAL(clicked()),toolPathEdit,SLOT(sl_clear()));

    QHBoxLayout* layout = new QHBoxLayout(widget);
    layout->setSpacing(0);
    layout->setMargin(0);
    layout->addWidget(toolPathEdit);
    QHBoxLayout* layout2 = new QHBoxLayout(widget);
    layout2->addWidget(selectToolPathButton);
    layout2->addWidget(clearToolPathButton);
    layout->addLayout(layout2);
    buttonsWidth=layout2->minimumSize().width();

    return widget;
}
void ExternalToolSupportSettingsPageWidget::setState(AppSettingsGUIPageState* s) {
    ExternalToolSupportSettingsPageState* state = qobject_cast<ExternalToolSupportSettingsPageState*>(s);

    foreach(ExternalTool* tool, state->externalTools){
        ExternalToolInfo info;
        info.name=tool->getName();
        info.path=tool->getPath();
        info.description=tool->getDescription();
        info.valid=tool->isValid();
        info.version=tool->getVersion();
        externalToolsInfo.insert(info.name,info);
    }
    treeWidget->setColumnWidth(0,this->geometry().width()/3);

    QList<QList<ExternalTool*> > listToolKits=AppContext::getExternalToolRegistry()->getAllEntriesSortedByToolKits();
    foreach(QList<ExternalTool*> toolsList, listToolKits){
        if(toolsList.length()>1){
            QTreeWidgetItem* rootItem=new QTreeWidgetItem((QStringList) toolsList.first()->getToolKitName());
            rootItem->setIcon(0, toolsList.first()->getIcon());
            treeWidget->insertTopLevelItem(0, rootItem);
            foreach(ExternalTool* tool, toolsList){
                insertChild(rootItem, tool->getName(), 0);
            }
            rootItem->setExpanded(true);
            //draw widget for path select button
            QWidget * widget = new QWidget(treeWidget);
            QToolButton * selectToolKitPathButton = new QToolButton(widget);
            selectToolKitPathButton->setVisible(true);
            selectToolKitPathButton->setText("...");
            selectToolKitPathButton->setMinimumWidth(buttonsWidth);
            selectToolKitPathButton->setSizePolicy(QSizePolicy(QSizePolicy::Fixed,QSizePolicy::Preferred));
            connect(selectToolKitPathButton, SIGNAL(clicked()), this, SLOT(sl_onPathEditWidgetClick()));
            connect(selectToolKitPathButton, SIGNAL(clicked()), this, SLOT(sl_onBrowseToolKitPath()));
            QHBoxLayout* layout = new QHBoxLayout(widget);
            layout->setSpacing(0);
            layout->setMargin(0);
            layout->addStretch();
            layout->addWidget(selectToolKitPathButton);
            treeWidget->setItemWidget(rootItem,1,widget);
        }else{
            QTreeWidgetItem* item=new QTreeWidgetItem((QStringList) toolsList.first()->getName());
            treeWidget->addTopLevelItem(item);
            treeWidget->setItemWidget(item,1,createPathEditor(treeWidget, toolsList.first()->getPath()));
            if(toolsList.first()->getPath().isEmpty()){
                item->setIcon(0, toolsList.first()->getGrayIcon());
            }else if(toolsList.first()->isValid()){
                item->setIcon(0, toolsList.first()->getIcon());
            }else{
                item->setIcon(0, toolsList.first()->getWarnIcon());
            }
        }
    }
    connect(treeWidget,SIGNAL(itemSelectionChanged()),SLOT(sl_itemSelectionChanged()));
}
void ExternalToolSupportSettingsPageWidget::insertChild(QTreeWidgetItem* rootItem, QString name, int pos){
    QTreeWidgetItem* item=new QTreeWidgetItem((QStringList)name);
    rootItem->insertChild(pos,item);
    treeWidget->setItemWidget(item,1,createPathEditor(treeWidget, externalToolsInfo[name].path));
    if(externalToolsInfo.value(name).path.isEmpty()){
        item->setIcon(0, AppContext::getExternalToolRegistry()->getByName(name)->getGrayIcon());
    }else if(externalToolsInfo.value(name).valid){
        item->setIcon(0, AppContext::getExternalToolRegistry()->getByName(name)->getIcon());
    }else{
        item->setIcon(0, AppContext::getExternalToolRegistry()->getByName(name)->getWarnIcon());
    }
}

AppSettingsGUIPageState* ExternalToolSupportSettingsPageWidget::getState(QString& ) const {
    ExternalToolSupportSettingsPageState* state = new ExternalToolSupportSettingsPageState();
    ExternalTool* externalTool;
    foreach(ExternalToolInfo info, externalToolsInfo){
        externalTool=new ExternalTool(info.name,info.path);
        externalTool->setValid(info.valid);
        externalTool->setVersion(info.version);
        state->externalTools.append(externalTool);
    }
    return state;
}
void ExternalToolSupportSettingsPageWidget::sl_toolPathCanged(){
    PathLineEdit* s=qobject_cast<PathLineEdit*>(sender());
    assert(s!=NULL);

    if(!s->isModified()){
        return;
    }
    QWidget* par=s->parentWidget();
    QString path=s->text();
    s->setModified(false);

    QList<QTreeWidgetItem*> listOfItems=treeWidget->findItems("",Qt::MatchContains|Qt::MatchRecursive);
    assert(listOfItems.length()!=0);
    treeWidget->clearSelection();
    foreach(QTreeWidgetItem* item, listOfItems){
        QWidget* itemWid=treeWidget->itemWidget(item,1);
        if(par == itemWid){//may be no good method for check QTreeWidgetItem
            if(s->text().isEmpty()){
                item->setIcon(0, AppContext::getExternalToolRegistry()->getByName(item->text(0))->getGrayIcon());
                externalToolsInfo[item->text(0)].path="";
                externalToolsInfo[item->text(0)].valid=false;
                externalToolsInfo[item->text(0)].version="";
            }else{
                ExternalToolValidateTask* validateTask=new ExternalToolValidateTask(item->text(0), path);
                connect(validateTask,SIGNAL(si_stateChanged()),SLOT(sl_validateTaskStateChanged()));
                AppContext::getTaskScheduler()->registerTopLevelTask(validateTask);
                externalToolsInfo[item->text(0)].path=path;
            }

        }
    }
}
void ExternalToolSupportSettingsPageWidget::sl_validateTaskStateChanged(){
    ExternalToolValidateTask* s=qobject_cast<ExternalToolValidateTask*>(sender());
    assert(s);
    if(s->isFinished()){
        QList<QTreeWidgetItem*> listOfItems=treeWidget->findItems(s->getToolName(),Qt::MatchExactly|Qt::MatchRecursive);
        assert(listOfItems.length()==1);
        if(s->isValidTool()){
            listOfItems.at(0)->setIcon(0, AppContext::getExternalToolRegistry()->getByName(s->getToolName())->getIcon());
            externalToolsInfo[s->getToolName()].valid=true;
        }else{
            listOfItems.at(0)->setIcon(0, AppContext::getExternalToolRegistry()->getByName(s->getToolName())->getWarnIcon());
            externalToolsInfo[s->getToolName()].valid=false;
        }
        externalToolsInfo[s->getToolName()].version=s->getToolVersion();
        descriptionTextEdit->setText(AppContext::getExternalToolRegistry()->getByName(s->getToolName())->getDescription());
        descriptionTextEdit->setText(descriptionTextEdit->toHtml()+tr("<br>Version: ")+externalToolsInfo[s->getToolName()].version);
        descriptionTextEdit->setText(descriptionTextEdit->toHtml()+tr("<br>Binary path: ")+externalToolsInfo[s->getToolName()].path);
    }
}
void ExternalToolSupportSettingsPageWidget::sl_itemSelectionChanged(){
    QList<QTreeWidgetItem *> selectedItems=treeWidget->selectedItems();
    if(selectedItems.length()==0){
        descriptionTextEdit->setText(tr("Select an external tool to view more information about it."));
        return;
    }
    assert(selectedItems.length()==1);
    QString name=selectedItems.at(0)->text(0);
    if((selectedItems.at(0)->text(0) != "BLAST") && (selectedItems.at(0)->text(0) != "BLAST+")){
        descriptionTextEdit->setText(AppContext::getExternalToolRegistry()->getByName(selectedItems.at(0)->text(0))->getDescription());
        if(!externalToolsInfo[selectedItems.at(0)->text(0)].version.isEmpty()){
            descriptionTextEdit->setText(descriptionTextEdit->toHtml()+tr("<br>Version: ")+externalToolsInfo[selectedItems.at(0)->text(0)].version);
            descriptionTextEdit->setText(descriptionTextEdit->toHtml()+tr("<br>Binary path: ")+externalToolsInfo[selectedItems.at(0)->text(0)].path);
        }
    }
    if(selectedItems.at(0)->text(0) == "BLAST"){
        descriptionTextEdit->setText(tr("The <i>Basic Local Alignment Search Tool</i> (BLAST) finds regions of local similarity between sequences. "
                           "The program compares nucleotide or protein sequences to sequence databases and calculates the statistical significance of matches. "
                          "BLAST can be used to infer functional and evolutionary relationships between sequences as well as help identify members of gene families."));

    }
    if(selectedItems.at(0)->text(0) == "BLAST+"){
        descriptionTextEdit->setText(tr("<i>BLAST+</i> is a new version of the BLAST package from the NCBI."));
    }
}
void ExternalToolSupportSettingsPageWidget::sl_onPathEditWidgetClick(){
    QWidget* s=qobject_cast<QWidget*>(sender());
    assert(s);
    QList<QTreeWidgetItem*> listOfItems=treeWidget->findItems("",Qt::MatchContains|Qt::MatchRecursive);
    assert(listOfItems.length()!=0);
    treeWidget->clearSelection();
    foreach(QTreeWidgetItem* item, listOfItems){
        QWidget* par=s->parentWidget();
        QWidget* itemWid=treeWidget->itemWidget(item,1);
        if(par == itemWid){
            item->setSelected(true);

        }
    }
}
void ExternalToolSupportSettingsPageWidget::sl_onBrowseToolKitPath(){
    LastOpenDirHelper lod("toolkit path");
    QString dir;

    lod.url = dir = QFileDialog::getExistingDirectory(this, tr("Choose Directory"), lod.dir, QFileDialog::ShowDirsOnly | QFileDialog::DontResolveSymlinks);
    if (!dir.isEmpty()) {
        assert(treeWidget->selectedItems().isEmpty()==0);
        QString toolKitName=treeWidget->selectedItems().first()->text(0);
        QList<QTreeWidgetItem*> listOfItems=treeWidget->findItems("",Qt::MatchContains|Qt::MatchRecursive);
        assert(listOfItems.length()!=0);
        foreach(QTreeWidgetItem* item, listOfItems){
            if(AppContext::getExternalToolRegistry()->getByName(item->text(0)) != NULL){
                if(AppContext::getExternalToolRegistry()->getByName(item->text(0))->getToolKitName() == toolKitName){
                    QWidget* itemWid=treeWidget->itemWidget(item,1);
                    PathLineEdit* lineEdit=itemWid->findChild<PathLineEdit*>("PathLineEdit");
                    if(lineEdit->text().isEmpty()){
                        QString path=QDir::toNativeSeparators(dir+"/"+AppContext::getExternalToolRegistry()->getByName(item->text(0))->getExecutableFileName());
                        lineEdit->setText(path);
                        lineEdit->setModified(false);
                        externalToolsInfo[item->text(0)].path=path;
                        QToolButton* clearToolPathButton = itemWid->findChild<QToolButton*>("ClearToolPathButton");
                        assert(clearToolPathButton);
                        clearToolPathButton->setEnabled(true);
                        ExternalToolValidateTask* validateTask=new ExternalToolValidateTask(item->text(0), path);
                        connect(validateTask,SIGNAL(si_stateChanged()),SLOT(sl_validateTaskStateChanged()));
                        AppContext::getTaskScheduler()->registerTopLevelTask(validateTask);
                    }
                }
            }
        }
    }
}
////////////////////////////////////////
//PathLineEdit
void PathLineEdit::sl_onBrowse() {
    LastOpenDirHelper lod(type);

    QString name;
    if(text().isEmpty()){
        lod.url = name = QFileDialog::getOpenFileName(NULL, tr("Select a file"), lod.dir, FileFilter, 0, QFileDialog::DontConfirmOverwrite);
    }else{
        lod.url = name = QFileDialog::getOpenFileName(NULL, tr("Select a file"), text(), FileFilter, 0, QFileDialog::DontConfirmOverwrite);
    }
    if (!name.isEmpty()) {
        setText(QDir::toNativeSeparators(name));
        setModified(true);
        emit editingFinished();
    }
    QToolButton* clearToolPathButton = this->parentWidget()->findChild<QToolButton*>("ClearToolPathButton");
    assert(clearToolPathButton);
    clearToolPathButton->setEnabled(!text().isEmpty());
    setFocus();
}

void PathLineEdit::sl_clear(){
    QToolButton* s=qobject_cast<QToolButton*>(sender());
    assert(s);
    setText("");
    s->setEnabled(false);
    setModified(true);
    emit editingFinished();
}

} //namespace

