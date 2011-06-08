/**
 * UGENE - Integrated Bioinformatics Tools.
 * Copyright (C) 2008-2011 UniPro <ugene@unipro.ru>
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

#include "BlastPlusSupportRunDialog.h"
#include "BlastPlusSupport.h"
#include "ExternalToolSupportSettingsController.h"

#include <U2Core/ExternalToolRegistry.h>
#include <U2Core/AppContext.h>
#include <U2Core/AppSettings.h>
#include <U2Core/AppResources.h>
#include <U2Core/DNAAlphabet.h>
#include <U2Core/GObjectReference.h>
#include <U2Core/IOAdapter.h>
#include <U2Core/ProjectModel.h>
#include <U2Core/GObjectRelationRoles.h>
#include <U2Core/DNASequenceObject.h>
#include <U2Gui/AppSettingsGUI.h>
#include <U2Misc/DialogUtils.h>
#include <U2Gui/CreateAnnotationWidgetController.h>
#include <U2Core/LoadDocumentTask.h>
#include <U2Core/MultiTask.h>
#include <U2Core/ProjectService.h>

#include <QtGui/QFileDialog>
#include <QtGui/QToolButton>
#include <QtGui/QMessageBox>
#include <QtGui/QMainWindow>

namespace U2 {

////////////////////////////////////////
//BlastAllSupportRunDialog
BlastPlusSupportRunDialog::BlastPlusSupportRunDialog(DNASequenceObject* _dnaso, BlastTaskSettings& _settings, QString &_lastDBPath, QString &_lastDBName, QWidget* _parent) :
        BlastRunCommonDialog(_settings, _parent), dnaso(_dnaso), lastDBPath(_lastDBPath), lastDBName(_lastDBName)
{
    CreateAnnotationModel ca_m;
    ca_m.data->name = "misc_feature";
    ca_m.hideAnnotationName = true;
    ca_m.hideLocation = true;
    ca_m.sequenceObjectRef = GObjectReference(dnaso);
    ca_m.sequenceLen = dnaso->getSequenceLen();
    ca_c = new CreateAnnotationWidgetController(ca_m, this);
    //lowerCaseCheckBox->hide();
    QWidget *wdgt = ca_c->getWidget();
    wdgt->setMinimumHeight(150);
    verticalLayout_4->addWidget(wdgt);

    if(dnaso->getAlphabet()->getType() == DNAAlphabet_AMINO){
        programName->removeItem(0);//blastn
        programName->removeItem(1);//blastx
        programName->removeItem(2);//tblastx
        settings.isNucleotideSeq=false;
    }else{
        programName->removeItem(1);//blastp
        programName->removeItem(2);//tblastn
        settings.isNucleotideSeq=true;
    }
    databasePathLineEdit->setText(lastDBPath);
    baseNameLineEdit->setText(lastDBName);
    connect(cancelButton,SIGNAL(clicked()),SLOT(reject()));
}

void BlastPlusSupportRunDialog::sl_lineEditChanged(){
    bool isFilledDatabasePathLineEdit = !databasePathLineEdit->text().isEmpty();
    bool isFilledBaseNameLineEdit = !baseNameLineEdit->text().isEmpty();
    okButton->setEnabled(isFilledBaseNameLineEdit && isFilledDatabasePathLineEdit);
}
bool BlastPlusSupportRunDialog::checkToolPath(){

    bool needSetToolPath=false;
    QString toolName;
    QMessageBox msgBox;
    msgBox.setWindowTitle("BLAST+ Search");
    msgBox.setInformativeText(tr("Do you want to select it now?"));
    msgBox.setStandardButtons(QMessageBox::Yes | QMessageBox::No);
    msgBox.setDefaultButton(QMessageBox::Yes);
    if((programName->currentText() == "blastn") &&
       (AppContext::getExternalToolRegistry()->getByName(BLASTN_TOOL_NAME)->getPath().isEmpty())){
        needSetToolPath=true;
        toolName=BLASTN_TOOL_NAME;

    }else if((programName->currentText() == "blastp") &&
             (AppContext::getExternalToolRegistry()->getByName(BLASTP_TOOL_NAME)->getPath().isEmpty())){
        needSetToolPath=true;
        toolName=BLASTP_TOOL_NAME;

    }else if((programName->currentText() == "blastx") &&
             (AppContext::getExternalToolRegistry()->getByName(BLASTX_TOOL_NAME)->getPath().isEmpty())){
        needSetToolPath=true;
        toolName=BLASTX_TOOL_NAME;

    }else if((programName->currentText() == "tblastn") &&
             (AppContext::getExternalToolRegistry()->getByName(TBLASTN_TOOL_NAME)->getPath().isEmpty())){
        needSetToolPath=true;
        toolName=TBLASTN_TOOL_NAME;

    }else if((programName->currentText() == "tblastx") &&
             (AppContext::getExternalToolRegistry()->getByName(TBLASTX_TOOL_NAME)->getPath().isEmpty())){
        needSetToolPath=true;
        toolName=TBLASTX_TOOL_NAME;
    }
    if(needSetToolPath){
        msgBox.setText(tr("Path for <i>BLAST+ %1</i> tool not selected.").arg(toolName));
        int ret = msgBox.exec();
        switch (ret) {
           case QMessageBox::Yes:
               AppContext::getAppSettingsGUI()->showSettingsDialog(ExternalToolSupportSettingsPageId);
               break;
           case QMessageBox::No:
               return false;
               break;
           default:
               assert(NULL);
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
        settings.aobj = new AnnotationTableObject("Annotations");
        settings.aobj->addObjectRelation(GObjectRelation(ca_c->getModel().sequenceObjectRef, GObjectRelationRole::SEQUENCE));
    }
    else {
        ca_c->prepareAnnotationObject();
        settings.aobj = ca_c->getModel().getAnnotationObject();
    }
    settings.groupName=ca_c->getModel().groupName;

    settings.databaseNameAndPath=databasePathLineEdit->text()+"/"+baseNameLineEdit->text();
    settings.expectValue=evalueSpinBox->value();
    settings.wordSize=wordSizeSpinBox->value();
    settings.megablast=megablastCheckBox->isChecked();
    settings.alphabet=dnaso->getAlphabet();
    settings.numberOfHits=numberOfHitsSpinBox->value();
    settings.numberOfProcessors=numberOfCPUSpinBox->value();

    settings.gapOpenCost=costsComboBox->currentText().split(" ").at(0).toInt();
    settings.gapExtendCost=costsComboBox->currentText().split(" ").at(1).toInt();
    //setup filters
    if(lowComplexityFilterCheckBox->isChecked()){
        settings.filter="L";
    }
    if(repeatsCheckBox->isChecked()){
        settings.filter=settings.filter.isEmpty() ? "R" : settings.filter+"; R";
    }
//    if(lowerCaseCheckBox->isChecked()){
//        settings.filter=settings.filter.isEmpty() ? "???" : settings.filter+"; ???";
//    }
    if(lookupMaskCheckBox->isChecked()){
        settings.filter=settings.filter.isEmpty() ? "m" : "m "+settings.filter;
    }

    if(settings.isNucleotideSeq){
        if((((scoresComboBox->currentText() == "1 -4") || (scoresComboBox->currentText() == "1 -3")) && costsComboBox->currentText()=="2 2") || //-G 2 -E 2
            ((scoresComboBox->currentText() == "1 -2") && costsComboBox->currentText()=="2 2") || //-G 2 -E 2
            ((scoresComboBox->currentText() == "1 -1") && costsComboBox->currentText()=="4 2") || //-G 4 -E 2
            (((scoresComboBox->currentText() == "2 -7") || (scoresComboBox->currentText() == "2 -5"))&& costsComboBox->currentText()=="4 4") || //-G 4 -E 4
            ((scoresComboBox->currentText() == "2 -3") && costsComboBox->currentText()=="6 4") || //-G 6 -E 4
            (((scoresComboBox->currentText() == "4 -5") || (scoresComboBox->currentText() == "5 -4")) && costsComboBox->currentText()=="12 8"))//-G 12 -E 8
        {
            settings.isDefaultCosts=true;
        }else{
            settings.isDefaultCosts=false;
        }
        settings.isDefautScores=(scoresComboBox->currentText() == "1 -3");
    }else{
        if(((matrixComboBox->currentText() == "PAM30") && costsComboBox->currentText()=="9 1") || //-G 9 -E 1
                ((matrixComboBox->currentText() == "PAM70") && costsComboBox->currentText()=="10 1") || //-G 10 -E 1
                ((matrixComboBox->currentText() == "BLOSUM45") && costsComboBox->currentText()=="15 2") ||
                ((matrixComboBox->currentText() == "BLOSUM62") && costsComboBox->currentText()=="11 1") ||
                ((matrixComboBox->currentText() == "BLOSUM80") && costsComboBox->currentText()=="10 1"))
        {
            settings.isDefaultCosts=true;
        }else{
            settings.isDefaultCosts=false;
        }
        settings.isDefaultMatrix=(matrixComboBox->currentText() == "BLOSUM62");

    }
    lastDBPath=databasePathLineEdit->text();
    lastDBName=baseNameLineEdit->text();
    settings.outputType=5;//By default set output file format to xml
    accept();
}
////////////////////////////////////////
//BlastPlusWithExtFileSpecifySupportRunDialog
BlastPlusWithExtFileSpecifySupportRunDialog::BlastPlusWithExtFileSpecifySupportRunDialog(QList<BlastTaskSettings>& _settingsList, QString &_lastDBPath, QString &_lastDBName, QWidget *_parent) :
        BlastRunCommonDialog(_settingsList[0], _parent), settingsList(_settingsList), lastDBPath(_lastDBPath), lastDBName(_lastDBName)
{
    ca_c=NULL;
    wasNoOpenProject=false;
    //create input file widget
    QWidget * widget = new QWidget(_parent);
    inputFileLineEdit= new FileLineEdit("","", false, widget);
    inputFileLineEdit->setReadOnly(true);
    inputFileLineEdit->setText("");
    QToolButton * selectToolPathButton = new QToolButton(widget);
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

    databasePathLineEdit->setText(lastDBPath);
    baseNameLineEdit->setText(lastDBName);
    connect(cancelButton,SIGNAL(clicked()),SLOT(sl_cancel()));
    connect(this,SIGNAL(rejected()),SLOT(sl_cancel()));
}

void BlastPlusWithExtFileSpecifySupportRunDialog::sl_lineEditChanged(){
    bool isFilledDatabasePathLineEdit = !databasePathLineEdit->text().isEmpty();
    bool isFilledBaseNameLineEdit = !baseNameLineEdit->text().isEmpty();
    bool isInputFileLineEdit = !inputFileLineEdit->text().isEmpty();
    okButton->setEnabled(isFilledBaseNameLineEdit && isFilledDatabasePathLineEdit && isInputFileLineEdit);
}

void BlastPlusWithExtFileSpecifySupportRunDialog::sl_inputFileLineEditChanged(const QString& str){
    if(!str.isEmpty()){
        QFileInfo fi(str);
        if (fi.exists()){
            QList<Task *> tasks;
            Project* proj = AppContext::getProject();
            if (proj == NULL) {
                wasNoOpenProject=true;
                QList<GUrl> emptyList;
                tasks.append( AppContext::getProjectLoader()->openProjectTask(emptyList, false) );
            }

            DocumentFormatConstraints c;
            c.checkRawData = true;
            c.supportedObjectTypes += GObjectTypes::SEQUENCE;
            c.rawData = BaseIOAdapters::readFileHeader(str);
            QList<DocumentFormatId> formats = AppContext::getDocumentFormatRegistry()->selectFormats(c);
            if (formats.isEmpty()) {
                //stateInfo.setError(tr("input_format_error"));
                //show error message
                return;
            }
            DocumentFormatId df = formats.first();
            LoadDocumentTask* loadDocumentTask=
                    new LoadDocumentTask(df,
                                 str,
                                 AppContext::getIOAdapterRegistry()->getIOAdapterFactoryById(BaseIOAdapters::url2io(str)));
            connect(loadDocumentTask,SIGNAL(si_stateChanged()),SLOT(sl_inputFileLoadTaskStateChanged()));
            tasks.append(loadDocumentTask);
            AppContext::getTaskScheduler()->registerTopLevelTask(new MultiTask("Load documents and open project",tasks));
        }else{
            return;
        }
    }
}
void BlastPlusWithExtFileSpecifySupportRunDialog::sl_inputFileLoadTaskStateChanged(){
    LoadDocumentTask* s=qobject_cast<LoadDocumentTask*>(sender());
    if((s->isFinished())&&(!s->hasError())){
        int numOfSequences=0;
        foreach(GObject* gobj, s->getDocument()->getObjects()){
            if(gobj->getGObjectType()==GObjectTypes::SEQUENCE){
                numOfSequences++;
            }
        }
        settingsList.clear();
        sequencesRefList.clear();
        if(numOfSequences ==0){
            QMessageBox::critical(this, tr("Wrong input file"), tr("This file not contain any sequence."));
            inputFileLineEdit->setText("");
            return;
        }
        foreach(GObject* gobj, s->getDocument()->getObjects()){
            if(gobj->getGObjectType()==GObjectTypes::SEQUENCE){
                DNASequenceObject* seq=(DNASequenceObject*)gobj;
                BlastTaskSettings localSettings;
                localSettings.querySequence=seq->getDNASequence().seq;
                localSettings.alphabet=seq->getDNASequence().alphabet;
                if(localSettings.alphabet->getType() != DNAAlphabet_AMINO){
                    localSettings.isNucleotideSeq=true;
                }
                localSettings.queryFile=s->getURL().getURLString();
                settingsList.append(localSettings);
                sequencesRefList.append(GObjectReference(gobj));
            }
        }

        //DNASequenceObject* seq=(DNASequenceObject*)sequencesRefList[0];
        CreateAnnotationModel ca_m;
        ca_m.data->name = "misc_feature";
        ca_m.hideAnnotationName = true;
        ca_m.hideLocation = true;
        ca_m.sequenceObjectRef = sequencesRefList[0];//GObjectReference(seq);//not needed, it unused
        ca_m.sequenceLen = 10;//dnaso->getSequenceLen();
        if(ca_c != NULL){
            verticalLayout_4->removeWidget(ca_c->getWidget());
            delete ca_c;
        }
        ca_c = new CreateAnnotationWidgetController(ca_m, this);

        QWidget *wdgt = ca_c->getWidget();
        wdgt->setMinimumHeight(150);
        verticalLayout_4->addWidget(wdgt);
    }
    sl_lineEditChanged();
}

bool BlastPlusWithExtFileSpecifySupportRunDialog::checkToolPath(){

    bool needSetToolPath=false;
    QString toolName;
    QMessageBox msgBox;
    msgBox.setWindowTitle("BLAST+ Search");
    msgBox.setInformativeText(tr("Do you want to select it now?"));
    msgBox.setStandardButtons(QMessageBox::Yes | QMessageBox::No);
    msgBox.setDefaultButton(QMessageBox::Yes);
    if((programName->currentText() == "blastn") &&
       (AppContext::getExternalToolRegistry()->getByName(BLASTN_TOOL_NAME)->getPath().isEmpty())){
        needSetToolPath=true;
        toolName=BLASTN_TOOL_NAME;

    }else if((programName->currentText() == "blastp") &&
             (AppContext::getExternalToolRegistry()->getByName(BLASTP_TOOL_NAME)->getPath().isEmpty())){
        needSetToolPath=true;
        toolName=BLASTP_TOOL_NAME;

    }else if((programName->currentText() == "blastx") &&
             (AppContext::getExternalToolRegistry()->getByName(BLASTX_TOOL_NAME)->getPath().isEmpty())){
        needSetToolPath=true;
        toolName=BLASTX_TOOL_NAME;

    }else if((programName->currentText() == "tblastn") &&
             (AppContext::getExternalToolRegistry()->getByName(TBLASTN_TOOL_NAME)->getPath().isEmpty())){
        needSetToolPath=true;
        toolName=TBLASTN_TOOL_NAME;

    }else if((programName->currentText() == "tblastx") &&
             (AppContext::getExternalToolRegistry()->getByName(TBLASTX_TOOL_NAME)->getPath().isEmpty())){
        needSetToolPath=true;
        toolName=TBLASTX_TOOL_NAME;
    }
    if(needSetToolPath){
        msgBox.setText(tr("Path for <i>BLAST+ %1</i> tool not selected.").arg(toolName));
        int ret = msgBox.exec();
        switch (ret) {
           case QMessageBox::Yes:
               AppContext::getAppSettingsGUI()->showSettingsDialog(ExternalToolSupportSettingsPageId);
               break;
           case QMessageBox::No:
               return false;
               break;
           default:
               assert(NULL);
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
            settingsList[i].aobj = new AnnotationTableObject(sequencesRefList[i].objName+" annotations");
            settingsList[i].aobj->addObjectRelation(GObjectRelation(sequencesRefList[i], GObjectRelationRole::SEQUENCE));
        } else {
            assert(NULL);//allways created new document for annotations
//            ca_c->prepareAnnotationObject();
//            settings.aobj = caControllers.at(i)->getModel().getAnnotationObject();
        }
        settingsList[i].groupName=ca_c->getModel().groupName;

        settingsList[i].databaseNameAndPath=databasePathLineEdit->text()+"/"+baseNameLineEdit->text();
        settingsList[i].expectValue=evalueSpinBox->value();
        settingsList[i].wordSize=wordSizeSpinBox->value();
        settingsList[i].megablast=megablastCheckBox->isChecked();
    //    settings.alphabet=dnaso->getAlphabet();
        settingsList[i].numberOfHits=numberOfHitsSpinBox->value();
        settingsList[i].numberOfProcessors=numberOfCPUSpinBox->value();

        settingsList[i].gapOpenCost=costsComboBox->currentText().split(" ").at(0).toInt();
        settingsList[i].gapExtendCost=costsComboBox->currentText().split(" ").at(1).toInt();
        //setup filters
        if(lowComplexityFilterCheckBox->isChecked()){
            settingsList[i].filter="L";
        }
        if(repeatsCheckBox->isChecked()){
            settingsList[i].filter=settingsList[i].filter.isEmpty() ? "R" : settingsList[i].filter+"; R";
        }
    //    if(lowerCaseCheckBox->isChecked()){
    //        settings.filter=settings.filter.isEmpty() ? "???" : settings.filter+"; ???";
    //    }
        if(lookupMaskCheckBox->isChecked()){
            settingsList[i].filter=settingsList[i].filter.isEmpty() ? "m" : "m "+settingsList[i].filter;
        }

        if(settingsList[i].isNucleotideSeq){
            if((((scoresComboBox->currentText() == "1 -4") || (scoresComboBox->currentText() == "1 -3")) && costsComboBox->currentText()=="2 2") || //-G 2 -E 2
                ((scoresComboBox->currentText() == "1 -2") && costsComboBox->currentText()=="2 2") || //-G 2 -E 2
                ((scoresComboBox->currentText() == "1 -1") && costsComboBox->currentText()=="4 2") || //-G 4 -E 2
                (((scoresComboBox->currentText() == "2 -7") || (scoresComboBox->currentText() == "2 -5"))&& costsComboBox->currentText()=="4 4") || //-G 4 -E 4
                ((scoresComboBox->currentText() == "2 -3") && costsComboBox->currentText()=="6 4") || //-G 6 -E 4
                (((scoresComboBox->currentText() == "4 -5") || (scoresComboBox->currentText() == "5 -4")) && costsComboBox->currentText()=="12 8"))//-G 12 -E 8
            {
                settingsList[i].isDefaultCosts=true;
            }else{
                settingsList[i].isDefaultCosts=false;
                settingsList[i].matchReward=scoresComboBox->currentText().split(" ").at(0).toInt();
                settingsList[i].mismatchPenalty=scoresComboBox->currentText().split(" ").at(1).toInt();
            }
            settingsList[i].isDefautScores=(scoresComboBox->currentText() == "1 -3");
        }else{
            if(((matrixComboBox->currentText() == "PAM30") && costsComboBox->currentText()=="9 1") || //-G 9 -E 1
                    ((matrixComboBox->currentText() == "PAM70") && costsComboBox->currentText()=="10 1") || //-G 10 -E 1
                    ((matrixComboBox->currentText() == "BLOSUM45") && costsComboBox->currentText()=="15 2") ||
                    ((matrixComboBox->currentText() == "BLOSUM62") && costsComboBox->currentText()=="11 1") ||
                    ((matrixComboBox->currentText() == "BLOSUM80") && costsComboBox->currentText()=="10 1"))
            {
                settingsList[i].isDefaultCosts=true;
            }else{
                settingsList[i].isDefaultCosts=false;
                settingsList[i].matrix=matrixComboBox->currentText();
            }
            settingsList[i].isDefaultMatrix=(matrixComboBox->currentText() == "BLOSUM62");
        }
        settingsList[i].programName=programName->currentText();
    }
    bool docAlreadyInProject=false;
    Project* proj=AppContext::getProject();
    foreach(Document* doc, proj->getDocuments()){
        if(doc->getURL() == inputFileLineEdit->text()){
            docAlreadyInProject=true;
        }
    }
    if(!docAlreadyInProject){
        AppContext::getTaskScheduler()->registerTopLevelTask(AppContext::getProjectLoader()->openProjectTask(inputFileLineEdit->text(),false));
    }
    lastDBPath=databasePathLineEdit->text();
    lastDBName=baseNameLineEdit->text();
    settings.outputType=5;//By default set output file format to xml
    settings.isGappedAlignment=gappedAlignmentCheckBox->isChecked();
    accept();
}
void BlastPlusWithExtFileSpecifySupportRunDialog::sl_cancel(){
    if(qobject_cast<BlastPlusWithExtFileSpecifySupportRunDialog*>(sender()) == NULL){
        reject();
        return;
    }
    if(wasNoOpenProject){
        AppContext::getTaskScheduler()->registerTopLevelTask(AppContext::getProjectService()->closeProjectTask());
    }
}
}//namespace
