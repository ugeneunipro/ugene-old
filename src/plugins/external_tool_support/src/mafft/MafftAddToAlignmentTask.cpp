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

#include <QtCore/QCoreApplication>
#include <QtCore/QDir>

#include "MAFFTSupport.h"
#include "MAFFTSupportTask.h"
#include "MafftAddToAlignmentTask.h"

#include <U2Core/AppContext.h>
#include <U2Core/AppSettings.h>
#include <U2Core/Counter.h>
#include <U2Core/UserApplicationsSettings.h>
#include <U2Core/DocumentModel.h>
#include <U2Core/ExternalToolRegistry.h>
#include <U2Core/GObjectUtils.h>
#include <U2Core/LoadDocumentTask.h>
#include <U2Core/Log.h>
#include <U2Core/ProjectModel.h>
#include <U2Core/MAlignmentExporter.h>
#include <U2Core/MAlignmentObject.h>
#include <U2Core/MsaDbiUtils.h>
#include <U2Core/MSAUtils.h>
#include <U2Core/IOAdapterUtils.h>
#include <U2Core/U2SafePoints.h>
#include <U2Core/AddDocumentTask.h>
#include <U2Core/U2SafePoints.h>
#include <U2Core/U2Mod.h>
#include <U2Core/U2OpStatusUtils.h>

#include <U2Algorithm/AlignmentAlgorithmsRegistry.h>
#include <U2Algorithm/BaseAlignmentAlgorithmsIds.h>

#include <U2Gui/OpenViewTask.h>

