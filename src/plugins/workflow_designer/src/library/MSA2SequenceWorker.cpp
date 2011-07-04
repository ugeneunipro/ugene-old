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

#include "MSA2SequenceWorker.h"

#include <U2Core/AppContext.h>
#include <U2Core/FailTask.h>
#include <U2Lang/CoreLibConstants.h>
#include <U2Lang/BaseSlots.h>
#include <U2Lang/BaseTypes.h>
#include <U2Lang/BasePorts.h>
#include <U2Lang/BaseActorCategories.h>
#include <U2Lang/WorkflowEnv.h>
#include <U2Lang/ActorPrototypeRegistry.h>
#include <U2Designer/DelegateEditors.h>

#include <U2Core/MAlignment.h>
#include <U2Core/MSAUtils.h>

namespace U2 {
namespace LocalWorkflow {

const QString Alignment2SequenceWorkerFactory::ACTOR_ID("convert-alignment-to-sequence");

static const Descriptor ALN_2_SEQUENCE_IN_TYPE_ID("alignment-2-sequence-in-type");
static const Descriptor ALN_2_SEQUENCE_OUT_TYPE_ID("alignment-2-sequence-out-type");

/*******************************
 * Alignment2SequenceWorker
 *******************************/

void Alignment2SequenceWorker::init() {
    input = ports.value(BasePorts::IN_MSA_PORT_ID());
    output = ports.value(BasePorts::OUT_SEQ_PORT_ID());
}

bool Alignment2SequenceWorker::isReady() {
    return input->hasMessage();
}

Task * Alignment2SequenceWorker::tick() {
    Message inputMessage = getMessageAndSetupScriptValues(input);
    MAlignment ma = inputMessage.getData().toMap().value(BaseSlots::MULTIPLE_ALIGNMENT_SLOT().getId()).value<MAlignment>();
    if(ma.isEmpty()) {
        return new FailTask(tr("empty input alignment"));
    }
    QList<DNASequence> seqs = MSAUtils::ma2seq(ma,true);
    foreach(const DNASequence &seq, seqs) {
        QVariantMap msgData;
        msgData[BaseSlots::DNA_SEQUENCE_SLOT().getId()] = qVariantFromValue<DNASequence>(seq);
        output->put(Message(BaseTypes::DNA_SEQUENCE_TYPE(), msgData));
    }
    if(!input->hasMessage()) {
        output->setEnded();
    }
    return NULL;
}

bool Alignment2SequenceWorker::isDone() {
    return input->isEnded();
}

void Alignment2SequenceWorker::cleanup() {
}

/*******************************
 * Alignment2SequenceWorkerFactory
 *******************************/
void Alignment2SequenceWorkerFactory::init() {
    // ports description
    QList<PortDescriptor*> portDescs;
    {
        QMap<Descriptor, DataTypePtr> inM;
        inM[BaseSlots::MULTIPLE_ALIGNMENT_SLOT()] = BaseTypes::MULTIPLE_ALIGNMENT_TYPE();
        DataTypePtr inSet(new MapDataType(ALN_2_SEQUENCE_IN_TYPE_ID, inM));
        Descriptor inPortDesc(BasePorts::IN_MSA_PORT_ID(), Alignment2SequenceWorker::tr("Input alignment"), 
            Alignment2SequenceWorker::tr("A alignment which will be split into sequences"));
        portDescs << new PortDescriptor(inPortDesc, inSet, true);
        
        QMap<Descriptor, DataTypePtr> outM;
        outM[BaseSlots::DNA_SEQUENCE_SLOT()] = BaseTypes::DNA_SEQUENCE_TYPE();
        DataTypePtr outSet(new MapDataType(ALN_2_SEQUENCE_OUT_TYPE_ID, outM));
        Descriptor outPortDesc(BasePorts::OUT_SEQ_PORT_ID(), Alignment2SequenceWorker::tr("Output sequences"), 
            Alignment2SequenceWorker::tr("Converted sequences"));
        portDescs << new PortDescriptor(outPortDesc, outSet, false);
    }

    QList<Attribute*> attrs;
   
    Descriptor protoDesc(Alignment2SequenceWorkerFactory::ACTOR_ID, 
        Alignment2SequenceWorker::tr("Split alignment into sequences"), 
        Alignment2SequenceWorker::tr("Splits input alignment into sequences"));
    ActorPrototype * proto = new IntegralBusActorPrototype(protoDesc, portDescs, attrs);
    
    proto->setPrompter(new Alignment2SequencePrompter());
    
    WorkflowEnv::getProtoRegistry()->registerProto(BaseActorCategories::CATEGORY_ALIGNMENT(), proto);
    WorkflowEnv::getDomainRegistry()->getById( LocalDomainFactory::ID )->registerEntry( new Alignment2SequenceWorkerFactory() );
}

Worker * Alignment2SequenceWorkerFactory::createWorker(Actor* a) {
    return new Alignment2SequenceWorker(a);
}

/*******************************
 * Alignment2SequencePrompter
 *******************************/
QString Alignment2SequencePrompter::composeRichDoc() {
    QString unsetStr = "<font color='red'>"+tr("unset")+"</font>";
    IntegralBusPort* input = qobject_cast<IntegralBusPort*>(target->getPort(BasePorts::IN_MSA_PORT_ID()));
    Actor* producer = input->getProducer(BaseSlots::MULTIPLE_ALIGNMENT_SLOT().getId());
    QString source = tr(" from <u>%1</u>").arg(producer ? producer->getLabel() : unsetStr); 
    return tr("Split alignment%1 into sequences").arg(source);
}

} // LocalWorkflow
} // U2
