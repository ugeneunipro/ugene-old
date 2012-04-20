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

#include "SequencesToMSAWorker.h"

#include <U2Core/TaskSignalMapper.h>
#include <U2Core/FailTask.h>
#include <U2Designer/DelegateEditors.h>
#include <U2Lang/WorkflowEnv.h>
#include <U2Lang/ActorPrototypeRegistry.h>
#include <U2Lang/BaseSlots.h>
#include <U2Lang/BaseTypes.h>
#include <U2Lang/BasePorts.h>
#include <U2Lang/BaseActorCategories.h>


namespace U2 {
namespace LocalWorkflow {

QString SequencesToMSAPromter::composeRichDoc() {
    return tr("Creates multiple sequence alignment from supplied sequences.");
}

void SequencesToMSAWorker::init() {
    inPort = ports.value(BasePorts::IN_SEQ_PORT_ID());
    outPort = ports.value(BasePorts::OUT_MSA_PORT_ID());
}

bool SequencesToMSAWorker::isReady() {
    return inPort->hasMessage();
}

bool SequencesToMSAWorker::isDone() {
    return outPort->isEnded();;
}

void SequencesToMSAWorker::cleanup() {}

Task* SequencesToMSAWorker::tick() {
    Message inputMessage = getMessageAndSetupScriptValues(inPort);
    QVariantMap qm = inputMessage.getData().toMap();
    SharedDbiDataHandler seqId = qm.value(BaseSlots::DNA_SEQUENCE_SLOT().getId()).value<SharedDbiDataHandler>();
    std::auto_ptr<U2SequenceObject> seqObj(StorageUtils::getSequenceObject(context->getDataStorage(), seqId));
    if (NULL == seqObj.get()) {
        return NULL;
    }
    DNASequence seq = seqObj->getWholeSequence();
    data.append(seq);
    if (inPort->isEnded()) {
        Task* t = new MSAFromSequencesTask(data);
        connect(new TaskSignalMapper(t), SIGNAL(si_taskFinished(Task*)), SLOT(sl_onTaskFinished(Task*)));
        return t;
    }
    return NULL;
}

void MSAFromSequencesTask::run() {
    DNASequence seq = sequences_.first();
    ma.setAlphabet(seq.alphabet);
    ma.addRow( MAlignmentRow(seq.getName(),seq.seq) );
    for (int i=1; i<sequences_.size(); i++) {
        DNASequence sqnc = sequences_.at(i);
        ma.addRow( MAlignmentRow(sqnc.getName(),sqnc.seq) );
    }
}

void SequencesToMSAWorker::sl_onTaskFinished(Task* t) {
    MSAFromSequencesTask* maTask = qobject_cast<MSAFromSequencesTask*>(t);
    MAlignment ma = maTask->getResult();

    outPort->put( Message(BaseTypes::MULTIPLE_ALIGNMENT_TYPE(), qVariantFromValue(ma)) );

    if (inPort->isEnded()) {
        outPort->setEnded();
    }
}

const QString SequencesToMSAWorkerFactory::ACTOR_ID("sequences-to-msa");

void SequencesToMSAWorkerFactory::init() {
    QList<PortDescriptor*> p; QList<Attribute*> a;
    {
        Descriptor id(BasePorts::IN_SEQ_PORT_ID(),
            SequencesToMSAWorker::tr("Input sequences"), 
            SequencesToMSAWorker::tr("Sequences to be joined into alignment."));

        Descriptor od(BasePorts::OUT_MSA_PORT_ID(),
            SequencesToMSAWorker::tr("Result alignment"), 
            SequencesToMSAWorker::tr("Alignment created from the given sequences."));

        QMap<Descriptor, DataTypePtr> inM;
        inM[BaseSlots::DNA_SEQUENCE_SLOT()] = BaseTypes::DNA_SEQUENCE_TYPE();
        p << new PortDescriptor(id, DataTypePtr(new MapDataType("seq2msa.seq", inM)), true /*input*/);

        QMap<Descriptor, DataTypePtr> outM;
        outM[BaseSlots::MULTIPLE_ALIGNMENT_SLOT()] = BaseTypes::MULTIPLE_ALIGNMENT_TYPE();
        p << new PortDescriptor(od, DataTypePtr(new MapDataType("seq2msa.msa", outM)), false /*input*/, true /*multi*/);
    }

    Descriptor desc( SequencesToMSAWorkerFactory::ACTOR_ID, 
        SequencesToMSAWorker::tr("Join Sequences into Alignment"), 
        SequencesToMSAWorker::tr("Creates multiple sequence alignment from sequences.") );
    ActorPrototype * proto = new IntegralBusActorPrototype( desc, p, QList<Attribute*>() );

    proto->setEditor(new DelegateEditor(QMap<QString, PropertyDelegate*>()));
    proto->setPrompter( new SequencesToMSAPromter() );
    WorkflowEnv::getProtoRegistry()->registerProto( BaseActorCategories::CATEGORY_ALIGNMENT(), proto );

    DomainFactory* localDomain = WorkflowEnv::getDomainRegistry()->getById( LocalDomainFactory::ID );
    localDomain->registerEntry( new SequencesToMSAWorkerFactory() );
}

} //LocalWorkflow namespace
} //U2 namespace
