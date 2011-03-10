#include "HMMBuildDialogController.h"
#include "TaskLocalStorage.h"

#include <HMMIO.h>
#include <hmmer2/funcs.h>

#include <U2Core/BaseDocumentFormats.h>
#include <U2Core/AppContext.h>
#include <U2Core/IOAdapter.h>
#include <U2Core/DocumentModel.h>
#include <U2Core/DNAAlphabet.h>
#include <U2Core/Counter.h>
#include <U2Core/MAlignmentObject.h>
#include <U2Misc/DialogUtils.h>
#include <U2Core/LoadDocumentTask.h>

#include <QtGui/QMessageBox>
#include <QtGui/QFileDialog>

namespace U2 {
HMMBuildDialogController::HMMBuildDialogController(const QString& _pn, const MAlignment& _ma, QWidget* p) 
:QDialog(p), ma(_ma), profileName(_pn)
{
    setupUi(this);

    if (!ma.isEmpty()) {
        msaFileButton->setHidden(true);
        msaFileEdit->setHidden(true);
        msaFileLabel->setHidden(true);
    }
    connect(msaFileButton, SIGNAL(clicked()), SLOT(sl_msaFileClicked()));
    connect(resultFileButton, SIGNAL(clicked()), SLOT(sl_resultFileClicked()));
    connect(okButton, SIGNAL(clicked()), SLOT(sl_okClicked()));
    
    task = NULL;
}

void HMMBuildDialogController::sl_msaFileClicked() {
    LastOpenDirHelper lod;
    lod.url = QFileDialog::getOpenFileName(this, tr("select_file_with_alignment"), 
        lod, DialogUtils::prepareDocumentsFileFilterByObjType(GObjectTypes::MULTIPLE_ALIGNMENT, true));
    
    if (lod.url.isEmpty()) {
        return;
    }

    msaFileEdit->setText(QFileInfo(lod.url).absoluteFilePath());
}

void HMMBuildDialogController::sl_resultFileClicked() {
    LastOpenDirHelper lod(HMMIO::HMM_ID);
    lod.url = QFileDialog::getSaveFileName(this, tr("Select file with HMM profile"), lod, HMMIO::getHMMFileFilter());
    if (lod.url.isEmpty()) {
        return;
    }
    resultFileEdit->setText(QFileInfo(lod.url).absoluteFilePath());
}

void HMMBuildDialogController::sl_okClicked() {
    if (task != NULL) {
        accept(); //go to background
        return;
    }

    //prepare model
    UHMMBuildSettings s;
    s.name = profileName;
    QString errMsg;

    QString inFile = msaFileEdit->text();
    if (ma.isEmpty() && (inFile.isEmpty() || !QFileInfo(inFile).exists())) {
        errMsg = tr("incorrect_ali_file");
        msaFileEdit->setFocus();
    }
    QString outFile = resultFileEdit->text();
    if (outFile.isEmpty() && errMsg.isEmpty()) {
        errMsg = tr("incorrect_hmm_file");
        resultFileEdit->setFocus();
    }
    if (expertGroup->isChecked() && errMsg.isEmpty()) {
        s.name = nameEdit->text().trimmed();
        s.strategy = P7_LS_CONFIG;
        if (hmmfsButton->isChecked()) {
            s.strategy = P7_FS_CONFIG;
        } else if (hmmsButton->isChecked()) {
            s.strategy = P7_BASE_CONFIG;
        } else if (hmmswButton->isChecked()) {
            s.strategy = P7_SW_CONFIG;
        }
    }
    
    if (!errMsg.isEmpty()) {
        QMessageBox::critical(this, tr("error"), errMsg);
        return;
    }

    task = ma.isEmpty() ? new HMMBuildToFileTask(inFile, outFile, s) : new HMMBuildToFileTask(ma, outFile, s);
    task->setReportingEnabled(true);
    connect(task, SIGNAL(si_stateChanged()), SLOT(sl_onStateChanged()));
    connect(task, SIGNAL(si_progressChanged()), SLOT(sl_onProgressChanged()));
    AppContext::getTaskScheduler()->registerTopLevelTask(task);
    statusLabel->setText(tr("starting_build_process"));

    //update buttons
    okButton->setText(tr("back_button"));
    cancelButton->setText(tr("cancel_button"));

    // new default behavior: hide dialog and use taskview to track the progress and results
    accept(); //go to background
}


void HMMBuildDialogController::reject() {
    if (task!=NULL) {
        task->cancel();
    }
    QDialog::reject();
}

void HMMBuildDialogController::sl_onStateChanged() {
    Task* t = qobject_cast<Task*>(sender());
    assert(task!=NULL);
    if (task!=t || t->getState() != Task::State_Finished) {
        return;
    }
    task->disconnect(this);
    const TaskStateInfo& si = task->getStateInfo();
    if (si.hasErrors()) {
        statusLabel->setText(tr("build_finished_with_errors_%1").arg(si.getError()));
    } else if (task->isCanceled()) {
        statusLabel->setText(tr("build_canceled"));
    } else {
        statusLabel->setText(tr("build_finished_successfuly"));
    }
    okButton->setText(tr("ok_button"));
    cancelButton->setText(tr("close_button"));

    AppContext::getTaskScheduler()->disconnect(this);
    task = NULL;

}

void HMMBuildDialogController::sl_onProgressChanged() {
    assert(task==sender());
    statusLabel->setText(tr("progress_%1%").arg(task->getProgress()));
}



//////////////////////////////////////////////////////////////////////////
//  HMMBuildTask

HMMBuildToFileTask::HMMBuildToFileTask(const QString& inFile, const QString& _outFile, const UHMMBuildSettings& s) 
: Task("", TaskFlag_ReportingIsSupported), settings(s), outFile(_outFile), loadTask(NULL), buildTask(NULL)
{
    setTaskName(tr("Build HMM profile '%1' -> '%2'").arg(QFileInfo(inFile).fileName()).arg(QFileInfo(outFile).fileName()));
    setVerboseLogMode(true);

    assert(!(inFile.isEmpty() || outFile.isEmpty()));
    //todo: support all formats with alignments!

    DocumentFormatConstraints c;
    c.checkRawData = true;
    c.supportedObjectTypes += GObjectTypes::MULTIPLE_ALIGNMENT;
    c.rawData = BaseIOAdapters::readFileHeader(inFile);
    QList<DocumentFormatId> formats = AppContext::getDocumentFormatRegistry()->selectFormats(c);
    if (formats.isEmpty()) {
        stateInfo.setError(  tr("input_format_error") );
        return;
    }

    DocumentFormatId alnFormat = formats.first();

    
    loadTask = new LoadDocumentTask(alnFormat, inFile, 
                                    AppContext::getIOAdapterRegistry()->getIOAdapterFactoryById(BaseIOAdapters::url2io(inFile)),
                                    QVariantMap());
    addSubTask(loadTask);
}

HMMBuildToFileTask::HMMBuildToFileTask(const MAlignment& _ma, const QString& _outFile, const UHMMBuildSettings& s) 
: Task("", TaskFlags_FOSCOE | TaskFlag_ReportingIsSupported),
settings(s), outFile(_outFile), ma(_ma), loadTask(NULL), buildTask(NULL)
{
    setTaskName(tr("Build HMM profile to '%1'").arg(QFileInfo(outFile).fileName()));
    setVerboseLogMode(true);

    assert(!(outFile.isEmpty() || ma.isEmpty()));
    if (settings.name.isEmpty()) {
        QFileInfo fi(outFile);
        settings.name = fi.baseName(); //FIXME temporary workaround
    }
    buildTask = new HMMBuildTask(settings, ma);
    addSubTask(buildTask);
}


QList<Task*> HMMBuildToFileTask::onSubTaskFinished(Task* subTask) {
    QList<Task*> res;
    
    if (hasErrors() || isCanceled()) {
        return res;
    }

    if (subTask == loadTask) {
        assert(ma.isEmpty());
        Document*  doc = loadTask->getDocument();
        //assert(doc);
        if(!doc) {
            stateInfo.setError(tr("Incorrect input file"));
            return res;
        }
        QList<GObject*> list = doc->findGObjectByType(GObjectTypes::MULTIPLE_ALIGNMENT);
        if (list.isEmpty()) {
            stateInfo.setError(  tr("alignment_object_not_found") );
        } else {
            MAlignmentObject* msa = qobject_cast<MAlignmentObject*>(list.first());
            const MAlignment& ma = msa->getMAlignment();
            if (settings.name.isEmpty()) {
                settings.name = msa->getGObjectName() == MA_OBJECT_NAME ? doc->getName() : msa->getGObjectName();
            }
            buildTask = new HMMBuildTask(settings, ma);
            res.append(buildTask);
        }
    }
    return res;
}

void HMMBuildToFileTask::run() {
    TaskLocalData::createHMMContext(getTaskId(), true);
    _run();
    TaskLocalData::freeHMMContext(getTaskId());
}

void HMMBuildToFileTask::_run() {
    if (stateInfo.hasErrors()) {
        return;
    }
    assert(buildTask!=NULL);
    assert(buildTask->getState() == Task::State_Finished);
    if (buildTask->getStateInfo().hasErrors()) {
        stateInfo.setError(  buildTask->getStateInfo().getError() );
        return;
    }
    plan7_s* hmm = buildTask->getHMM();
    assert(hmm!=NULL);
	IOAdapterFactory* iof;
	iof = AppContext::getIOAdapterRegistry()->getIOAdapterFactoryById(BaseIOAdapters::url2io(outFile));
	
    //todo: write command line too!
    HMMIO::writeHMM2(iof, outFile, stateInfo, hmm);
}

QString HMMBuildToFileTask::generateReport() const {
    QString res;
    res+="<table>";
    res+="<tr><td width=200><b>" + tr("Source alignment") + "</b></td><td>" + (loadTask ==  NULL ? settings.name : loadTask->getURL().getURLString()) + "</td></tr>";
    res+="<tr><td><b>" + tr("Profile name") + "</b></td><td>" + settings.name + "</td></tr>";
    if (hasErrors()) {
        res+="<tr><td width=200><b>" + tr("Task was not finished") + "</b></td><td></td></tr>";
        res+="</table>";
        return res;
    }

    res+="<tr><td><b>" + tr("Profile file") + "</b></td><td>" + outFile + "</td></tr>";
    res+="<tr><td><b>" + tr("Expert options") + "</b></td><td>";
    switch(settings.strategy) {
        case P7_BASE_CONFIG:  res+= "-g"; break;
        case P7_LS_CONFIG:    res+= tr("none"); break;
        case P7_FS_CONFIG:    res+= "-f"; break;
        case P7_SW_CONFIG:    res+= "-s"; break;
    }
    res+="</td></tr>";

    res+="</table>";
    return res;
}


HMMBuildTask::HMMBuildTask(const UHMMBuildSettings& s, const MAlignment& _ma) 
  : Task("", TaskFlag_None), ma(_ma), settings(s), hmm(NULL)
{
    GCOUNTER( cvar, tvar, "HMMBuildTask" );
    setTaskName(tr("Build HMM profile '%1'").arg(s.name));
}

HMMBuildTask::~HMMBuildTask() {
    if (hmm!=NULL) {
        FreePlan7(hmm);
    }
}

void HMMBuildTask::run() {
    TaskLocalData::createHMMContext(getTaskId(), true);
    _run();
    TaskLocalData::freeHMMContext(getTaskId());
}

void HMMBuildTask::_run() {
    if (ma.getNumRows() == 0) {
        stateInfo.setError(  tr("multiple_alignment_is_empty") );
        return;
    }
    if (ma.getLength() == 0) {
        stateInfo.setError(  tr("multiple_alignment_is_0_len") );
        return;
    }
    //todo: check HMM for RAW alphabet!
    if (ma.getAlphabet()->isRaw()) {
        stateInfo.setError(  tr("only_amino_and_nucl_alphabets_are_supported") );
        return;
    }

    //everything ok here: fill msa

    msa_struct* msa = MSAAlloc(ma.getNumRows(), ma.getLength());
    if (msa == NULL) {
        stateInfo.setError(  tr("error_creating_msa") );
        return;
    }
    for (int i=0; i<ma.getNumRows();i++) {
		const MAlignmentRow& row = ma.getRow(i);
        QByteArray seq = row.toByteArray(ma.getLength());
		free(msa->aseq[i]);
        msa->aseq[i] = sre_strdup(seq.constData(), seq.size());
        QByteArray name = row.getName().toAscii();
        msa->sqname[i] = sre_strdup(name.constData(), name.size());
        msa->wgt[i] = 1.0; // default weights 
    }
    
    int atype = ma.getAlphabet()->isNucleic() ? hmmNUCLEIC :hmmAMINO;
    try {
        hmm = UHMMBuild::build(msa, atype, settings, stateInfo);
    } catch (HMMException e) {
        stateInfo.setError(  e.error );
    }
    
    assert(hmm!=NULL || stateInfo.hasErrors());
    
    MSAFree(msa);
}

}//namespace
