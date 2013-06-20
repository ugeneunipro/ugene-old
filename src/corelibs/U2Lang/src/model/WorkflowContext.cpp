/**
 * UGENE - Integrated Bioinformatics Tools.
 * Copyright (C) 2008-2013 UniPro <ugene@unipro.ru>
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

#include <QtGui/QApplication>
#include <QtCore/QMutexLocker>

#include <U2Core/AppContext.h>
#include <U2Core/AppFileStorage.h>
#include <U2Core/U2OpStatusUtils.h>
#include <U2Core/U2SafePoints.h>

#include <U2Lang/ActorModel.h>
#include <U2Lang/BaseTypes.h>
#include <U2Lang/Datatype.h>
#include <U2Lang/GrouperOutSlot.h>
#include <U2Lang/IntegralBus.h>
#include <U2Lang/WorkflowMonitor.h>

#include "WorkflowContext.h"

namespace U2 {

namespace Workflow {

static QString getWorkflowId(WorkflowContext *ctx) {
    qint64 pid = QApplication::applicationPid();
    QString wId = QByteArray::number(pid) + "_" + QByteArray::number(qint64(ctx));

    return wId;
}

WorkflowContext::WorkflowContext(const QList<Actor*> &procs, WorkflowMonitor *_monitor)
: monitor(_monitor), storage(NULL), process("")
{
    foreach (Actor *p, procs) {
        procMap.insert(p->getId(), p);
    }
    
    { // register WD process
        AppFileStorage *fileStorage = AppContext::getAppFileStorage();
        CHECK(NULL != fileStorage, );

        U2OpStatusImpl os;
        process = WorkflowProcess(getWorkflowId(this));
        fileStorage->registerWorkflowProcess(process, os);
        CHECK_OP(os, );
    }
}

WorkflowContext::~WorkflowContext() {
    foreach (const QString &url, externalProcessFiles) {
        QFile::remove(url);
    }
    delete storage;

    // unregister WD process
    if (!process.getId().isEmpty()) {
        AppFileStorage *fileStorage = AppContext::getAppFileStorage();
        CHECK(NULL != fileStorage, );

        U2OpStatusImpl os;
        fileStorage->unregisterWorkflowProcess(process, os);
    }
}

bool WorkflowContext::init() {
    storage = new DbiDataStorage();
    return storage->init();
}

DbiDataStorage * WorkflowContext::getDataStorage() {
    return storage;
}

WorkflowMonitor * WorkflowContext::getMonitor() {
    return monitor;
}

void WorkflowContext::addExternalProcessFile(const QString &url) {
    QMutexLocker locker(&addFileMutex);
    externalProcessFiles << url;
}

DataTypePtr WorkflowContext::getOutSlotType(const QString &slotStr) {
    QStringList tokens = slotStr.split(">");
    assert(tokens.size() > 0);
    tokens = tokens[0].split(".");
    assert(2 == tokens.size());

    Actor *proc = procMap.value(tokens[0], NULL);
    if (NULL == proc) {
        return DataTypePtr();
    }

    QString slotId = tokens[1];
    foreach (Port *port, proc->getOutputPorts()) {
        assert(port->getOutputType()->isMap());
        QMap<Descriptor, DataTypePtr> typeMap = port->getOutputType()->getDatatypesMap();

        if (typeMap.keys().contains(slotId)) {
            DataTypePtr type = typeMap.value(slotId);
            assert(DataType::Single == type->kind());
            return type;
        }
    }

    return DataTypePtr();
}

const WorkflowProcess &WorkflowContext::getWorkflowProcess() const {
    return process;
}

WorkflowProcess &WorkflowContext::getWorkflowProcess() {
    return process;
}

} // Workflow
} // U2
