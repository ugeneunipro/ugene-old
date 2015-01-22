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

#include <U2Core/AppContext.h>
#include <U2Core/AppSettings.h>
#include <U2Core/AppResources.h>
#include <U2Core/U2DbiRegistry.h>
#include <U2Core/DNAAlphabet.h>
#include <U2Core/DocumentUtils.h>
#include <U2Core/AnnotationTableObject.h>
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
#include <U2Core/U2OpStatusUtils.h>
#include <U2Gui/HelpButton.h>

#include <U2Gui/GUIUtils.h>
#include <U2Gui/OpenViewTask.h>

#if (QT_VERSION < 0x050000) //Qt 5
#include <QtGui/QToolButton>
#include <QtGui/QMessageBox>
#else
#include <QtWidgets/QToolButton>
#include <QtWidgets/QMessageBox>
#endif

#include "BlastAllSupportRunDialog.h"

namespace U2 {

////////////////////////////////////////
//BlastAllSupportRunDialog
BlastAllSupportRunDialog::BlastAllSupportRunDialog(U2SequenceObject *dnaso, QString &lastDBPath, QString &lastDBName, QWidget *parent)
: BlastRunCommonDialog(parent, BlastAll, false, QStringList()), dnaso(dnaso), lastDBPath(lastDBPath), lastDBName(lastDBName)
{
    CreateAnnotationModel ca_m;
    ca_m.data.name = "misc_feature";
    ca_m.hideAnnotationName = true;
    ca_m.hideLocation = true;
    ca_m.sequenceObjectRef = GObjectReference(dnaso);
    ca_m.sequenceLen = dnaso->getSequenceLength();
    ca_c = new CreateAnnotationWidgetController(ca_m, this);
    //lowerCaseCheckBox->hide();
    QWidget *wdgt = ca_c->getWidget();
    wdgt->setMinimumHeight(150);
    verticalLayout_4->addWidget(wdgt);

    okButton = buttonBox->button(QDialogButtonBox::Ok);
    cancelButton = buttonBox->button(QDialogButtonBox::Cancel);

    programName->removeItem(2);//gpu-blastp
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
    bool pathWarning = databasePathLineEdit->text().contains(' ');
    QString pathTooltip = pathWarning ? tr("Database path contains space characters.") : "";
    GUIUtils::setWidgetWarning(databasePathLineEdit, pathWarning);
    databasePathLineEdit->setToolTip(pathTooltip);

    bool nameWarning = baseNameLineEdit->text().contains(' ');
    QString nameTooltip = nameWarning ? tr("Database name contains space characters.") : "";
    GUIUtils::setWidgetWarning(baseNameLineEdit, nameWarning);
    baseNameLineEdit->setToolTip(nameTooltip);

    bool isFilledDatabasePathLineEdit = !databasePathLineEdit->text().isEmpty();
    bool isFilledBaseNameLineEdit = !baseNameLineEdit->text().isEmpty();
    bool hasSpacesInDBPath = pathWarning || nameWarning;
    okButton->setEnabled(isFilledBaseNameLineEdit && isFilledDatabasePathLineEdit && !hasSpacesInDBPath);
}

void BlastAllSupportRunDialog::sl_runQuery(){
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
BlastAllWithExtFileSpecifySupportRunDialog::BlastAllWithExtFileSpecifySupportRunDialog(QString &lastDBPath, QString &lastDBName, QWidget *parent)
: BlastRunCommonDialog(parent, BlastAll, false, QStringList()), lastDBPath(lastDBPath), lastDBName(lastDBName), hasValidInput(false)
{
    ca_c=NULL;
    wasNoOpenProject=false;
    //create input file widget
    QWidget *widget = new QWidget(parent);
    inputFileLineEdit= new FileLineEdit("","", false, widget);
    inputFileLineEdit->setReadOnly(true);
    inputFileLineEdit->setText("");
    QToolButton * selectToolPathButton = new QToolButton(widget);
    selectToolPathButton->setObjectName("browseInput");
    selectToolPathButton->setVisible(true);
    selectToolPathButton->setText("...");

    okButton = buttonBox->button(QDialogButtonBox::Ok);
    cancelButton = buttonBox->button(QDialogButtonBox::Cancel);

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

    programName->removeItem(2);//gpu-blastp

    databasePathLineEdit->setText(lastDBPath);
    baseNameLineEdit->setText(lastDBName);
    connect(cancelButton,SIGNAL(clicked()),SLOT(sl_cancel()));
    connect(this,SIGNAL(rejected()),SLOT(sl_cancel()));
}

const QList<BlastTaskSettings> &BlastAllWithExtFileSpecifySupportRunDialog::getSettingsList() const {
    return settingsList;
}

void BlastAllWithExtFileSpecifySupportRunDialog::sl_lineEditChanged(){
    bool pathWarning = databasePathLineEdit->text().contains(' ');
    QString pathTooltip = pathWarning ? tr("Database path contains space characters.") : "";
    GUIUtils::setWidgetWarning(databasePathLineEdit, pathWarning);
    databasePathLineEdit->setToolTip(pathTooltip);

    bool nameWarning = baseNameLineEdit->text().contains(' ');
    QString nameTooltip = nameWarning ? tr("Database name contains space characters.") : "";
    GUIUtils::setWidgetWarning(baseNameLineEdit, nameWarning);
    baseNameLineEdit->setToolTip(nameTooltip);

    bool isFilledDatabasePathLineEdit = !databasePathLineEdit->text().isEmpty();
    bool isFilledBaseNameLineEdit = !baseNameLineEdit->text().isEmpty();
    bool isInputFileLineEdit = !inputFileLineEdit->text().isEmpty();
    bool hasSpacesInDBPath = pathWarning || nameWarning;
    okButton->setEnabled(isFilledBaseNameLineEdit && isFilledDatabasePathLineEdit && isInputFileLineEdit && !hasSpacesInDBPath && hasValidInput);
}

void BlastAllWithExtFileSpecifySupportRunDialog::sl_inputFileLineEditChanged(const QString &url){
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

void BlastAllWithExtFileSpecifySupportRunDialog::onFormatError() {
    QMessageBox::critical(this, tr("Wrong input file"), tr("This file has the incompatible format for the BLAST+ search."));
    inputFileLineEdit->setText("");
}

void BlastAllWithExtFileSpecifySupportRunDialog::loadDoc(const QString &url) {
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

void BlastAllWithExtFileSpecifySupportRunDialog::sl_inputFileOpened(){
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
void BlastAllWithExtFileSpecifySupportRunDialog::tryApplyDoc(Document *doc) {
    int numOfSequences = 0;
    foreach (GObject *obj, doc->getObjects()) {
        if (obj->getGObjectType()==GObjectTypes::SEQUENCE) {
            numOfSequences++;
        }
    }
    settingsList.clear();
    sequencesRefList.clear();
    if (0 == numOfSequences) {
        QMessageBox::critical(this, tr("Wrong input file"), tr("This file does not contain any sequence."));
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
        localSettings.querySequence = seq->getWholeSequenceData();
        localSettings.alphabet = seq->getAlphabet();
        if (localSettings.alphabet->getType() != DNAAlphabet_AMINO) {
            localSettings.isNucleotideSeq = true;
        }
        localSettings.queryFile = doc->getURLString();
        settingsList.append(localSettings);
        sequencesRefList.append(GObjectReference(obj));
    }

    CreateAnnotationModel ca_m;
    ca_m.data.name = "misc_feature";
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
void BlastAllWithExtFileSpecifySupportRunDialog::sl_runQuery(){
    QString error = ca_c->validate();
    if (!error.isEmpty()) {
        QMessageBox::critical(this, tr("Wrong parameters for creating annotations"), error);
        return;
    }

    for(int i=0; i<settingsList.length();i++){
        settingsList[i].outputResFile=ca_c->getModel().newDocUrl;
        if(ca_c->isNewObject()) {
            U2OpStatusImpl os;
            const U2DbiRef dbiRef = AppContext::getDbiRegistry( )->getSessionTmpDbiRef( os );
            SAFE_POINT_OP( os, );
            settingsList[i].aobj = new AnnotationTableObject( sequencesRefList[i].objName+" annotations", dbiRef );
            settingsList[i].aobj->addObjectRelation(GObjectRelation(sequencesRefList[i], ObjectRole_Sequence));
        }
        else {
            assert(NULL);//allways created new document for annotations
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
        ProjectService *projService = AppContext::getProjectService();
        CHECK(NULL != projService, );
        AppContext::getTaskScheduler()->registerTopLevelTask(projService->closeProjectTask());
    }
}
}//namespace
