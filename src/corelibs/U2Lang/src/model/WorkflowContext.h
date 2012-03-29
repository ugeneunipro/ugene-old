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

#ifndef _U2_WORKFLOW_CONTEXT_H_
#define _U2_WORKFLOW_CONTEXT_H_

#include <U2Lang/Datatype.h>
#include <U2Lang/DbiDataStorage.h>

namespace U2 {
namespace Workflow {

class Actor;

/**
 * Contains a common data for whole workflow running process
 */
class U2LANG_EXPORT WorkflowContext {
public:
    WorkflowContext(const QList<Actor*> &procs);
    ~WorkflowContext();

    bool init();
    DbiDataStorage *getDataStorage();

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

private:
    DbiDataStorage *storage;
    QMap<QString, Actor*> procMap;
};

} // Workflow
} // U2


#endif // _U2_WORKFLOW_CONTEXT_H_
