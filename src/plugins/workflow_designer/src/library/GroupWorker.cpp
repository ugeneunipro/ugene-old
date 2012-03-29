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

#include <U2Core/AnnotationData.h>
#include <U2Core/MAlignment.h>
#include <U2Core/U2OpStatusUtils.h>
#include <U2Core/U2SafePoints.h>
#include <U2Core/U2SequenceUtils.h>
#include <U2Core/QVariantUtils.h>

#include <U2Designer/DelegateEditors.h>
#include <U2Designer/GrouperEditor.h>

#include <U2Lang/ActorPrototypeRegistry.h>
#include <U2Lang/BaseActorCategories.h>
#include <U2Lang/BaseTypes.h>
#include <U2Lang/CoreLibConstants.h>
#include <U2Lang/GrouperSlotAttribute.h>
#include <U2Lang/WorkflowEnv.h>

#include "GroupWorker.h"

namespace U2 {
namespace LocalWorkflow {

static const QString INPUT_PORT("input-data");
static const QString OUTPUT_PORT("output-data");

static const QString OPER_ATTR_ID("group-op");

/************************************************************************/
/* GroupWorker */
/************************************************************************/
GroupWorker::GroupWorker(Actor *p)
: BaseWorker(p, false), inChannel(NULL), outChannel(NULL), messageCounter(0)
{
    
}

void GroupWorker::init() {
    inChannel = ports.value(INPUT_PORT);
    outChannel = ports.value(OUTPUT_PORT);
    mtype = outChannel->getBusType();

    GrouperSlotAttribute *slotsAttr = dynamic_cast<GrouperSlotAttribute*>(actor->getParameter(CoreLibConstants::GROUPER_OUT_SLOTS_ATTR));
    outSlots = slotsAttr->getOutSlots();
    groupSlot = actor->getParameter(CoreLibConstants::GROUPER_SLOT_ATTR)->getAttributePureValue().toString();
    inType = context->getOutSlotType(groupSlot);
    groupOp = actor->getParameter(OPER_ATTR_ID)->getAttributePureValue().toString();

    if (groupOp != GroupOperations::BY_VALUE().getId()
        && groupOp != GroupOperations::BY_NAME().getId()
        && groupOp != GroupOperations::BY_ID().getId()) {
        groupOp = GroupOperations::BY_ID().getId();
    }
    groupSlot = GrouperOutSlot::readable2busMap(groupSlot);
}

Task *GroupWorker::tick() {
    while (inChannel->hasMessage()) {
        Message inMessage = inChannel->look();
        inChannel->get();
        QVariantMap mData = inMessage.getData().toMap();
        if (!mData.keys().contains(groupSlot)) {
            continue;
        }
        messageCounter++;

        QVariant gsData = mData.value(groupSlot);
        // search group slot data at unique data
        int foundId = -1;
        PerformersMap perfs;
        foreach (int id, uniqueData.keys()) {
            const QVariant &d = uniqueData[id];
            bool equal = GrouperActionUtils::equalData(groupOp, d, gsData, inType, context);
            if (equal) {
                foundId = id;
                perfs = groupedData[id];
                break;
            }
        }

        // apply actions for out slots
        GrouperActionUtils::applyActions(context, outSlots, mData, perfs);

        // add new unique data and action performers
        if (foundId < 0) {
            foundId = uniqueData.size();
            uniqueData[foundId] = gsData;
        }
        groupedData[foundId] = perfs;
    }
    if (inChannel->isEnded()) {
        foreach (int id, groupedData.keys()) {
            QMap<QString, ActionPerformer*> perfs = groupedData[id];

            QVariantMap data;
            // create output data set from action performers
            foreach (const QString &slotId, perfs.keys()) {
                ActionPerformer *perf = perfs[slotId];
                U2OpStatusImpl os;
                QVariant slotData = perf->finishAction(os);
                if (os.isCoR()) {
                    continue;
                }
                data[slotId] = slotData;
            }

            QVariantMap context;
            context[groupSlot] = uniqueData[id];
            outChannel->setContext(context);
            outChannel->put(Message(mtype, data));
        }
        setDone();
        outChannel->setEnded();
    }

    return NULL;
}

void GroupWorker::cleanup() {
    foreach (const PerformersMap &perfs, groupedData.values()) {
        foreach (ActionPerformer *p, perfs.values()) {
            delete p;
        }
    }
}

/************************************************************************/
/* GroupWorkerFactory */
/************************************************************************/
const QString GroupWorkerFactory::ACTOR_ID = CoreLibConstants::GROUPER_ID;

void GroupWorkerFactory::init() {
    QList<PortDescriptor*> portDescs;
    {
        QMap<Descriptor, DataTypePtr> emptyTypeMap;
        DataTypePtr emptyTypeSet(new MapDataType(Descriptor(DataType::EMPTY_TYPESET_ID), emptyTypeMap));

        Descriptor inputDesc1(INPUT_PORT, GroupWorker::tr("Input data flow"), GroupWorker::tr("Input data flow"));
        portDescs << new PortDescriptor(inputDesc1, emptyTypeSet, true);

        Descriptor outputDesc(OUTPUT_PORT, GroupWorker::tr("Grouped output data flow"), GroupWorker::tr("Grouped output data flow"));
        portDescs << new PortDescriptor(outputDesc, emptyTypeSet, false, true);
    }

    QList<Attribute*> attrs;
    {
        Descriptor slotsDesc(CoreLibConstants::GROUPER_OUT_SLOTS_ATTR, GroupWorker::tr("Out slots"), GroupWorker::tr("Out slots"));
        Attribute *slotsAttr = new GrouperSlotAttribute(slotsDesc, BaseTypes::STRING_TYPE(), false);

        Descriptor groupDesc(CoreLibConstants::GROUPER_SLOT_ATTR, GroupWorker::tr("Group slot"), GroupWorker::tr("Group slot"));
        Attribute *groupAttr = new Attribute(groupDesc, BaseTypes::STRING_TYPE(), true);

        Descriptor opDesc(OPER_ATTR_ID, GroupWorker::tr("Group operation"), GroupWorker::tr("Group operation"));
        Attribute *opAttr = new Attribute(opDesc, BaseTypes::STRING_TYPE(), true);

        attrs << slotsAttr;
        attrs << groupAttr;
        attrs << opAttr;
    }

    Descriptor protoDesc(GroupWorkerFactory::ACTOR_ID,
        GroupWorker::tr("Grouper"),
        GroupWorker::tr("Groups incoming data by set input slot"));

    ActorPrototype *proto = new IntegralBusActorPrototype(protoDesc, portDescs, attrs);

    proto->setEditor(new GrouperEditor());
    proto->setPrompter(new GroupPrompter());
    proto->setAllowsEmptyPorts(true);

    WorkflowEnv::getProtoRegistry()->registerProto(BaseActorCategories::CATEGORY_DATAFLOW(), proto);
    WorkflowEnv::getDomainRegistry()->getById(LocalDomainFactory::ID)->registerEntry(new GroupWorkerFactory());
}

Worker *GroupWorkerFactory::createWorker(Actor *a) {
    return new GroupWorker(a);
}

/************************************************************************/
/* GroupPrompter */
/************************************************************************/
QString GroupPrompter::composeRichDoc() {
    QString unsetStr = "<font color='red'>" + tr("unset") + "</font>";
    QString inputName = unsetStr;

    Port *input = target->getInputPorts().first();
    if (input->getLinks().size() > 0) {
        Port *src = input->getLinks().keys().first();
        IntegralBusPort *bus = dynamic_cast<IntegralBusPort*>(src);
        assert(NULL != bus);
        DataTypePtr type = bus->getType();
        QMap<Descriptor, DataTypePtr> busMap = type->getDatatypesMap();

        Attribute *groupSlotAttr = target->getParameter(CoreLibConstants::GROUPER_SLOT_ATTR);
        QString groupSlot = groupSlotAttr->getAttributeValueWithoutScript<QString>();
        groupSlot = GrouperOutSlot::readable2busMap(groupSlot);

        foreach (const Descriptor &d, busMap.keys()) {
            if (d.getId() == groupSlot) {
                inputName = d.getDisplayName();
                break;
            }
        }
    }

    QString result = tr("Group all incoming messages by<u>%1</u> of <u>%2</u> slot data");
    if (inputName == unsetStr) {
        return result.arg("").arg(inputName);
    } else {
        Attribute *groupOpAttr = target->getParameter(CoreLibConstants::GROUPER_OPER_ATTR);
        QString opId = groupOpAttr->getAttributeValueWithoutScript<QString>();

        QString op;
        if ("by-id" == opId) {
            op = tr(" id");
        } else if ("by-name" == opId) {
            op = tr(" name");
        } else if ("by-value" == opId) {
            op = tr(" value");
        }
        return result.arg(op).arg(inputName);
    }
}

} // LocalWorkflow
} // U2
