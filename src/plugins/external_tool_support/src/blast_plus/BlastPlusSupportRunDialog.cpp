/**
 * UGENE - Integrated Bioinformatics Tools.
 * Copyright (C) 2008-2015 UniPro <ugene@unipro.ru>
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

#include <QMainWindow>
#include <QMessageBox>
#include <QToolButton>

#include <U2Core/AnnotationTableObject.h>
#include <U2Core/AppContext.h>
#include <U2Core/AppResources.h>
#include <U2Core/AppSettings.h>
#include <U2Core/DNAAlphabet.h>
#include <U2Core/DNASequenceObject.h>
#include <U2Core/DocumentUtils.h>
#include <U2Core/ExternalToolRegistry.h>
#include <U2Core/GObjectReference.h>
#include <U2Core/GObjectRelationRoles.h>
#include <U2Core/IOAdapter.h>
#include <U2Core/IOAdapterUtils.h>
#include <U2Core/L10n.h>
#include <U2Core/LoadDocumentTask.h>
#include <U2Core/ProjectModel.h>
#include <U2Core/ProjectService.h>
#include <U2Core/U2DbiRegistry.h>
#include <U2Core/U2OpStatusUtils.h>

#include <U2Gui/AppSettingsGUI.h>
#include <U2Gui/CreateAnnotationWidgetController.h>
#include <U2Gui/DialogUtils.h>
#include <U2Gui/GUIUtils.h>
#include <U2Gui/OpenViewTask.h>
#include <U2Core/QObjectScopedPointer.h>

#include "BlastPlusSupport.h"
#include "BlastPlusSupportRunDialog.h"
#include "ExternalToolSupportSettingsController.h"

namespace U2 {

namespace {
    QStringList getCompValues() {
        QStringList result;
        result << "blastp";
        result << "blastx";
        result << "tblastn";
        return result;
    }
}

////////////////////////////////////////
//BlastAllSupportRunDialog
BlastPlusSupportRunDialog::BlastPlusSupportRunDialog(U2SequenceObject *dnaso, QString &lastDBPath, QString &lastDBName, QWidget *parent)
: BlastRunCommonDialog(parent, BlastPlus, true, getCompValues()), dnaso(dnaso), lastDBPath(lastDBPath), lastDBName(lastDBName)
{
    CreateAnnotationModel ca_m;
    ca_m.hideAnnotationType = true;
    ca_m.hideAnnotationName = true;
    ca_m.hideLocation = true;
    ca_m.sequenceObjectRef = GObjectReference(dnaso);
    ca_m.sequenceLen = dnaso->getSequenceLength();
    ca_c = new CreateAnnotationWidgetController(ca_m, this);
    //lowerCaseCheckBox->hide();
    QWidget *wdgt = ca_c->getWidget();
    wdgt->setMinimumHeight(150);
    verticalLayout_4->addWidget(wdgt);

    //programName->removeItem(3);//cuda-blastp
    if(dnaso->getAlphabet()->getType() == DNAAlphabet_AMINO){
        programName->removeItem(0);//blastn
        programName->removeItem(1);//blastx
        programName->removeItem(2);//tblastx
        settings.isNucleotideSeq=false;
    }else{
        programName->removeItem(1);//blastp
        programName->removeItem(1);//gpu-blastp
        programName->removeItem(2);//tblastn
        settings.isNucleotideSeq=true;
    }
    dbSelector->databasePathLineEdit->setText(lastDBPath);
    dbSelector->baseNameLineEdit->setText(lastDBName);
    connect(cancelButton,SIGNAL(clicked()),SLOT(reject()));
}

void BlastPlusSupportRunDialog::sl_lineEditChanged(){
    okButton->setEnabled(dbSelector->isInputDataValid());
}

bool BlastPlusSupportRunDialog::checkToolPath(){
    bool needSetToolPath=false;
    QString toolName;
    QObjectScopedPointer<QMessageBox> msgBox = new QMessageBox;
    msgBox->setWindowTitle("BLAST+ Search");
    msgBox->setInformativeText(tr("Do you want to select it now?"));
    msgBox->setStandardButtons(QMessageBox::Yes | QMessageBox::No);
    msgBox->setDefaultButton(QMessageBox::Yes);
    if((programName->currentText() == "blastn") &&
       (AppContext::getExternalToolRegistry()->getByName(ET_BLASTN)->getPath().isEmpty())){
        needSetToolPath=true;
        toolName=ET_BLASTN;

    }else if((programName->currentText() == "blastp") &&
             (AppContext::getExternalToolRegistry()->getByName(ET_BLASTP)->getPath().isEmpty())){
        needSetToolPath=true;
        toolName=ET_BLASTP;

// https://ugene.unipro.ru/tracker/browse/UGENE-945
//     }else if((programName->currentText() == "gpu-blastp") &&
//              (AppContext::getExternalToolRegistry()->getByName(GPU_BLASTP_TOOL_NAME)->getPath().isEmpty())){
//         needSetToolPath=true;
//         toolName=GPU_BLASTP_TOOL_NAME;

    }else if((programName->currentText() == "blastx") &&
             (AppContext::getExternalToolRegistry()->getByName(ET_BLASTX)->getPath().isEmpty())){
        needSetToolPath=true;
        toolName=ET_BLASTX;

    }else if((programName->currentText() == "tblastn") &&
             (AppContext::getExternalToolRegistry()->getByName(ET_TBLASTN)->getPath().isEmpty())){
        needSetToolPath=true;
        toolName=ET_TBLASTN;

    }else if((programName->currentText() == "tblastx") &&
             (AppContext::getExternalToolRegistry()->getByName(ET_TBLASTX)->getPath().isEmpty())){
        needSetToolPath=true;
        toolName=ET_TBLASTX;
    }
    if(needSetToolPath){
        msgBox->setText(tr("Path for <i>BLAST+ %1</i> tool not selected.").arg(toolName));
        const int ret = msgBox->exec();
        CHECK(!msgBox.isNull(), false);

        switch (ret) {
           case QMessageBox::Yes:
               AppContext::getAppSettingsGUI()->showSettingsDialog(ExternalToolSupportSettingsPageId);
               break;
           case QMessageBox::No:
               return false;
               break;
           default:
               assert(false);
               break;
        }
        if(!AppContext::getExternalToolRegistry()->getByName(toolName)->getPath().isEmpty()){
            return true;
        }else{
            return false;
        }

    }else{
        return true;
    }
}

void BlastPlusSupportRunDialog::sl_runQuery(){

    if(!checkToolPath()){
        return;
    }

    QString error = ca_c->validate();
    if (!error.isEmpty()) {
        QMessageBox::critical(NULL, tr("Wrong parameters for creating annotations"), error);
        return;
    }
    settings.outputResFile=ca_c->getModel().newDocUrl;
    if(ca_c->isNewObject()) {
        U2OpStatusImpl os;
        const U2DbiRef dbiRef = AppContext::getDbiRegistry( )->getSessionTmpDbiRef( os );
        SAFE_POINT_OP( os, );
        settings.aobj = new AnnotationTableObject( "Annotations", dbiRef );
        settings.aobj->addObjectRelation(GObjectRelation(ca_c->getModel().sequenceObjectRef, ObjectRole_Sequence));
    }
    else {
        bool objectPrepared = ca_c->prepareAnnotationObject();
        if (!objectPrepared){
            QMessageBox::warning(this, tr("Error"), tr("Cannot create an annotation object. Please check settings"));
            return;
        }
        settings.aobj = ca_c->getModel().getAnnotationObject();
    }
    settings.groupName = ca_c->getModel().groupName;
    settings.annDescription = ca_c->getModel().description;

    getSettings(settings);
    settings.alphabet = dnaso->getAlphabet();
    lastDBPath = dbSelector->databasePathLineEdit->text();
    lastDBName = dbSelector->baseNameLineEdit->text();
    settings.outputType = 5;//By default set output file format to xml
    accept();
}
////////////////////////////////////////
//BlastPlusWithExtFileSpecifySupportRunDialog
BlastPlusWithExtFileSpecifySupportRunDialog::BlastPlusWithExtFileSpecifySupportRunDialog(QString &lastDBPath, QString &lastDBName, QWidget *parent)
: BlastRunCommonDialog(parent, BlastPlus, true, getCompValues()), lastDBPath(lastDBPath), lastDBName(lastDBName), hasValidInput(false)
{
    ca_c=NULL;
    wasNoOpenProject=false;
    //create input file widget
    QWidget * widget = new QWidget(parent);
    inputFileLineEdit= new FileLineEdit("","", false, widget);
    inputFileLineEdit->setReadOnly(true);
    inputFileLineEdit->setText("");
    QToolButton * selectToolPathButton = new QToolButton(widget);
    selectToolPathButton->setObjectName("browseInput");
    selectToolPathButton->setVisible(true);
    selectToolPathButton->setText("...");
    connect(selectToolPathButton, SIGNAL(clicked()), inputFileLineEdit, SLOT(sl_onBrowse()));
    connect(inputFileLineEdit,SIGNAL(textChanged(QString)),this, SLOT(sl_inputFileLineEditChanged(QString)));

    QHBoxLayout* layout = new QHBoxLayout(widget);
    layout->addWidget(inputFileLineEdit);
    layout->addWidget(selectToolPathButton);

    QGroupBox* inputFileGroupBox=new QGroupBox(tr("Select input file"),widget);
    inputFileGroupBox->setLayout(layout);
    QBoxLayout* parentLayout = qobject_cast<QBoxLayout*>(this->layout());
    assert(parentLayout);
    parentLayout->insertWidget(0, inputFileGroupBox);

    programName->removeItem(3);//cuda-blastp

    dbSelector->databasePathLineEdit->setText(lastDBPath);
    dbSelector->baseNameLineEdit->setText(lastDBName);
    connect(cancelButton,SIGNAL(clicked()),SLOT(sl_cancel()));
    connect(this,SIGNAL(rejected()),SLOT(sl_cancel()));
}

const QList<BlastTaskSettings> &BlastPlusWithExtFileSpecifySupportRunDialog::getSettingsList() const {
    return settingsList;
}

void BlastPlusWithExtFileSpecifySupportRunDialog::sl_lineEditChanged(){
    okButton->setEnabled(dbSelector->isInputDataValid() && hasValidInput);
}

void BlastPlusWithExtFileSpecifySupportRunDialog::sl_inputFileLineEditChanged(const QString &url){
    hasValidInput = false;
    sl_lineEditChanged();
    CHECK(!url.isEmpty(), );

    Project *proj = AppContext::getProject();
    if (NULL == proj) {
        wasNoOpenProject = true;
    } else {
        Document *doc = proj->findDocumentByURL(url);
        if (NULL != doc) {
            tryApplyDoc(doc);
            return;
        }
    }

    loadDoc(url);
}

namespace {
    const char *INPUT_URL_PROP = "input_url";
}

void BlastPlusWithExtFileSpecifySupportRunDialog::onFormatError() {
    QMessageBox::critical(this, tr("Wrong input file"), tr("This file has the incompatible format for the BLAST+ search."));
    inputFileLineEdit->setText("");
}

void BlastPlusWithExtFileSpecifySupportRunDialog::loadDoc(const QString &url) {
    FormatDetectionConfig config;
    config.useExtensionBonus = true;
    QList<FormatDetectionResult> formats = DocumentUtils::detectFormat(url, config);
    CHECK_EXT(!formats.isEmpty() && (NULL != formats.first().format), onFormatError(), );

    DocumentFormat *format = formats.first().format;
    CHECK_EXT(format->getSupportedObjectTypes().contains(GObjectTypes::SEQUENCE), onFormatError(), );

    LoadDocumentTask *loadTask= new LoadDocumentTask(format->getFormatId(), url, AppContext::getIOAdapterRegistry()->getIOAdapterFactoryById(IOAdapterUtils::url2io(url)));
    AddDocumentAndOpenViewTask *openTask = new AddDocumentAndOpenViewTask(loadTask);
    openTask->setProperty(INPUT_URL_PROP, url);

    connect(openTask, SIGNAL(si_stateChanged()), SLOT(sl_inputFileOpened()));
    AppContext::getTaskScheduler()->registerTopLevelTask(openTask);
}

void BlastPlusWithExtFileSpecifySupportRunDialog::sl_inputFileOpened() {
    Task *t = qobject_cast<Task*>(sender());
    CHECK(NULL != t, );
    CHECK(t->isFinished() && !t->hasError(), );

    Project *proj = AppContext::getProject();
    SAFE_POINT(NULL != proj, "No opened project", );

    QString url = t->property(INPUT_URL_PROP).toString();
    Document *doc = proj->findDocumentByURL(url);
    SAFE_POINT(NULL != doc, "No loaded document", );

    tryApplyDoc(doc);
}

void BlastPlusWithExtFileSpecifySupportRunDialog::tryApplyDoc(Document *doc) {
    int numOfSequences = 0;
    foreach (GObject *obj, doc->getObjects()) {
        if (obj->getGObjectType() == GObjectTypes::SEQUENCE) {
            numOfSequences++;
        }
    }
    settingsList.clear();
    sequencesRefList.clear();
    if (0 == numOfSequences) {
        QMessageBox::critical(this, tr("Wrong input file"), tr("This file does not contain sequences."));
        inputFileLineEdit->setText("");
        return;
    }

    hasValidInput = true;
    foreach (GObject *obj, doc->getObjects()) {
        if (obj->getGObjectType() != GObjectTypes::SEQUENCE) {
            continue;
        }
        U2SequenceObject *seq = dynamic_cast<U2SequenceObject*>(obj);
        SAFE_POINT(NULL != seq, "NULL sequence object", );

        BlastTaskSettings localSettings;
        U2OpStatusImpl os;
        localSettings.querySequence = seq->getWholeSequenceData(os);
        CHECK_OP_EXT(os, QMessageBox::critical(this, L10N::errorTitle(), os.getError()), );
        localSettings.alphabet = seq->getAlphabet();
        if (localSettings.alphabet->getType() != DNAAlphabet_AMINO){
            localSettings.isNucleotideSeq = true;
        }
        localSettings.queryFile = doc->getURLString();
        settingsList.append(localSettings);
        sequencesRefList.append(GObjectReference(obj));
    }
    settings = settingsList.first();

    CreateAnnotationModel ca_m;
    ca_m.hideAnnotationType = true;
    ca_m.hideAnnotationName = true;
    ca_m.hideLocation = true;
    ca_m.sequenceObjectRef = sequencesRefList[0];
    ca_m.sequenceLen = 10;
    ca_m.defaultIsNewDoc = true;
    if (NULL == ca_c) {
        ca_c = new CreateAnnotationWidgetController(ca_m, this);
        QWidget *wdgt = ca_c->getWidget();
        wdgt->setMinimumHeight(150);
        verticalLayout_4->addWidget(wdgt);
    } else {
        ca_c->updateWidgetForAnnotationModel(ca_m);
    }

    sl_lineEditChanged();
}

bool BlastPlusWithExtFileSpecifySupportRunDialog::checkToolPath(){

    bool needSetToolPath=false;
    QString toolName;
    QObjectScopedPointer<QMessageBox> msgBox = new QMessageBox;
    msgBox->setWindowTitle("BLAST+ Search");
    msgBox->setInformativeText(tr("Do you want to select it now?"));
    msgBox->setStandardButtons(QMessageBox::Yes | QMessageBox::No);
    msgBox->setDefaultButton(QMessageBox::Yes);
    if((programName->currentText() == "blastn") &&
       (AppContext::getExternalToolRegistry()->getByName(ET_BLASTN)->getPath().isEmpty())){
        needSetToolPath=true;
        toolName=ET_BLASTN;

    }else if((programName->currentText() == "blastp") &&
             (AppContext::getExternalToolRegistry()->getByName(ET_BLASTP)->getPath().isEmpty())){
        needSetToolPath=true;
        toolName=ET_BLASTP;

// https://ugene.unipro.ru/tracker/browse/UGENE-945
//     }else if((programName->currentText() == "gpu-blastp") &&
//              (AppContext::getExternalToolRegistry()->getByName(GPU_BLASTP_TOOL_NAME)->getPath().isEmpty())){
//         needSetToolPath=true;
//         toolName=GPU_BLASTP_TOOL_NAME;

    }else if((programName->currentText() == "blastx") &&
             (AppContext::getExternalToolRegistry()->getByName(ET_BLASTX)->getPath().isEmpty())){
        needSetToolPath=true;
        toolName=ET_BLASTX;

    }else if((programName->currentText() == "tblastn") &&
             (AppContext::getExternalToolRegistry()->getByName(ET_TBLASTN)->getPath().isEmpty())){
        needSetToolPath=true;
        toolName=ET_TBLASTN;

    }else if((programName->currentText() == "tblastx") &&
             (AppContext::getExternalToolRegistry()->getByName(ET_TBLASTX)->getPath().isEmpty())){
        needSetToolPath=true;
        toolName=ET_TBLASTX;
    }
    if(needSetToolPath){
        msgBox->setText(tr("Path for <i>BLAST+ %1</i> tool not selected.").arg(toolName));
        const int ret = msgBox->exec();
        CHECK(!msgBox.isNull(), false);

        switch (ret) {
           case QMessageBox::Yes:
               AppContext::getAppSettingsGUI()->showSettingsDialog(ExternalToolSupportSettingsPageId);
               break;
           case QMessageBox::No:
               return false;
               break;
           default:
               assert(false);
               break;
        }
        if(!AppContext::getExternalToolRegistry()->getByName(toolName)->getPath().isEmpty()){
            return true;
        }else{
            return false;
        }

    }else{
        return true;
    }
}

void BlastPlusWithExtFileSpecifySupportRunDialog::sl_runQuery(){

    if(!checkToolPath()){
        return;
    }

    QString error = ca_c->validate();
    if (!error.isEmpty()) {
        QMessageBox::critical(NULL, tr("Wrong parameters for creating annotations"), error);
        return;
    }

    for(int i=0; i<settingsList.length();i++){
        settingsList[i].outputResFile=ca_c->getModel().newDocUrl;
        if(ca_c->isNewObject()) {
            U2OpStatusImpl os;
            const U2DbiRef dbiRef = AppContext::getDbiRegistry( )->getSessionTmpDbiRef( os );
            SAFE_POINT_OP( os, );
            settingsList[i].aobj = new AnnotationTableObject(sequencesRefList[i].objName+" annotations", dbiRef);
            settingsList[i].aobj->addObjectRelation(GObjectRelation(sequencesRefList[i], ObjectRole_Sequence));
        } else {
            assert(false);//always created new document for annotations
        }
        settingsList[i].groupName=ca_c->getModel().groupName;

        getSettings(settingsList[i]);
        settingsList[i].outputType = 5;//By default set output file format to xml
    }
    bool docAlreadyInProject=false;
    Project* proj=AppContext::getProject();
    foreach(Document* doc, proj->getDocuments()){
        if(doc->getURL() == inputFileLineEdit->text()){
            docAlreadyInProject=true;
        }
    }
    if(!docAlreadyInProject){
        QString url = inputFileLineEdit->text();
        Task * t = AppContext::getProjectLoader()->openWithProjectTask(url);
        if (t != NULL) {
            AppContext::getTaskScheduler()->registerTopLevelTask(t);
        }
    }
    lastDBPath = dbSelector->databasePathLineEdit->text();
    lastDBName = dbSelector->baseNameLineEdit->text();
    accept();
}
void BlastPlusWithExtFileSpecifySupportRunDialog::sl_cancel(){
    if(qobject_cast<BlastPlusWithExtFileSpecifySupportRunDialog*>(sender()) == NULL){
        reject();
        return;
    }
    if(wasNoOpenProject){
        ProjectService *projService = AppContext::getProjectService();
        CHECK(NULL != projService, );
        AppContext::getTaskScheduler()->registerTopLevelTask(projService->closeProjectTask());
    }
}
}//namespace
