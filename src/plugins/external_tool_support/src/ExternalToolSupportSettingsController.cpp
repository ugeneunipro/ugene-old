/**
 * UGENE - Integrated Bioinformatics Tools.
 * Copyright (C) 2008-2013 UniPro <ugene@unipro.ru>
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

#include "ExternalToolSupportSettingsController.h"
#include "ExternalToolSupportSettings.h"
#include "utils/ExternalToolValidateTask.h"

#include <U2Core/AppContext.h>
#include <U2Core/L10n.h>
#include <U2Core/U2SafePoints.h>

#include <U2Gui/LastUsedDirHelper.h>
#include <U2Gui/GUIUtils.h>

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
    QHBoxLayout* buttonsLayout = new QHBoxLayout();
    buttonsLayout->addWidget(selectToolPathButton);
    buttonsLayout->addWidget(clearToolPathButton);
    layout->addLayout(buttonsLayout);
    buttonsWidth=buttonsLayout->minimumSize().width();

    return widget;
}
void ExternalToolSupportSettingsPageWidget::sl_linkActivated(QString url){
    GUIUtils::runWebBrowser(url);
}
void ExternalToolSupportSettingsPageWidget::setState(AppSettingsGUIPageState* s) {
    ExternalToolSupportSettingsPageState* state = qobject_cast<ExternalToolSupportSettingsPageState*>(s);
    connect(selectToolPackButton, SIGNAL(clicked()), this, SLOT(sl_onBrowseToolPackPath()));
    connect(linkLabel,SIGNAL(linkActivated(QString)),this,SLOT(sl_linkActivated(QString)));
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
    
    if(!s || !s->isModified()){
        return;
    }

    QWidget* par=s->parentWidget();
    QString path=s->text();
    s->setModified(false);

    QList<QTreeWidgetItem*> listOfItems=treeWidget->findItems("",Qt::MatchContains|Qt::MatchRecursive);
    SAFE_POINT(listOfItems.length()!=0, "ExternalToolSupportSettings, NO items're selected", );
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
    SAFE_POINT(selectedItems.length()!=0, "ExternalToolSupportSettings, NO items're selected", );
    QString name=selectedItems.at(0)->text(0);
    if(name == "BLAST"){
        descriptionTextEdit->setText(tr("The <i>Basic Local Alignment Search Tool</i> (BLAST) finds regions of local similarity between sequences. "
                           "The program compares nucleotide or protein sequences to sequence databases and calculates the statistical significance of matches. "
                          "BLAST can be used to infer functional and evolutionary relationships between sequences as well as help identify members of gene families."));

    }
    else if(name == "CUDA-BLAST"){
        descriptionTextEdit->setText(tr("The <i>Basic Local Alignment Search Tool</i> (BLAST) finds regions of local similarity between sequences. "
                           "The program compares nucleotide or protein sequences to sequence databases and calculates the statistical significance of matches. "
                          "BLAST can be used to infer functional and evolutionary relationships between sequences as well as help identify members of gene families."));

    }
    else if(name == "BLAST+"){
        descriptionTextEdit->setText(tr("<i>BLAST+</i> is a new version of the BLAST package from the NCBI."));
    }
    else if(name == "GPU-BLAST+"){
        descriptionTextEdit->setText(tr("<i>BLAST+</i> is a new version of the BLAST package from the NCBI."));
    }
    else if(name == "Bowtie"){
        descriptionTextEdit->setText(tr("<i>Bowtie<i> is an ultrafast, memory-efficient short read aligner. "
                       "It aligns short DNA sequences (reads) to the human genome at "
                       "a rate of over 25 million 35-bp reads per hour. "
                       "Bowtie indexes the genome with a Burrows-Wheeler index to keep "
                       "its memory footprint small: typically about 2.2 GB for the human "
                       "genome (2.9 GB for paired-end)."));
    }
    else if(name == "Cufflinks Tools"){
        descriptionTextEdit->setText(tr("<i>Cufflinks</i> assembles transcripts, estimates"
            " their abundances, and tests for differential expression and regulation"
            " in RNA-Seq samples. It accepts aligned RNA-Seq reads and assembles"
            " the alignments into a parsimonious set of transcripts. It also estimates"
            " the relative abundances of these transcripts based on how many reads"
            " support each one, taking into account biases in library preparation protocols. "));
    }
    else if(name == "Bowtie 2 Tools"){
        descriptionTextEdit->setText(tr("<i>Bowtie 2</i> is an ultrafast and memory-efficient tool"
            " for aligning sequencing reads to long reference sequences. It is particularly good"
            " at aligning reads of about 50 up to 100s or 1000s of characters, and particularly"
            " good at aligning to relatively long (e.g. mammalian) genomes."
            " <br/><br/>It indexes the genome with an FM index to keep its memory footprint small:"
            " for the human genome, its memory footprint is typically around 3.2Gb."
            " <br/><br/><i>Bowtie 2</i> supports gapped, local, and paired-end alignment modes."));
    }
    else if(name == "Cistrome"){
        descriptionTextEdit->setText(tr("<i>Cistrome</i> is a UGENE version of Cistrome pipeline which also includes some tools useful for ChIP-seq analysis"
            "This pipeline is aimed to provide the following analysis steps: peak calling and annotating, motif search and gene ontology."));
    }else{ //no description or tool custom description
        ExternalTool* tool = AppContext::getExternalToolRegistry()->getByName(name);
        if (!tool){
            descriptionTextEdit->setText(tr("No description"));
        }else{
            descriptionTextEdit->setText(tool->getDescription());
            if(!externalToolsInfo[name].version.isEmpty()){
                descriptionTextEdit->setText(descriptionTextEdit->toHtml()+tr("<br>Version: ")+externalToolsInfo[name].version);
                descriptionTextEdit->setText(descriptionTextEdit->toHtml()+tr("<br>Binary path: ")+externalToolsInfo[name].path);
            }
        }
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
//looks in selected folder +1 level 1 subfolders
void ExternalToolSupportSettingsPageWidget::sl_onBrowseToolKitPath(){
    LastUsedDirHelper lod("toolkit path");
    QString dir;

    lod.url = dir = QFileDialog::getExistingDirectory(this, tr("Choose Directory With Executables"), lod.dir, QFileDialog::ShowDirsOnly | QFileDialog::DontResolveSymlinks);
    if (!dir.isEmpty()) {
        assert(treeWidget->selectedItems().isEmpty()==0);
        QString toolKitName=treeWidget->selectedItems().first()->text(0);
        QList<QTreeWidgetItem*> listOfItems=treeWidget->findItems("",Qt::MatchContains|Qt::MatchRecursive);
        assert(listOfItems.length()!=0);
        foreach(QTreeWidgetItem* item, listOfItems){
            QString toolItemText = item->text(0);
            if(AppContext::getExternalToolRegistry()->getByName(toolItemText) != NULL){
                if(AppContext::getExternalToolRegistry()->getByName(item->text(0))->getToolKitName() == toolKitName){
                    QWidget* itemWid=treeWidget->itemWidget(item,1);
                    PathLineEdit* lineEdit=itemWid->findChild<PathLineEdit*>("PathLineEdit");
                    if(lineEdit->text().isEmpty()){
                        QDirIterator it(dir,QDirIterator::Subdirectories);
                        bool fileNotFound=true;
                        QString executableFileName=AppContext::getExternalToolRegistry()->getByName(item->text(0))->getExecutableFileName();
                        while (it.hasNext()&&fileNotFound) {
                            it.next();
                            QString fpath = it.filePath()+"/"+executableFileName;
                            QFileInfo info(fpath);
                            //if(info.exists() && info.isFile() && info.isExecutable()){
                            if(info.exists() && info.isFile()){
                                QString path=QDir::toNativeSeparators(fpath);
                                lineEdit->setText(path);
                                lineEdit->setModified(false);
                                externalToolsInfo[item->text(0)].path=path;
                                QToolButton* clearToolPathButton = itemWid->findChild<QToolButton*>("ClearToolPathButton");
                                assert(clearToolPathButton);
                                clearToolPathButton->setEnabled(true);
                                ExternalToolValidateTask* validateTask=new ExternalToolValidateTask(item->text(0), path);
                                connect(validateTask,SIGNAL(si_stateChanged()),SLOT(sl_validateTaskStateChanged()));
                                AppContext::getTaskScheduler()->registerTopLevelTask(validateTask);
                                fileNotFound=false;
                            }
                        }
                    }
                }
            }
        }
    }
}
void ExternalToolSupportSettingsPageWidget::sl_onBrowseToolPackPath(){
    LastUsedDirHelper lod("toolpack path");
    QString dirPath;
    bool isPathValid=false;
    lod.url = dirPath = QFileDialog::getExistingDirectory(this, tr("Choose Directory With External Tools Pack"), lod.dir, QFileDialog::ShowDirsOnly | QFileDialog::DontResolveSymlinks);
    if (!dirPath.isEmpty()) {
        QDir dir = QDir(dirPath);
        QList<QTreeWidgetItem*> listOfItems=treeWidget->findItems("",Qt::MatchContains|Qt::MatchRecursive);
        assert(listOfItems.length()!=0);
        foreach(QString dirName, dir.entryList(QDir::Dirs | QDir::NoDotAndDotDot)){
            foreach(QTreeWidgetItem* item, listOfItems){
                if (item->childCount() > 0) {
                    // those items are only for grouping
                    continue;
                }
                if(AppContext::getExternalToolRegistry()->getByName(item->text(0)) != NULL){
                    QString toolKitName=AppContext::getExternalToolRegistry()->getByName(item->text(0))->getToolKitName();
                    if(dirName.contains(toolKitName,Qt::CaseInsensitive)){
                        isPathValid=true;
                        QWidget* itemWid=treeWidget->itemWidget(item,1);
                        PathLineEdit* lineEdit=itemWid->findChild<PathLineEdit*>("PathLineEdit");
                        if(lineEdit->text().isEmpty()){
                            QString toolPath = dirPath + "/" + dirName;
                            QDirIterator it(toolPath,QDirIterator::Subdirectories);
                            bool fileNotFound=true;
                            QString executableFileName = AppContext::getExternalToolRegistry()->getByName(item->text(0))->getExecutableFileName();
                            while (it.hasNext()&&fileNotFound) {
                                it.next();
                                QFileInfo info(it.filePath()+"/"+executableFileName);
                                if(info.exists() && info.isFile() && info.isExecutable()){
                                    QString path=QDir::toNativeSeparators(it.filePath()+"/"+executableFileName);
                                    lineEdit->setText(path);
                                    lineEdit->setModified(false);
                                    externalToolsInfo[item->text(0)].path=path;
                                    QToolButton* clearToolPathButton = itemWid->findChild<QToolButton*>("ClearToolPathButton");
                                    assert(clearToolPathButton);
                                    clearToolPathButton->setEnabled(true);
                                    ExternalToolValidateTask* validateTask=new ExternalToolValidateTask(item->text(0), path);
                                    connect(validateTask,SIGNAL(si_stateChanged()),SLOT(sl_validateTaskStateChanged()));
                                    AppContext::getTaskScheduler()->registerTopLevelTask(validateTask);
                                    fileNotFound=false;
                                }

                            }
                        }
                    }
                }
            }
        }
        if(!isPathValid){
            QMessageBox::warning(this, L10N::warningTitle(),
                                            tr("Not a valid external tools directory"),
                                            QMessageBox::Ok);
        }
    }
}
////////////////////////////////////////
//PathLineEdit
void PathLineEdit::sl_onBrowse() {
    LastUsedDirHelper lod(type);

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

