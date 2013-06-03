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

#ifndef _U2_WORKFLOW_CONTEXT_H_
#define _U2_WORKFLOW_CONTEXT_H_

#include <QMutex>

#include <U2Core/AppFileStorage.h>

#include <U2Lang/Datatype.h>
#include <U2Lang/DbiDataStorage.h>

namespace U2 {
using namespace FileStorage;
namespace Workflow {

class Actor;
class WorkflowMonitor;

/**
 * Contains a common data for whole workflow running process
 */
class U2LANG_EXPORT WorkflowContext {
public:
    WorkflowContext(const QList<Actor*> &procs, WorkflowMonitor *monitor);
    virtual ~WorkflowContext();

    bool init();
    DbiDataStorage * getDataStorage();
    WorkflowMonitor * getMonitor();

    /**
     * @slotStr = "actor.slot>actor_path1,actor_path1,..."
     * or just "actor.slot"
     */
    DataTypePtr getOutSlotType(const QString &slotStr);
    /**
     * Frequently sequence annotations are associated with some sequence slot.
     * Returns this slot. If annotations are free then returns empty string.
     */
    QString getCorrespondingSeqSlot(const QString &annsSlot);
    /**
     * Files created by external tools workers could be used by some other scheme elements.
     * In that case, it is needed to add these files to the context and remove them after
     * the whole scheme performing is finished.
     */
    void addExternalProcessFile(const QString &url);

    const WorkflowProcess &getWorkflowProcess() const;
    WorkflowProcess &getWorkflowProcess();

private:
    WorkflowMonitor *monitor;
    DbiDataStorage *storage;
    QMap<QString, Actor*> procMap;

    QMutex addFileMutex;
    QStringList externalProcessFiles;
    WorkflowProcess process;
};

} // Workflow
} // U2


#endif // _U2_WORKFLOW_CONTEXT_H_
