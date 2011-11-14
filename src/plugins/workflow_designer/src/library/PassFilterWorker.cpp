/**
 * UGENE - Integrated Bioinformatics Tools.
 * Copyright (C) 2008-2011 UniPro <ugene@unipro.ru>
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

#include <U2Designer/DelegateEditors.h>

#include <U2Lang/ActorPrototypeRegistry.h>
#include <U2Lang/BaseActorCategories.h>
#include <U2Lang/BaseAttributes.h>
#include <U2Lang/BasePorts.h>
#include <U2Lang/BaseSlots.h>
#include <U2Lang/BaseTypes.h>
#include <U2Lang/Marker.h>
#include <U2Lang/WorkflowEnv.h>

#include "PassFilterWorker.h"

namespace U2 {
namespace LocalWorkflow {

const QString PassFilterWorkerFactory::ACTOR_ID("pass-filter");

/*******************************
 * FilterSequenceWorker
 *******************************/
PassFilterWorker::PassFilterWorker(Actor *p)
: BaseWorker(p), inChannel(NULL), outChannel(NULL), done(false)
{
}

void PassFilterWorker::init() {
    inChannel = ports.value(BasePorts::IN_TEXT_PORT_ID());
    outChannel = ports.value(BasePorts::OUT_TEXT_PORT_ID());

    foreach (QString val, actor->getAttributes().first()->getAttributePureValue().toString().split(",")) {
        passedValues << val.trimmed();
    }
}

bool PassFilterWorker::isReady() {
    int hasMsg = inChannel->hasMessage();
    bool ended = inChannel->isEnded();
    return hasMsg || (ended && !done);
}

Task *PassFilterWorker::tick() {
    while (inChannel->hasMessage()) {
        Message inputMessage = getMessageAndSetupScriptValues(inChannel);
        QVariantMap data = inputMessage.getData().toMap();
        QString value = data.value(BaseSlots::TEXT_SLOT().getId()).toString();

        if (passedValues.contains(value)) {
            outChannel->put(inputMessage);
        }
    }
    if (inChannel->isEnded()) {
        done = true;
        outChannel->setEnded();
    }
    return NULL;
}

bool PassFilterWorker::isDone() {
    return done;
}

void PassFilterWorker::cleanup() {
}

/*******************************
 * FilterSequenceWorkerFactory
 *******************************/
void PassFilterWorkerFactory::init() {
    QList<PortDescriptor*> portDescs;
    QList<Attribute*> attrs;

    QMap<Descriptor, DataTypePtr> inTypeMap;
    QMap<Descriptor, DataTypePtr> outTypeMap;
    Descriptor passDesc(BaseSlots::TEXT_SLOT().getId(), PassFilterWorker::tr("Passing values"), PassFilterWorker::tr("Passing values."));
    inTypeMap[passDesc] = BaseTypes::STRING_TYPE();
    outTypeMap[passDesc] = BaseTypes::STRING_TYPE();
    DataTypePtr inTypeSet(new MapDataType(BaseSlots::TEXT_SLOT(), inTypeMap));
    DataTypePtr outTypeSet(new MapDataType(BaseSlots::TEXT_SLOT(), outTypeMap));

    portDescs << new PortDescriptor(BasePorts::IN_TEXT_PORT_ID(), inTypeSet, true);
    portDescs << new PortDescriptor(BasePorts::OUT_TEXT_PORT_ID(), outTypeSet, false);

    Descriptor passVals(BaseSlots::TEXT_SLOT().getId(), PassFilterWorker::tr("Passed values"), PassFilterWorker::tr("Comma separated list of values which are passed by this filter."));
    attrs << new Attribute(passVals, BaseTypes::STRING_TYPE(), true);

    Descriptor protoDesc(PassFilterWorkerFactory::ACTOR_ID,
        PassFilterWorker::tr("Filter"),
        PassFilterWorker::tr("Check an incoming text value if that is contained in passed values list. If it is contained then a data goes on moving through the workflow and vice versa."));

    ActorPrototype *proto = new IntegralBusActorPrototype(protoDesc, portDescs, attrs);
    proto->setEditor(new DelegateEditor(QMap<QString, PropertyDelegate*>()));
    proto->setPrompter(new PassFilterPrompter());

    WorkflowEnv::getProtoRegistry()->registerProto(BaseActorCategories::CATEGORY_DATAFLOW(), proto);
    WorkflowEnv::getDomainRegistry()->getById(LocalDomainFactory::ID)->registerEntry(new PassFilterWorkerFactory());
}

Worker *PassFilterWorkerFactory::createWorker(Actor* a) {
    return new PassFilterWorker(a);
}

/*******************************
 * FilterSequencePrompter
 *******************************/
QString PassFilterPrompter::composeRichDoc() {
    IntegralBusPort* input = qobject_cast<IntegralBusPort*>(target->getPort(BasePorts::IN_TEXT_PORT_ID()));
    Actor* producer = input->getProducer(BaseSlots::TEXT_SLOT().getId());
    QString unsetStr = "<font color='red'>"+tr("unset")+"</font>";
    QString producerName = tr("<u>%1</u>").arg(producer ? producer->getLabel() : unsetStr);
    QString passVals = getRequiredParam(BaseSlots::TEXT_SLOT().getId());
    passVals = getHyperlink(BaseSlots::TEXT_SLOT().getId(), passVals);

    QString res = tr("Filter data in the workflow channel from %1 by passed values: %2").arg(producerName).arg(passVals);
    return res;
}


} // LocalWorkflow
} // U2
