/**
 * UGENE - Integrated Bioinformatics Tools.
 * Copyright (C) 2008-2012 UniPro <ugene@unipro.ru>
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

#include "ExportConsensusTask.h"
#include <U2Core/DNASequenceObject.h>
#include <U2Core/DocumentModel.h>
#include <U2Core/IOAdapterUtils.h>
#include <U2Core/ProjectModel.h>
#include <U2Core/SaveDocumentTask.h>
#include <U2Core/U2AlphabetUtils.h>
#include <U2Core/U2SafePoints.h>
#include <U2Formats/DocumentFormatUtils.h>
#include <U2Gui/OpenViewTask.h>

namespace U2 {

ExportConsensusTask::ExportConsensusTask(const ExportConsensusTaskSettings &settings_)
    : DocumentProviderTask("", TaskFlags_NR_FOSE_COSC), settings(settings_)
{
    setTaskName(tr("Export consensus of assembly '%1' to '%2'")
                .arg(settings.model->getAssembly().visualName)
                .arg(QFileInfo(settings.fileName).fileName()));
    setMaxParallelSubtasks(1);
}

void ExportConsensusTask::prepare() {
    SAFE_POINT_EXT(!settings.fileName.isEmpty(), setError(tr("File name cannot be empty")),);

    DocumentFormat * df = AppContext::getDocumentFormatRegistry()->getFormatById(settings.formatId);
    SAFE_POINT_EXT(df != NULL, setError(tr("Internal: couldn't find document format with id '%1'").arg(settings.formatId)),);

    IOAdapterFactory * iof = AppContext::getIOAdapterRegistry()->getIOAdapterFactoryById(IOAdapterUtils::url2io(settings.fileName));
    resultDocument = df->createNewLoadedDocument(iof, settings.fileName, stateInfo);
    CHECK_OP(stateInfo, );

    SaveDocFlags saveFlags = SaveDoc_Overwrite;
    consensusTask = new AssemblyConsensusTask(settings);
    consensusTask->setSubtaskProgressWeight(100);
    addSubTask(consensusTask);

    addSubTask(new SaveDocumentTask(resultDocument, saveFlags));

    Project * p = AppContext::getProject();
    if(p != NULL && p->findDocumentByURL(resultDocument->getURL()) != NULL) {
        // if already has such document in project, do not add
        settings.addToProject = false;
    }

    if(settings.addToProject) {
        addSubTask(new AddDocumentAndOpenViewTask(takeDocument()));
    }
}

QList<Task*> ExportConsensusTask::onSubTaskFinished(Task *finished) {
    QList<Task*> newSubTasks;
    if(finished == consensusTask) {
        QByteArray consensus = consensusTask->getResult().consensus;

        if(! settings.keepGaps) {
            consensus.replace(QString(AssemblyConsensusAlgorithm::EMPTY_CHAR), "");
        }

        U2SequenceObject *seqObj = DocumentFormatUtils::addSequenceObject(resultDocument->getDbiRef(), settings.seqObjName, consensus, settings.circular, QVariantMap(), stateInfo);
        CHECK_OP(stateInfo, newSubTasks);

        resultDocument->addObject(seqObj);
    }
    return newSubTasks;
}

} // namespace
