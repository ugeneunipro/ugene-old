/**
 * UGENE - Integrated Bioinformatics Tools.
 * Copyright (C) 2008-2014 UniPro <ugene@unipro.ru>
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

#include "SiteconBuildDialogController.h"

#include "SiteconIO.h"
#include "SiteconPlugin.h"


#include <U2Core/AppContext.h>
#include <U2Core/IOAdapter.h>
#include <U2Core/IOAdapterUtils.h>
#include <U2Core/DocumentModel.h>
#include <U2Core/DNAAlphabet.h>
#include <U2Core/Settings.h>
#include <U2Core/Counter.h>
#include <U2Core/LoadDocumentTask.h>
#include <U2Core/GObjectTypes.h>
#include <U2Core/MAlignmentObject.h>

#include <U2Gui/LastUsedDirHelper.h>
#include <U2Gui/DialogUtils.h>
#include <U2Gui/HelpButton.h>

#include <QtGui/QFileDialog>
#include <QtGui/QMessageBox>
#include <QtGui/QPushButton>


#define SETTINGS_ROOT   QString("plugin_sitecon/")
#define CALIBRATION_LEN "calibration_len"
#define WEIGHT_ALG      "weight_alg"

namespace U2 {

SiteconBuildDialogController::SiteconBuildDialogController(SiteconPlugin* pl, QWidget* w) 
: QDialog(w), plug(pl)
{
    task = NULL;
    setupUi(this);
    new HelpButton(this, buttonBox, "4227671");
    buttonBox->button(QDialogButtonBox::Ok)->setText(tr("Build"));
    buttonBox->button(QDialogButtonBox::Cancel)->setText(tr("Cancel"));


    weightAlgCombo->setCurrentIndex(AppContext::getSettings()->getValue(SETTINGS_ROOT + WEIGHT_ALG, 1).toInt());
    calibrationSeqLenBox->setCurrentIndex(AppContext::getSettings()->getValue(SETTINGS_ROOT + CALIBRATION_LEN).toInt());

    okButton = buttonBox->button(QDialogButtonBox::Ok);
    cancelButton = buttonBox->button(QDialogButtonBox::Cancel);
    connect(inputButton, SIGNAL(clicked()), SLOT(sl_inFileButtonClicked()));
    connect(outputButton, SIGNAL(clicked()), SLOT(sl_outFileButtonClicked()));
    connect(okButton, SIGNAL(clicked()), SLOT(sl_okButtonClicked()));

}


void SiteconBuildDialogController::sl_inFileButtonClicked() {
    LastUsedDirHelper lod;
    lod.url = QFileDialog::getOpenFileName(this, tr("select_file_with_alignment"), lod, 
                                                DialogUtils::prepareDocumentsFileFilterByObjType(GObjectTypes::MULTIPLE_ALIGNMENT, true));
    if (lod.url.isEmpty()) {
        return;
    }
    inputEdit->setText(QFileInfo(lod.url).absoluteFilePath());
}

void SiteconBuildDialogController::sl_outFileButtonClicked() {
    LastUsedDirHelper lod(SiteconIO::SITECON_ID);
    lod.url = QFileDialog::getSaveFileName(this, tr("Select file to save model to..."), lod, SiteconIO::getFileFilter(false));
    if (lod.url.isEmpty()) {
        return;
    }
    outputEdit->setText(QFileInfo(lod.url).absoluteFilePath());
}

void SiteconBuildDialogController::sl_okButtonClicked() {
    if (task != NULL) {
        accept(); //go to background
        return;
    }

    // try prepare model
   SiteconBuildSettings s;
   s.props = plug->getDinucleotiteProperties();
   s.randomSeed = seedSpin->value();
   int idx = qBound(0, calibrationSeqLenBox->currentIndex(), 3);
   static int lenVals[] = {100*1000, 500*1000, 1000*1000, 5*1000*1000};
   s.secondTypeErrorCalibrationLen = lenVals[idx];
   s.weightAlg = weightAlgCombo->currentIndex() == 0 ? SiteconWeightAlg_None : SiteconWeightAlg_Alg2;

   QString errMsg;

   QString inFile = inputEdit->text();
   if (inFile.isEmpty() && !QFileInfo(inFile).exists()) {
       errMsg = tr("illegal_in_file_name");
       inputEdit->setFocus();
   }
   QString outFile = outputEdit->text();
   if (outFile.isEmpty()) {
       errMsg = tr("illegal_out_file_name");
       outputEdit->setFocus();
   }
   
   s.windowSize = windowSizeSpin->value();
   if (!errMsg.isEmpty())  {
       QMessageBox::critical(this, tr("error"), errMsg);
       return;
   }
   //save settings
   AppContext::getSettings()->setValue(SETTINGS_ROOT + CALIBRATION_LEN, idx);
   AppContext::getSettings()->setValue(SETTINGS_ROOT + WEIGHT_ALG, weightAlgCombo->currentIndex());

   // run task
   task = new SiteconBuildToFileTask(inFile, outFile, s);
   connect(task, SIGNAL(si_stateChanged()), SLOT(sl_onStateChanged()));
   connect(task, SIGNAL(si_progressChanged()), SLOT(sl_onProgressChanged()));
   AppContext::getTaskScheduler()->registerTopLevelTask(task);
   statusLabel->setText(tr("starting_calibration_process"));

   //update buttons
   okButton->setText(tr("hide_button"));
   cancelButton->setText(tr("cancel_button"));
}


void SiteconBuildDialogController::sl_onStateChanged() {
    Task* t = qobject_cast<Task*>(sender());
    assert(task!=NULL);
    if (task != t || t->getState() != Task::State_Finished) {
        return;
    }
    task->disconnect(this);
    const TaskStateInfo& si = task->getStateInfo();
    if (si.hasError()) {
        statusLabel->setText(tr("build_finished_with_errors_%1").arg(si.getError()));
    } else if (task->isCanceled()) {
        statusLabel->setText(tr("build_canceled"));
    } else {
        statusLabel->setText(tr("build_finished_successfuly"));
    }
    okButton->setText(tr("start_button"));
    cancelButton->setText(tr("close_button"));
    task = NULL;
}

void SiteconBuildDialogController::sl_onProgressChanged() {
    assert(task==sender());
    statusLabel->setText(tr("running_state_%1_progress_%2%").arg(task->getStateInfo().getDescription()).arg(task->getProgress()));
}

void SiteconBuildDialogController::reject() {
    if (task!=NULL) {
        task->cancel();
    }
    QDialog::reject();
}


//////////////////////////////////////////////////////////////////////////
// task

SiteconBuildTask::SiteconBuildTask(const SiteconBuildSettings& s, const MAlignment& ma, const QString& origin) 
: Task (tr("build_sitecon_model"), TaskFlag_None), settings(s), ma(ma)
{
    GCOUNTER( cvar, tvar, "SiteconBuildTask" );
    tpm = Task::Progress_Manual;
    m.aliURL = origin;
}

void SiteconBuildTask::run() {
    // compute average/dispersion matrix
    if (!ma.hasEmptyGapModel()) {
        stateInfo.setError( tr("alignment_has_gaps") );
        return;
    }
    if (ma.isEmpty()) {
        stateInfo.setError(  tr("alignment_is_empty") );
        return;
    }
    if (ma.getNumRows() < 2) {
        stateInfo.setError(  tr("Alignment must have at least 2 sequences") );
        return;
    }
    if (!ma.getAlphabet()->isNucleic()) {
        stateInfo.setError(  tr("alignment_is_not_nucleic") );
        return;
    }
    if (ma.getLength() < settings.windowSize) {
        stateInfo.setError(  tr("window_greater_then_length") );
        return;
    }
    
    int centerPos = ma.getLength() / 2;
    int startPos = centerPos - settings.windowSize / 2;
    int endPos = centerPos + (settings.windowSize - settings.windowSize / 2);
    assert(startPos >=0 && endPos <= ma.getLength());
    ma = ma.mid(startPos, endPos - startPos);
    assert(ma.getLength() == settings.windowSize);

    SiteconAlgorithm::calculateACGTContent(ma, settings);
    settings.numSequencesInAlignment = ma.getNumRows();
    m.settings = settings;
    stateInfo.setDescription(tr("calculating_ave_disp_matrix"));
    m.matrix = SiteconAlgorithm::calculateDispersionAndAverage(ma, settings, stateInfo);
    if (stateInfo.hasError() || isCanceled()) {
        return;
    }
    stateInfo.setDescription(tr("calculating_weights"));
    SiteconAlgorithm::calculateWeights(ma, m.matrix, m.settings, false, stateInfo);
    if (stateInfo.hasError() || isCanceled()) {
        return;
    }
    stateInfo.progress+=5;
    stateInfo.setDescription(tr("calculating_firstTypeErr"));
    m.err1 = SiteconAlgorithm::calculateFirstTypeError(ma, settings, stateInfo);
    if (stateInfo.hasError() || isCanceled()) {
        return;
    }
    stateInfo.progress+=10;
    stateInfo.setDescription(tr("calculating_second_type_err"));
    m.err2 = SiteconAlgorithm::calculateSecondTypeError(m.matrix, settings, stateInfo);
    if (stateInfo.hasError() || isCanceled()) {
        return;
    }
}

SiteconBuildToFileTask::SiteconBuildToFileTask(const QString& inFile, const QString& _outFile, const SiteconBuildSettings& s) 
: Task (tr("build_sitecon_model"), TaskFlag_NoRun), loadTask(NULL), buildTask(NULL), outFile(_outFile), settings(s)
{
    tpm = Task::Progress_SubTasksBased;
    
    DocumentFormatConstraints c;
    c.checkRawData = true;
    c.supportedObjectTypes += GObjectTypes::MULTIPLE_ALIGNMENT;
    c.rawData = IOAdapterUtils::readFileHeader(inFile);
    QList<DocumentFormatId> formats = AppContext::getDocumentFormatRegistry()->selectFormats(c);
    if (formats.isEmpty()) {
        stateInfo.setError(  tr("input_format_error") );
        return;
    }
    DocumentFormatId format = formats.first();
    IOAdapterFactory* iof = AppContext::getIOAdapterRegistry()->getIOAdapterFactoryById(IOAdapterUtils::url2io(inFile));
    loadTask = new LoadDocumentTask(format, inFile, iof);
    loadTask->setSubtaskProgressWeight(0.03F);
    stateInfo.progress = 0;
    stateInfo.setDescription(tr("loading_ali"));
    addSubTask(loadTask);
}

QList<Task*> SiteconBuildToFileTask::onSubTaskFinished(Task* subTask) {
    QList<Task*> res;
    if (isCanceled()) {
        return res;
    }
    if (subTask->getStateInfo().hasError()) {
        stateInfo.setError(  subTask->getStateInfo().getError() );
        return res;
    }
    if (subTask == loadTask) {
        setUseDescriptionFromSubtask(true);
        Document* d = loadTask->getDocument();
        assert(d != NULL);
        QList<GObject*> mobjs = d->findGObjectByType(GObjectTypes::MULTIPLE_ALIGNMENT);
        if (mobjs.isEmpty()) {
            stateInfo.setError(  tr("no_alignments_found") );
        } else {
            MAlignmentObject* mobj =  qobject_cast<MAlignmentObject*>(mobjs.first());
            MAlignment ma = mobj->getMAlignment();
            QString baseName = mobj->getDocument()->getURL().baseFileName();
            buildTask = new SiteconBuildTask(settings, ma, baseName);
            res.append(buildTask);
        }
    } else if (subTask == buildTask) {
        Task* t = new SiteconWriteTask(outFile, buildTask->getResult());
        t->setSubtaskProgressWeight(0);
        res.append(t);
    }
    return res;
}

}//namespace
