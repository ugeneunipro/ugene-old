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
#include <QTemporaryFile>

#include "MAFFTSupport.h"
#include "MAFFTSupportTask.h"
#include "MafftAddToAlignmentTask.h"

#include <U2Core/AppContext.h>
#include <U2Core/AppResources.h>
#include <U2Core/AppSettings.h>
#include <U2Core/Counter.h>
#include <U2Core/DNAAlphabet.h>
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
#include <U2Core/U2AlphabetUtils.h>
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
  : AbstractAlignmentTask(tr("Align sequences to alignment task"), TaskFlag_None),
    settings(settings),
    logParser(NULL),
    saveSequencesDocumentTask(NULL),
    saveAlignmentDocumentTask(NULL),
    mafftTask(NULL),
    loadTmpDocumentTask(NULL),
    modStep(NULL)
{
    GCOUNTER(cvar, tvar, "MafftAddToAlignmentTask" );

    SAFE_POINT_EXT(settings.isValid(), setError("Incorrect settings were passed into MafftAddToAlignmentTask"), );

    MAlignmentExporter alnExporter;
    inputMsa = alnExporter.getAlignment(settings.msaRef.dbiRef, settings.msaRef.entityId, stateInfo);
    int rowNumber = inputMsa.getNumRows();
    for (int i = 0; i < rowNumber; i++) {
        inputMsa.renameRow(i, QString::number(i));
    }
}

static QString generateTmpFileUrl(const QString &filePathAndPattern) {
    QTemporaryFile *generatedFile = new QTemporaryFile(filePathAndPattern);
    QFileInfo generatedFileInfo(generatedFile->fileName());
    while (generatedFile->exists() || generatedFileInfo.baseName().contains(" ") || !generatedFile->open()) {
        delete generatedFile;
        generatedFile = new QTemporaryFile(filePathAndPattern);
    }
    generatedFile->close();
    QString result = generatedFile->fileName();
    delete generatedFile;
    return result;
}

void MafftAddToAlignmentTask::prepare()
{
    algoLog.info(tr("Align sequences to an existing alignment by MAFFT started"));

    tmpDirUrl = ExternalToolSupportUtils::createTmpDir("add_to_alignment", stateInfo);

    QString tmpAddedUrl = generateTmpFileUrl(tmpDirUrl + QDir::separator() + "XXXXXXXXXXXXXXXX_add.fa");;

    DocumentFormatRegistry *dfr = AppContext::getDocumentFormatRegistry();
    DocumentFormat *dfd = dfr->getFormatById(BaseDocumentFormats::FASTA);
    Document* tempDocument = dfd->createNewLoadedDocument(IOAdapterUtils::get(BaseIOAdapters::LOCAL_FILE), GUrl(tmpAddedUrl), stateInfo);

    QListIterator<QString> namesIterator(settings.addedSequencesNames);
    int currentRowNumber = inputMsa.getNumRows();
    foreach(const U2EntityRef& sequenceRef, settings.addedSequencesRefs) {
        uniqueIdsToNames[QString::number(currentRowNumber)] = namesIterator.next();
        U2SequenceObject seqObject(QString::number(currentRowNumber), sequenceRef);
        GObject* cloned = seqObject.clone(tempDocument->getDbiRef(), stateInfo);
        CHECK_OP(stateInfo, );
        cloned->setGObjectName(QString::number(currentRowNumber));
        tempDocument->addObject(cloned);
        currentRowNumber++;
    }

    saveSequencesDocumentTask = new SaveDocumentTask(tempDocument, tempDocument->getIOAdapterFactory(), tmpAddedUrl, SaveDocFlags(SaveDoc_Roll) | SaveDoc_DestroyAfter);
    addSubTask(saveSequencesDocumentTask);

    QString tmpExistingAlignmentUrl = generateTmpFileUrl(tmpDirUrl + QDir::separator() + "XXXXXXXXXXXXXXXX.fa");

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
        const DNAAlphabet* alphabet = U2AlphabetUtils::getById(settings.alphabet);
        SAFE_POINT_EXT(alphabet != NULL, setError("Albhabet is invalid."), subTasks);
        if(alphabet->isRaw()) {
            arguments << "--anysymbol";
        }
        if(useMemsaveOption()) {
            arguments << "--memsave";
        }
        if(settings.reorderSequences) {
            arguments << "--reorder";
        }
        arguments << saveAlignmentDocumentTask->getDocument()->getURLString();
        QString outputUrl = resultFilePath + ".out.fa";

        logParser = new MAFFTLogParser(inputMsa.getNumRows(), 1, outputUrl);
        mafftTask = new ExternalToolRunTask(ET_MAFFT, arguments, logParser);
        mafftTask->setStandartOutputFile(resultFilePath);
        mafftTask->setSubtaskProgressWeight(65);
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
    } else if(subTask == loadTmpDocumentTask) {
        modStep = new U2UseCommonUserModStep(settings.msaRef, stateInfo);
    }

    return subTasks;
}

void MafftAddToAlignmentTask::run() {
    CHECK_OP(stateInfo, );
    tpm = Progress_Manual;
    SAFE_POINT(loadTmpDocumentTask != NULL, QString("Load task is NULL"), );
    tmpDoc = QSharedPointer<Document>(loadTmpDocumentTask->takeDocument());
    SAFE_POINT(tmpDoc != NULL, QString("output document '%1' not loaded").arg(tmpDoc->getURLString()), );
    SAFE_POINT(tmpDoc->getObjects().length()!=0, QString("no objects in output document '%1'").arg(tmpDoc->getURLString()), );

    U2MsaDbi *dbi = modStep->getDbi()->getMsaDbi();

    QStringList rowNames = inputMsa.getRowNames();

    int posInMsa = 0;
    int objectsCount = tmpDoc->getObjects().count();

    dbi->updateMsaAlphabet(settings.msaRef.entityId, settings.alphabet, stateInfo);
    CHECK_OP(stateInfo, );
    foreach(GObject* object, tmpDoc->getObjects()) {
        if (hasError() || isCanceled()) {
            return;
        }
        stateInfo.setProgress(70 + 30 * posInMsa / objectsCount);
        U2SequenceObject* sequenceObject = qobject_cast<U2SequenceObject*>(object);
        if(!rowNames.contains(sequenceObject->getSequenceName())) {
            sequenceObject->setGObjectName(uniqueIdsToNames[sequenceObject->getGObjectName()]);
            SAFE_POINT(sequenceObject != NULL, "U2SequenceObject is null", );
            U2MsaRow row = MSAUtils::copyRowFromSequence(sequenceObject, settings.msaRef.dbiRef, stateInfo);
            dbi->addRow(settings.msaRef.entityId, posInMsa, row, stateInfo);
            CHECK_OP(stateInfo, );
        }
        posInMsa++;
    }

    if (hasError()) {
        return;
    }
    algoLog.info(tr("MAFFT alignment successfully finished"));
}

Task::ReportResult MafftAddToAlignmentTask::report() {
    ExternalToolSupportUtils::removeTmpDir(tmpDirUrl, stateInfo);
    delete modStep;

    return ReportResult_Finished;
}

bool MafftAddToAlignmentTask::useMemsaveOption() const {
    qint64 maxLength = qMax(qint64(inputMsa.getLength()), settings.maxSequenceLength);
    qint64 memoryInMB = 10 * maxLength * maxLength / 1024 / 1024;
    AppResourcePool* pool = AppContext::getAppSettings()->getAppResourcePool();
    return memoryInMB > qMin(pool->getMaxMemorySizeInMB(), pool->getTotalPhysicalMemory() / 2);
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
