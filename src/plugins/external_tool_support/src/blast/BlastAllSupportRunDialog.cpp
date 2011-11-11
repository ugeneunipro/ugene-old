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

#include "BlastAllSupportRunDialog.h"
#include <U2Core/AppContext.h>
#include <U2Core/AppSettings.h>
#include <U2Core/AppResources.h>
#include <U2Core/DNAAlphabet.h>
#include <U2Core/GObjectReference.h>
#include <U2Core/IOAdapter.h>
#include <U2Core/IOAdapterUtils.h>
#include <U2Core/ProjectModel.h>
#include <U2Core/GObjectRelationRoles.h>
#include <U2Core/DNASequenceObject.h>
#include <U2Gui/DialogUtils.h>
#include <U2Gui/CreateAnnotationWidgetController.h>
#include <U2Core/LoadDocumentTask.h>
#include <U2Core/MultiTask.h>
#include <U2Core/ProjectService.h>

#include <QtGui/QFileDialog>
#include <QtGui/QToolButton>
#include <QtGui/QMessageBox>

namespace U2 {

////////////////////////////////////////
//BlastAllSupportRunDialog
BlastAllSupportRunDialog::BlastAllSupportRunDialog(U2SequenceObject* _dnaso, QString &_lastDBPath, QString &_lastDBName, QWidget* _parent) :
        BlastRunCommonDialog(_parent), dnaso(_dnaso), lastDBPath(_lastDBPath), lastDBName(_lastDBName)
{
    CreateAnnotationModel ca_m;
    ca_m.data->name = "misc_feature";
    ca_m.hideAnnotationName = true;
    ca_m.hideLocation = true;
    ca_m.sequenceObjectRef = GObjectReference(dnaso);
    ca_m.sequenceLen = dnaso->getSequenceLength();
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

void BlastAllSupportRunDialog::sl_lineEditChanged(){
    bool isFilledDatabasePathLineEdit = !databasePathLineEdit->text().isEmpty();
    bool isFilledBaseNameLineEdit = !baseNameLineEdit->text().isEmpty();
    okButton->setEnabled(isFilledBaseNameLineEdit && isFilledDatabasePathLineEdit);
}

void BlastAllSupportRunDialog::sl_runQuery(){
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
    settings.alphabet=dnaso->getAlphabet();
    getSettings(settings);
    lastDBPath=databasePathLineEdit->text();
    lastDBName=baseNameLineEdit->text();
    settings.outputType=7;//By default set output file format to xml
    accept();
}
////////////////////////////////////////
//BlastAllWithExtFileSpecifySupportRunDialog
BlastAllWithExtFileSpecifySupportRunDialog::BlastAllWithExtFileSpecifySupportRunDialog(QString &_lastDBPath, QString &_lastDBName,QWidget* _parent) :
        BlastRunCommonDialog(_parent), lastDBPath(_lastDBPath), lastDBName(_lastDBName)
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

const QList<BlastTaskSettings> &BlastAllWithExtFileSpecifySupportRunDialog::getSettingsList() const {
    return settingsList;
}

void BlastAllWithExtFileSpecifySupportRunDialog::sl_lineEditChanged(){
    bool isFilledDatabasePathLineEdit = !databasePathLineEdit->text().isEmpty();
    bool isFilledBaseNameLineEdit = !baseNameLineEdit->text().isEmpty();
    bool isInputFileLineEdit = !inputFileLineEdit->text().isEmpty();
    okButton->setEnabled(isFilledBaseNameLineEdit && isFilledDatabasePathLineEdit && isInputFileLineEdit);
}

void BlastAllWithExtFileSpecifySupportRunDialog::sl_inputFileLineEditChanged(const QString& str){
    if(!str.isEmpty()){
        QFileInfo fi(str);
        if (fi.exists()){
            QList<Task *> tasks;
            Project* proj = AppContext::getProject();
            if (proj == NULL) {
                wasNoOpenProject=true;
                tasks.append( AppContext::getProjectLoader()->createNewProjectTask() );
            }

            DocumentFormatConstraints c;
            c.checkRawData = true;
            c.supportedObjectTypes += GObjectTypes::SEQUENCE;
            c.rawData = IOAdapterUtils::readFileHeader(str);
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
                                 AppContext::getIOAdapterRegistry()->getIOAdapterFactoryById(IOAdapterUtils::url2io(str)));
            connect(loadDocumentTask,SIGNAL(si_stateChanged()),SLOT(sl_inputFileLoadTaskStateChanged()));
            tasks.append(loadDocumentTask);
            AppContext::getTaskScheduler()->registerTopLevelTask(new MultiTask("Load documents and open project",tasks));
        }else{
            return;
        }
    }
}
void BlastAllWithExtFileSpecifySupportRunDialog::sl_inputFileLoadTaskStateChanged(){
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
                U2SequenceObject* seq=(U2SequenceObject*)gobj;
                BlastTaskSettings localSettings;
                localSettings.querySequence = seq->getWholeSequenceData();
                localSettings.alphabet = seq->getAlphabet();
                if(localSettings.alphabet->getType() != DNAAlphabet_AMINO){
                    localSettings.isNucleotideSeq=true;
                }
                localSettings.queryFile=s->getURL().getURLString();
                settingsList.append(localSettings);
                sequencesRefList.append(GObjectReference(gobj));
            }
        }

        //U2SequenceObject* seq=(U2SequenceObject*)sequencesRefList[0];
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
void BlastAllWithExtFileSpecifySupportRunDialog::sl_runQuery(){
    QString error = ca_c->validate();
    if (!error.isEmpty()) {
        QMessageBox::critical(this, tr("Wrong parameters for creating annotations"), error);
        return;
    }

    for(int i=0; i<settingsList.length();i++){
        settingsList[i].outputResFile=ca_c->getModel().newDocUrl;
        if(ca_c->isNewObject()) {
            settingsList[i].aobj = new AnnotationTableObject(sequencesRefList[i].objName+" annotations");
            settingsList[i].aobj->addObjectRelation(GObjectRelation(sequencesRefList[i], GObjectRelationRole::SEQUENCE));
        }
        else {
            assert(NULL);//allways created new document for annotations
//            ca_c->prepareAnnotationObject();
//            settings.aobj = caControllers.at(i)->getModel().getAnnotationObject();
        }
        settingsList[i].groupName=ca_c->getModel().groupName;

        getSettings(settingsList[i]);

        //setup filters
        settingsList[i].outputType=7;//By default set output file format to xml
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
    lastDBPath=databasePathLineEdit->text();
    lastDBName=baseNameLineEdit->text();
    accept();
}
void BlastAllWithExtFileSpecifySupportRunDialog::sl_cancel(){
    if(qobject_cast<BlastAllWithExtFileSpecifySupportRunDialog*>(sender()) == NULL){
        reject();
        return;
    }
    if(wasNoOpenProject){
        AppContext::getTaskScheduler()->registerTopLevelTask(AppContext::getProjectService()->closeProjectTask());
    }
}
}//namespace
