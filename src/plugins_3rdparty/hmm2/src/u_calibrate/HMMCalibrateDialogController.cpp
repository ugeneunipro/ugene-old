#include "HMMCalibrateDialogController.h"

#include "HMMCalibrateTask.h"
#include <HMMIO.h>

#include <U2Core/AppContext.h>
#include <U2Core/IOAdapter.h>
#include <U2Core/AppResources.h>

#include <U2Gui/LastUsedDirHelper.h>

#include <QtGui/QFileDialog>
#include <QtGui/QMessageBox>

namespace U2 {

HMMCalibrateDialogController::HMMCalibrateDialogController(QWidget* w) 
: QDialog(w)
{
    task = NULL;
    setupUi(this);

    connect(hmmFileButton, SIGNAL(clicked()), SLOT(sl_hmmFileButtonClicked()));
    connect(outFileButton, SIGNAL(clicked()), SLOT(sl_outFileButtonClicked()));
    connect(okButton, SIGNAL(clicked()), SLOT(sl_okButtonClicked()));
}


void HMMCalibrateDialogController::sl_hmmFileButtonClicked() {
    LastUsedDirHelper lod(HMMIO::HMM_ID);
    lod.url = QFileDialog::getOpenFileName(this, tr("select_file_with_hmm_model"), lod, HMMIO::getHMMFileFilter());
    if (lod.url.isEmpty()) {
        return;
    }
    hmmFileEdit->setText(QFileInfo(lod.url).absoluteFilePath());
}

void HMMCalibrateDialogController::sl_outFileButtonClicked() {
    LastUsedDirHelper lod(HMMIO::HMM_ID);
    lod.url= QFileDialog::getSaveFileName(this, tr("select_file_with_hmm_model"), lod, HMMIO::getHMMFileFilter());
    if (lod.url.isEmpty()) {
        return;
    }
    outFileEdit->setText(QFileInfo(lod.url).absoluteFilePath());
}

void HMMCalibrateDialogController::sl_okButtonClicked() {
    if (task != NULL) {
        accept(); //go to background
        return;
    }

    // try prepare model
   UHMMCalibrateSettings s;
   s.nThreads = AppResourcePool::instance()->getIdealThreadCount();
   QString errMsg;

   QString inFile = hmmFileEdit->text();
   QString outFile = inFile;
   if (inFile.isEmpty() && !QFileInfo(inFile).exists()) {
       errMsg = tr("illegal_in_file_name");
       hmmFileEdit->setFocus();
   }

   if (expertGroupBox->isChecked() && errMsg.isEmpty()) {
       if (fixedBox->value() < 0) {
           errMsg = tr("illegal fixed value");
           fixedBox->setFocus();
       } else {
           s.fixedlen = fixedBox->value();
       }

       s.lenmean = meanBox->value();
       s.nsample = numBox->value();
       s.lensd  = (float)sdBox->value();
       if (seedBox->value()!=0) {
           s.seed = seedBox->value();
       }
   }

   if (outputGroupBox->isChecked() && errMsg.isEmpty()) {
        outFile = outFileEdit->text();        
        if (outFile.isEmpty()) {
            errMsg = tr("illegal_out_file_name");
            outFileEdit->setFocus();
        }
   }

   if (!errMsg.isEmpty())  {
       QMessageBox::critical(this, tr("error"), errMsg);
       return;
   }

   // run task
   task = new HMMCalibrateToFileTask(inFile, outFile, s);
   task->setReportingEnabled(true);
   connect(task, SIGNAL(si_stateChanged()), SLOT(sl_onStateChanged()));
   connect(task, SIGNAL(si_progressChanged()), SLOT(sl_onProgressChanged()));
   AppContext::getTaskScheduler()->registerTopLevelTask(task);
   statusLabel->setText(tr("starting_calibration_process"));

   //update buttons
   okButton->setText(tr("back_button"));
   cancelButton->setText(tr("cancel_button"));

   // new default behavior: hide dialog and use taskview to track the progress and results
   accept(); //go to background
}


void HMMCalibrateDialogController::sl_onStateChanged() {
    Task* t = qobject_cast<Task*>(sender());
    assert(task!=NULL);
    if (task!=t || t->getState() != Task::State_Finished) {
        return;
    }
    task->disconnect(this);
    const TaskStateInfo& si = task->getStateInfo();
    if (si.hasError()) {
        statusLabel->setText(tr("calibration_finished_with_errors_%1").arg(si.getError()));
    } else if (task->isCanceled()) {
        statusLabel->setText(tr("calibration_canceled"));
    } else {
        statusLabel->setText(tr("calibration_finished_successfuly"));
    }
    okButton->setText(tr("ok_button"));
    cancelButton->setText(tr("close_button"));

    task = NULL;
}

void HMMCalibrateDialogController::sl_onProgressChanged() {
    assert(task==sender());
    statusLabel->setText(tr("progress_%1%").arg(task->getProgress()));
}

void HMMCalibrateDialogController::reject() {
    if (task!=NULL) {
        task->cancel();
    }
    QDialog::reject();
}



}//namespace
