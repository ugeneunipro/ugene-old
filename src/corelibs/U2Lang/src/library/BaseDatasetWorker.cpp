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

#include <U2Core/L10n.h>
#include <U2Core/U2OpStatusUtils.h>
#include <U2Core/U2SafePoints.h>

#include "BaseDatasetWorker.h"

namespace U2 {
namespace LocalWorkflow {

BaseDatasetWorker::BaseDatasetWorker(Actor *a, const QString &inPortId, const QString &outPortId)
: BaseWorker(a, /* autoTransitBus= */false), inPortId(inPortId), outPortId(outPortId), input(NULL), output(NULL), prepared(false), datasetInited(false)
{

}

void BaseDatasetWorker::init() {
    input = ports.value(inPortId);
    output = ports.value(outPortId);
    datasetInited = false;
}

Task * BaseDatasetWorker::tick() {
    if (!prepared) {
        U2OpStatusImpl os;
        Task *prepareTask = prepare(os);
        CHECK_OP(os, NULL);
        if (NULL != prepareTask) {
            return prepareTask;
        }
    }

    if (input->hasMessage()) {
        if (datasetChanged(input->lookMessage())) {
            return onDatasetChanged();
        } else {
            takeMessage();
        }
    } else if (input->isEnded()) {
        if (!datasetMessages.isEmpty()) {
            return onDatasetChanged();
        }
        output->setEnded();
        setDone();
    }
    return NULL;
}

void BaseDatasetWorker::cleanup() {
    datasetMessages.clear();
}

MessageMetadata BaseDatasetWorker::generateMetadata(const QString &datasetName) const {
    return MessageMetadata(datasetName);
}

void BaseDatasetWorker::sl_taskFinished() {
    Task *task = dynamic_cast<Task*>(sender());
    CHECK(NULL != task, );
    CHECK(task->isFinished() && !task->isCanceled() && !task->hasError(), );
    U2OpStatusImpl os;
    const QVariantMap result = getResult(task, os);
    CHECK_OP_EXT(os, reportError(os.getError()), );

    const MessageMetadata metadata = generateMetadata(datasetName);
    context->getMetadataStorage().put(metadata);
    const Message message(output->getBusType(), result, metadata.getId());
    output->put(message);
}

void BaseDatasetWorker::sl_prepared() {
    Task *task = dynamic_cast<Task*>(sender());
    CHECK(NULL != task, );
    CHECK(task->isFinished() && !task->isCanceled() && !task->hasError(), );
    U2OpStatusImpl os;
    onPrepared(task, os);
    if (os.hasError()) {
        reportError(os.getError());
        output->setEnded();
        setDone();
    }
}

Task * BaseDatasetWorker::prepare(U2OpStatus &os) {
    CHECK(!prepared, NULL);
    Task *task = createPrepareTask(os);
    if (os.hasError()) {
        reportError(os.getError());
        output->setEnded();
        setDone();
    }
    if (NULL != task) {
        connect(task, SIGNAL(si_stateChanged()), SLOT(sl_prepared()));
    }
    prepared = true;
    return task;
}

QString BaseDatasetWorker::getDatasetName(const Message &message) const {
    const int metadataId = message.getMetadataId();
    const MessageMetadata metadata = context->getMetadataStorage().get(metadataId);
    return metadata.getDatasetName();
}

bool BaseDatasetWorker::datasetChanged(const Message &message) const {
    CHECK(datasetInited, false);
    return (getDatasetName(message) != datasetName);
}

void BaseDatasetWorker::takeMessage() {
    const Message message = getMessageAndSetupScriptValues(input);
    datasetMessages << message;

    if (!datasetInited) {
        datasetInited = true;
        datasetName = getDatasetName(message);
    }

    SAFE_POINT(!datasetChanged(message), L10N::internalError("Unexpected method call"), );
}

Task * BaseDatasetWorker::onDatasetChanged() {
    datasetInited = false;
    Task *task = createTask(datasetMessages);
    connect(task, SIGNAL(si_stateChanged()), SLOT(sl_taskFinished()));
    datasetMessages.clear();
    return task;
}

} // LocalWorkflow
} // U2
