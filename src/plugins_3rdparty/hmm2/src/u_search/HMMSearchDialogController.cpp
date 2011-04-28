#include "HMMSearchDialogController.h"
#include "HMMSearchTask.h"
#include "TaskLocalStorage.h"

#include <HMMIO.h>
#include <hmmer2/funcs.h>

#include <U2Core/AppContext.h>
#include <U2Core/AppResources.h>
#include <U2Core/IOAdapter.h>

#include <U2Core/DNASequenceObject.h>
#include <U2Core/GObjectUtils.h>

#include <U2Gui/CreateAnnotationWidgetController.h>
#include <U2Misc/DialogUtils.h>
#include <U2Core/LoadDocumentTask.h>
#include <U2Core/CreateAnnotationTask.h>

#include <QtGui/QVBoxLayout>
#include <QtGui/QFileDialog>
#include <QtGui/QMessageBox>

namespace U2 {

HMMSearchDialogController::HMMSearchDialogController(const DNASequenceObject* seqObj, QWidget* p) 
: QDialog(p) , dnaSequence(seqObj->getDNASequence())
{
    searchTask = NULL;
    setupUi(this);

    CreateAnnotationModel cm;
    cm.hideLocation = true;
    cm.sequenceObjectRef = seqObj;
    cm.data->name = "hmm_signal";
    cm.sequenceLen = seqObj->getSequenceLen();
    createController = new CreateAnnotationWidgetController(cm, this);
    
    QWidget* w = createController->getWidget();
    QVBoxLayout* l = qobject_cast<QVBoxLayout*>(layout());
    l->insertWidget(1, w);
    setMinimumSize( layout()->totalSizeHint() );
#ifdef UGENE_CELL
    algoCombo->addItem( tr("Cell BE optimized"), HMMSearchAlgo_CellOptimized );
#endif
    if( AppResourcePool::isSSE2Enabled() ) {
        algoCombo->addItem( tr("SSE optimized"), HMMSearchAlgo_SSEOptimized );
    }
    algoCombo->addItem( tr("Conservative"), HMMSearchAlgo_Conservative );

    connect(hmmFileButton, SIGNAL(clicked()), SLOT(sl_hmmFileClicked()));
    connect(okButton, SIGNAL(clicked()), SLOT(sl_okClicked()));
	connect(domEvalueCuttofBox,SIGNAL(valueChanged(int)), SLOT(sl_expSpinBoxValueChanged(int)));
}

void HMMSearchDialogController::sl_expSpinBoxValueChanged(int i){
	if(i>=0){
		domEvalueCuttofBox->setPrefix("1E+");
	}
	else {
		domEvalueCuttofBox->setPrefix("1E");
	}
}

void HMMSearchDialogController::reject() {
    if (searchTask!=NULL) {
        searchTask->cancel();
    }
    QDialog::reject();
}

void HMMSearchDialogController::sl_hmmFileClicked() {
    LastOpenDirHelper lod(HMMIO::HMM_ID);
    lod.url = QFileDialog::getOpenFileName(this, tr("select_file_with_hmm_model"), lod, HMMIO::getHMMFileFilter());
    if (lod.url.isEmpty()) {
        return;
    }
    hmmFileEdit->setText(QFileInfo(lod.url).absoluteFilePath());
}

void HMMSearchDialogController::sl_okClicked() {
    if (searchTask!= NULL) {
        accept(); //go to background
        return;
    }

    QString errMsg;
    QString hmmFile = hmmFileEdit->text();
    if (hmmFile.isEmpty()) {
        hmmFileEdit->setFocus();
        errMsg = tr("hmm_file_not_set");
    }

    
    UHMMSearchSettings s;
    if (expertOptionsBox->isChecked() && errMsg.isEmpty()) {
		s.domE = pow(10,(float) domEvalueCuttofBox->value());
		s.domT = (float) minScoreBox->value();
        s.eValueNSeqs = evalueAsNSeqBox->value();
    }
    s.alg = HMMSearchAlgo( algoCombo->itemData( algoCombo->currentIndex() ).toInt() );
	
    if (errMsg.isEmpty()) {
        errMsg = createController->validate();
    }

    if (!errMsg.isEmpty())  {
        QMessageBox::critical(this, tr("error"), errMsg);
        return;
    }

    createController->prepareAnnotationObject();
    
    const CreateAnnotationModel& cm = createController->getModel();
    QString annotationName = cm.data->name;
    searchTask = new HMMSearchToAnnotationsTask(hmmFile, dnaSequence, cm.getAnnotationObject(), cm.groupName, annotationName, s);
    searchTask->setReportingEnabled(true);
    connect(searchTask, SIGNAL(si_stateChanged()), SLOT(sl_onStateChanged()));
    connect(searchTask, SIGNAL(si_progressChanged()), SLOT(sl_onProgressChanged()));
    AppContext::getTaskScheduler()->registerTopLevelTask(searchTask);
    statusLabel->setText(tr("starting_search_process"));

    //update buttons
    okButton->setText(tr("back_button"));
    cancelButton->setText(tr("cancel_button"));

    // new default behavior: hide dialog and use taskview to track the progress and results
    accept(); //go to background
}

void HMMSearchDialogController::sl_onStateChanged() {
    Task* t = qobject_cast<Task*>(sender());
    assert(searchTask != NULL);
    if (searchTask!=t || t->getState() != Task::State_Finished) {
        return;
    }
    searchTask->disconnect(this);
    const TaskStateInfo& si = searchTask->getStateInfo();
    if (si.hasError()) {
        statusLabel->setText(tr("search_finished_with_errors_%1").arg(si.getError()));
    } else {
        statusLabel->setText(tr("search_finished_successfuly"));
    }
    okButton->setText(tr("ok_button"));
    cancelButton->setText(tr("close_button"));

    searchTask = NULL;
}

void HMMSearchDialogController::sl_onProgressChanged() {
    assert(searchTask == sender());
    statusLabel->setText(tr("progress_%1%").arg(qMax(0, searchTask->getProgress())));
}

    
//////////////////////////////////////////////////////////////////////////
// TASKS

HMMSearchToAnnotationsTask::HMMSearchToAnnotationsTask(const QString& _hmmFile, const DNASequence& s, 
                                                       AnnotationTableObject* ao, const QString& _agroup, const QString& _aname,
                                                       const UHMMSearchSettings& _settings)
: Task("", TaskFlags_NR_FOSCOE | TaskFlag_ReportingIsSupported), 
hmmFile(_hmmFile), dnaSequence(s), agroup(_agroup), aname(_aname), settings(_settings), 
readHMMTask(NULL), searchTask(NULL), createAnnotationsTask(NULL), aobj(ao)
{
    setVerboseLogMode(true);
    setTaskName(tr("HMM search, file '%1'").arg(QFileInfo(hmmFile).fileName()));
    
    readHMMTask = new HMMReadTask(hmmFile);
    readHMMTask->setSubtaskProgressWeight(0);
    
    if (dnaSequence.alphabet->isRaw()) {
        stateInfo.setError(  tr("raw_alphabet_not_supported") );
    } else {
        addSubTask(readHMMTask);
    }
}

QList<Task*> HMMSearchToAnnotationsTask::onSubTaskFinished(Task* subTask) {
    Q_UNUSED(subTask);

    QList<Task*> res;

    if (hasError() || isCanceled()) {
        return res;
    }
    
    if (aobj.isNull()) {
        stateInfo.setError(  tr("annotation_obj_removed") );
        return res;
    }

    if (searchTask == NULL) {
        assert(readHMMTask->isFinished());
        plan7_s* hmm = readHMMTask->getHMM();
        assert(hmm!=NULL);

        searchTask = new HMMSearchTask(hmm, dnaSequence, settings);
        res.append(searchTask);
    } else if (createAnnotationsTask == NULL){
        assert(searchTask->isFinished() && !searchTask->hasError());
        QList<SharedAnnotationData> annotations = searchTask->getResultsAsAnnotations(aname);
        if (!annotations.isEmpty()) {
            createAnnotationsTask = new CreateAnnotationsTask(aobj, agroup, annotations);
            createAnnotationsTask->setSubtaskProgressWeight(0);
            res.append(createAnnotationsTask);
        }
    }
    return res;
}


QString HMMSearchToAnnotationsTask::generateReport() const {
    QString res;
    res+="<table>";
    res+="<tr><td width=200><b>" + tr("HMM profile used") + "</b></td><td>" + QFileInfo(hmmFile).absoluteFilePath() + "</td></tr>";

    if (hasError() || isCanceled()) {
        res+="<tr><td width=200><b>" + tr("Task was not finished") + "</b></td><td></td></tr>";
        res+="</table>";
        return res;
    }
    
    res+="<tr><td><b>" + tr("Result annotation table") + "</b></td><td>" + aobj->getDocument()->getName() + "</td></tr>";
    res+="<tr><td><b>" + tr("Result annotation group") + "</b></td><td>" + agroup + "</td></tr>";
    res+="<tr><td><b>" + tr("Result annotation name") +  "</b></td><td>" + aname + "</td></tr>";

    int nResults = createAnnotationsTask == NULL ? 0 : createAnnotationsTask->getAnnotations().size();
    res+="<tr><td><b>" + tr("Results count") +  "</b></td><td>" + QString::number(nResults)+ "</td></tr>";
    res+="</table>";
    return res;
}


}//namespace
