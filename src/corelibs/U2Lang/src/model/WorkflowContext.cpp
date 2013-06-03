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

// Very-very bad designed function
static bool parseSlotParams(const QString &annsSlot, const QMap<QString, Actor*> &procMap,
                            Actor **proc, QString &portId, QString &path,
                            QString &headPortId, QString &headSlotId) {
    // parse @annsSlot
    QString slotId;
    {
        QStringList tokens = annsSlot.split(">");
        assert(tokens.size() > 0);
        if (tokens.size() > 1) {
            path = tokens[1];
        }
        tokens = tokens[0].split(".");
        assert(2 == tokens.size());
        slotId = tokens[1];

        *proc = procMap.value(tokens[0], NULL);
        if (NULL == *proc) {
            return false;
        }
    }

    // search headSlot
    QString headSlot;
    {
        foreach (Port *port, (*proc)->getOutputPorts()) {
            assert(port->getOutputType()->isMap());
            QMap<Descriptor, DataTypePtr> typeMap = port->getOutputType()->getDatatypesMap();

            if (typeMap.keys().contains(slotId)) {
                portId = port->getId();
                QString dependentSlot = portId + "." + slotId;
                QStrStrMap slotRelations = (*proc)->getProto()->getSlotRelations();
                if (slotRelations.contains(dependentSlot)) {
                    headSlot = slotRelations[dependentSlot];
                }
                break;
            }
        }
        if (headSlot.isEmpty()) {
            return false;
        }
    }

    QStringList tokens = headSlot.split(".");
    assert(2 == tokens.size());
    headPortId = tokens[0];
    headSlotId = tokens[1];

    return true;
}

static void searchAnnsSlot(Port *inPort, const QString &headSlotId, QString &newAnnsSlot, DataTypePtr &type) {
    Attribute *b = inPort->getParameter(IntegralBusPort::BUS_MAP_ATTR_ID);
    QStrStrMap busMap = b->getAttributeValueWithoutScript<QStrStrMap>();
    Attribute *p = inPort->getParameter(IntegralBusPort::PATHS_ATTR_ID);
    SlotPathMap pathMap = p->getAttributeValueWithoutScript<SlotPathMap>();
    QMap<Descriptor, DataTypePtr> typeMap = inPort->getOutputType()->getDatatypesMap();
    type = typeMap.value(headSlotId);
    newAnnsSlot = busMap.value(headSlotId);
    QPair<QString, QString> slotPair(headSlotId, newAnnsSlot);
    if (pathMap.contains(slotPair)) {
        newAnnsSlot += ">" + pathMap.value(slotPair).join(",");
    }
    newAnnsSlot = GrouperOutSlot::busMap2readable(newAnnsSlot);
}

QString WorkflowContext::getCorrespondingSeqSlot(const QString &annsSlot) {
    Actor *proc = NULL;
    QString path;
    QString portId;
    QString headPortId;
    QString headSlotId;

    // Very-very bad designed function
    bool pres = parseSlotParams(annsSlot, procMap, &proc, portId, path, headPortId, headSlotId);
    if (!pres) {
        return "";
    }

    QString res;
    if (headPortId == portId) {
        res = proc->getId() + "." + headSlotId;
    } else {
        Port *inPort = proc->getPort(headPortId);
        assert(inPort->getOutputType()->isMap());
        assert(inPort->isInput());
        if (inPort->isOutput()) {
            return "";
        }

        QString newAnnsSlot;
        DataTypePtr type;
        searchAnnsSlot(inPort, headSlotId, newAnnsSlot, type);

        if (BaseTypes::DNA_SEQUENCE_TYPE() == type) {
            res = newAnnsSlot;
        } else if (BaseTypes::ANNOTATION_TABLE_TYPE() == type) {
            res = this->getCorrespondingSeqSlot(newAnnsSlot);
        }
    }
    if (res.isEmpty()) {
        return "";
    }

    if (!path.isEmpty()) {
        if (1 == res.split(">").size()) {
            res += ">";
        } else {
            res += ",";
        }
        res += path;
    }
    return res;
}

const WorkflowProcess &WorkflowContext::getWorkflowProcess() const {
    return process;
}

WorkflowProcess &WorkflowContext::getWorkflowProcess() {
    return process;
}

} // Workflow
} // U2
