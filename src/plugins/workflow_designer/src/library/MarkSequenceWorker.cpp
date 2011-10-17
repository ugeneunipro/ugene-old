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
#include <U2Designer/MarkerEditor.h>

#include <U2Lang/ActorPrototypeRegistry.h>
#include <U2Lang/BaseActorCategories.h>
#include <U2Lang/BaseAttributes.h>
#include <U2Lang/BasePorts.h>
#include <U2Lang/BaseSlots.h>
#include <U2Lang/BaseTypes.h>
#include <U2Lang/Marker.h>
#include <U2Lang/MarkerAttribute.h>
#include <U2Lang/WorkflowEnv.h>
#include <U2Core/Log.h>

#include <U2Core/QVariantUtils.h>
#include <U2Core/DNASequence.h>

#include "MarkSequenceWorker.h"

namespace U2 {
namespace LocalWorkflow {

const QString MarkSequenceWorkerFactory::ACTOR_ID("mark-sequence");
static const QString SEQ_TYPESET_ID("seq.content");
static const QString MARKED_SEQ_TYPESET_ID("marker.seq.content");

/*******************************
 * MarkSequenceWorker
 *******************************/
MarkSequenceWorker::MarkSequenceWorker(Actor *p)
: BaseWorker(p), inChannel(NULL), outChannel(NULL), done(false)
{
}

void MarkSequenceWorker::init() {
    inChannel = ports.value(BasePorts::IN_SEQ_PORT_ID());
    outChannel = ports.value(MarkerPorts::OUT_MARKER_SEQ_PORT());
    mtype = ports.value(MarkerPorts::OUT_MARKER_SEQ_PORT())->getBusType();
}

bool MarkSequenceWorker::isReady() {
    int hasMsg = inChannel->hasMessage();
    bool ended = inChannel->isEnded();
    return hasMsg || (ended && !done);
}

Task *MarkSequenceWorker::tick() {
    while (inChannel->hasMessage()) {
        Message inputMessage = getMessageAndSetupScriptValues(inChannel);
        QVariantMap data = inputMessage.getData().toMap();
        U2DataId seqId = data.value(BaseSlots::DNA_SEQUENCE_SLOT().getId()).value<U2DataId>();
        std::auto_ptr<U2SequenceObject> seqObj(StorageUtils::getSequenceObject(context->getDataStorage(), seqId));
        if (NULL == seqObj.get()) {
            return NULL;
        }
        DNASequence seq = seqObj->getWholeSequence();
        QList<SharedAnnotationData> anns = QVariantUtils::var2ftl(data.value(BaseSlots::ANNOTATION_TABLE_SLOT().getId()).toList());

        MarkerAttribute *attr = dynamic_cast<MarkerAttribute*>(actor->getParameter("markers"));
        QVariantMap m;
        foreach (QString markerId, attr->getMarkers().keys()) {
            Marker *marker = attr->getMarkers().value(markerId);

            QString res;
            if (SEQUENCE == marker->getGroup()) {
                res = marker->getMarkingResult(qVariantFromValue<DNASequence>(seq));
            } else if (QUALIFIER == marker->getGroup() || ANNOTATION == marker->getGroup()) {
                res = marker->getMarkingResult(data.value(BaseSlots::ANNOTATION_TABLE_SLOT().getId()));
            } else if (TEXT == marker->getGroup()) {
                res = marker->getMarkingResult(data.value(BaseSlots::URL_SLOT().getId()));
            }
            m.insert(marker->getName(), QVariant(res));
        }
        Message mes(mtype, m);
        outChannel->put(mes);
    }
    if (inChannel->isEnded()) {
        done = true;
        outChannel->setEnded();
    }
    return NULL;
}

bool MarkSequenceWorker::isDone() {
    return done;
}

void MarkSequenceWorker::cleanup() {
}

/*******************************
 * MarkSequenceWorkerFactory
 *******************************/
void MarkSequenceWorkerFactory::init() {
    QList<PortDescriptor*> portDescs;
    QList<Attribute*> attrs;

    // input port
    QMap<Descriptor, DataTypePtr> inTypeMap;
    Descriptor urlD(BaseSlots::URL_SLOT().getId(), MarkSequenceWorker::tr("Location"), MarkSequenceWorker::tr("Location of sequence"));
    inTypeMap[urlD] = BaseTypes::STRING_TYPE();
    inTypeMap[BaseSlots::DNA_SEQUENCE_SLOT()] = BaseTypes::DNA_SEQUENCE_TYPE();
    inTypeMap[BaseSlots::ANNOTATION_TABLE_SLOT()] = BaseTypes::ANNOTATION_TABLE_LIST_TYPE();
    DataTypePtr inTypeSet(new MapDataType(Descriptor(SEQ_TYPESET_ID), inTypeMap));

    Descriptor inPd(BasePorts::IN_SEQ_PORT_ID(), MarkSequenceWorker::tr("Sequence"), MarkSequenceWorker::tr("Sequence"));
    portDescs << new PortDescriptor(inPd, inTypeSet, true);

    // output port
    QMap<Descriptor, DataTypePtr> outTypeMap;
    DataTypePtr outTypeSet(new MapDataType(Descriptor(MARKED_SEQ_TYPESET_ID), outTypeMap));

    Descriptor outPd(MarkerPorts::OUT_MARKER_SEQ_PORT(), MarkSequenceWorker::tr("Marked sequence"), MarkSequenceWorker::tr("Marked sequence"));
    portDescs << new PortDescriptor(outPd, outTypeSet, false, true);

   
    Descriptor protoDesc(MarkSequenceWorkerFactory::ACTOR_ID,
        MarkSequenceWorker::tr("Marker (for sequence)"),
        MarkSequenceWorker::tr("Check an incoming sequence using user defined criterias and mark the sequence by the appropriate value for every criteria."));
    attrs << new MarkerAttribute(Descriptor("markers", MarkSequenceWorker::tr("Markers"), MarkSequenceWorker::tr("Markers.")), BaseTypes::STRING_TYPE(), false);

    ActorPrototype *proto = new IntegralBusActorPrototype(protoDesc, portDescs, attrs);

    proto->setEditor(new MarkerEditor());
    proto->setPrompter(new MarkSequencePrompter());
    
    WorkflowEnv::getProtoRegistry()->registerProto(BaseActorCategories::CATEGORY_DATAFLOW(), proto);
    WorkflowEnv::getDomainRegistry()->getById(LocalDomainFactory::ID)->registerEntry(new MarkSequenceWorkerFactory());
}

Worker *MarkSequenceWorkerFactory::createWorker(Actor* a) {
    return new MarkSequenceWorker(a);
}

/*******************************
 * MarkSequencePrompter
 *******************************/
QString MarkSequencePrompter::composeRichDoc() {
    return tr("Mark incoming sequences by user defined values");
}


} // LocalWorkflow
} // U2