namespace U2 {

/************************************************************************/
/* MafftAddToAlignmentTask */
/************************************************************************/
MafftAddToAlignmentTask::MafftAddToAlignmentTask(const AlignSequencesToAlignmentTaskSettings& settings)
: AbstractAlignmentTask(tr("Align sequences to alignment task"), TaskFlag_NoRun), settings(settings) {
    GCOUNTER(cvar, tvar, "MafftAddToAlignmentTask" );

    SAFE_POINT_EXT(settings.isValid(), setError("Incorrect settings were passed into MafftAddToAlignmentTask"), );

    MAlignmentExporter alnExporter;
    inputMsa = alnExporter.getAlignment(settings.msaRef.dbiRef, settings.msaRef.entityId, stateInfo);
}

MafftAddToAlignmentTask::~MafftAddToAlignmentTask() {
    delete logParser;
}

void MafftAddToAlignmentTask::prepare()
{
    algoLog.info(tr("Align sequences to an existing alignment by MAFFT started"));

    tmpDirUrl = ExternalToolSupportUtils::createTmpDir("add_to_alignment", stateInfo);
    QString tmpAddedUrl = tmpDirUrl + QDir::separator() + inputMsa.getName() + "_add" + ".fa";

    DocumentFormatRegistry *dfr = AppContext::getDocumentFormatRegistry();
    DocumentFormat *dfd = dfr->getFormatById(BaseDocumentFormats::FASTA);
    Document* tempDocument = dfd->createNewLoadedDocument(IOAdapterUtils::get(BaseIOAdapters::LOCAL_FILE), GUrl(tmpAddedUrl), stateInfo);

    QListIterator<QString> namesIterator(settings.addedSequencesNames);
    foreach(const U2EntityRef& sequenceRef, settings.addedSequencesRefs) {
        tempDocument->addObject(U2SequenceObject(namesIterator.next(), sequenceRef).clone(tempDocument->getDbiRef(), stateInfo));
    }

    saveSequencesDocumentTask = new SaveDocumentTask(tempDocument, tempDocument->getIOAdapterFactory(), tmpAddedUrl, SaveDocFlags(SaveDoc_Roll) | SaveDoc_DestroyAfter);
    addSubTask(saveSequencesDocumentTask);

    QString tmpExistingAlignmentUrl = tmpDirUrl + QDir::separator() + inputMsa.getName() + ".fa";
    saveAlignmentDocumentTask = new SaveMSA2SequencesTask(inputMsa, tmpExistingAlignmentUrl, false, BaseDocumentFormats::FASTA);
    addSubTask(saveAlignmentDocumentTask);
}

QList<Task*> MafftAddToAlignmentTask::onSubTaskFinished(Task* subTask) {
    QList<Task*> subTasks;

    propagateSubtaskError();
    if(subTask->isCanceled() || isCanceled() || hasError()) {
        return subTasks;
    }

    if((subTask == saveAlignmentDocumentTask || subTask == saveSequencesDocumentTask) && saveAlignmentDocumentTask->isFinished() 
        && saveSequencesDocumentTask->isFinished()) {

        resultFilePath = settings.resultFileName.isEmpty() ? tmpDirUrl + QDir::separator() + "result_aln.fa" : settings.resultFileName.getURLString();
        QStringList arguments;
        if(settings.addAsFragments) {
            arguments << "--addfragments";
        } else {
            arguments << "--add";
        }
        arguments << saveSequencesDocumentTask->getURL().getURLString();
        if(settings.reorderSequences) {
            arguments << "--reorder";
        }
        arguments << saveAlignmentDocumentTask->getDocument()->getURLString();
        QString outputUrl = resultFilePath + ".out.fa";

        logParser = new MAFFTLogParser(inputMsa.getNumRows(), 1, outputUrl);
        connect(logParser, SIGNAL(si_progressUndefined()), SLOT(sl_progressUndefined()));
        mafftTask = new ExternalToolRunTask(ET_MAFFT, arguments, logParser);
        mafftTask->setStandartOutputFile(resultFilePath);
        mafftTask->setSubtaskProgressWeight(95);
        subTasks.append(mafftTask);
    } else if (subTask == mafftTask) {
        SAFE_POINT(logParser != NULL, "logParser is null", subTasks);
        logParser->cleanup();
        if (!QFileInfo(resultFilePath).exists()) {
            if (AppContext::getExternalToolRegistry()->getByName(ET_MAFFT)->isValid()){
                stateInfo.setError(tr("Output file '%1' not found").arg(resultFilePath));
            } else {
                stateInfo.setError(tr("Output file '%3' not found. May be %1 tool path '%2' not valid?")
                    .arg(AppContext::getExternalToolRegistry()->getByName(ET_MAFFT)->getName())
                    .arg(AppContext::getExternalToolRegistry()->getByName(ET_MAFFT)->getPath())
                    .arg(resultFilePath));
            }
            return subTasks;
        }
        ioLog.details(tr("Loading output file '%1'").arg(resultFilePath));
        IOAdapterFactory* iof = AppContext::getIOAdapterRegistry()->getIOAdapterFactoryById(BaseIOAdapters::LOCAL_FILE);
        loadTmpDocumentTask = new LoadDocumentTask(BaseDocumentFormats::FASTA, resultFilePath, iof);
        loadTmpDocumentTask->setSubtaskProgressWeight(5);
        subTasks.append(loadTmpDocumentTask);
    } else if (subTask == loadTmpDocumentTask) {
        tmpDoc = QSharedPointer<Document>(loadTmpDocumentTask->takeDocument());
        SAFE_POINT(tmpDoc != NULL, QString("output document '%1' not loaded").arg(tmpDoc->getURLString()), subTasks);
        SAFE_POINT(tmpDoc->getObjects().length()!=0, QString("no objects in output document '%1'").arg(tmpDoc->getURLString()), subTasks);


        U2UseCommonUserModStep modStep(settings.msaRef, stateInfo);
        CHECK_OP(stateInfo, subTasks);
        U2MsaDbi *dbi = modStep.getDbi()->getMsaDbi();

        QStringList rowNames = inputMsa.getRowNames();

        QList<U2MsaRow> rows = dbi->getRows(settings.msaRef.entityId, stateInfo);

        int posInMsa = 0;
        foreach(GObject* object, tmpDoc->getObjects()) {
            U2SequenceObject* sequenceObject = qobject_cast<U2SequenceObject*>(object);
            if(!rowNames.contains(sequenceObject->getSequenceName())) {
                SAFE_POINT(sequenceObject != NULL, "U2SequenceObject is null", subTasks);
                U2MsaRow row = MSAUtils::copyRowFromSequence(sequenceObject, settings.msaRef.dbiRef, stateInfo);
                dbi->addRow(settings.msaRef.entityId, posInMsa, row, stateInfo);
            }
            posInMsa++;
        }

         if (hasError()) {
            return subTasks;
        }
        algoLog.info(tr("MAFFT alignment successfully finished"));
    }

    return subTasks;
}

Task::ReportResult MafftAddToAlignmentTask::report() {
    ExternalToolSupportUtils::removeTmpDir(tmpDirUrl, stateInfo);

    return ReportResult_Finished;
}

AbstractAlignmentTask* MafftAddToAlignmentTaskFactory::getTaskInstance(AbstractAlignmentTaskSettings *_settings) const {
    AlignSequencesToAlignmentTaskSettings* addSettings = dynamic_cast<AlignSequencesToAlignmentTaskSettings*>(_settings);
    SAFE_POINT(addSettings != NULL,
        "Add sequences to alignment: incorrect settings", NULL);
    return new MafftAddToAlignmentTask(*addSettings);
}

MafftAddToAligmnentAlgorithm::MafftAddToAligmnentAlgorithm() :
AlignmentAlgorithm(AddToAlignment, BaseAlignmentAlgorithmsIds::ALIGN_SEQUENCES_TO_ALIGNMENT_BY_MAFFT,
                     new MafftAddToAlignmentTaskFactory(),
                     NULL)
{
}

bool MafftAddToAligmnentAlgorithm::isAlgorithmAvailable() const {
    return AppContext::getExternalToolRegistry()->getByName("MAFFT")->isValid();
}

}
