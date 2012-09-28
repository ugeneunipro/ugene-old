#include <U2Core/Timer.h>
#include <U2Core/U2SafePoints.h>

#include <U2Designer/DelegateEditors.h>

#include <U2Lang/ActorPrototypeRegistry.h>
#include <U2Lang/BasePorts.h>
#include <U2Lang/BaseSlots.h>
#include <U2Lang/BaseTypes.h>
#include <U2Lang/WorkflowEnv.h>

#include "SpbPlugin.h"

#include "RandomFilterWorker.h"

using namespace U2::LocalWorkflow;

namespace SPB {

const QString RandomFilterWorkerFactory::ACTOR_ID("random-filter");

static const QString OUT_PORT_ID("filtered-sequences");

/******************************************
 * Worker
 *****************************************/
RandomFilterWorker::RandomFilterWorker(Actor *a)
    : BaseWorker(a), inPort(NULL), outPort(NULL)
{

}

void RandomFilterWorker::init() {
    inPort = ports.value(BasePorts::IN_SEQ_PORT_ID());
    outPort = ports.value(OUT_PORT_ID);
    qsrand(QDateTime::currentDateTime().toTime_t());
}

Task * RandomFilterWorker::tick() {
    while (inPort->hasMessage()) {
        Message m = getMessageAndSetupScriptValues(inPort);
        addSequence(m.getData().toMap());
    }
    if (inPort->isEnded()) {
        sendData();
        setDone();
        outPort->setEnded();
    }

    return NULL;
}

void RandomFilterWorker::addSequence(const QVariantMap &data) {
    CHECK(data.contains(BaseSlots::DNA_SEQUENCE_SLOT().getId()), );
    CHECK(data.contains(BaseSlots::URL_SLOT().getId()), );

    QString url = data[BaseSlots::URL_SLOT().getId()].toString();
    SharedDbiDataHandler seqId = data[BaseSlots::DNA_SEQUENCE_SLOT().getId()].value<SharedDbiDataHandler>();

    sequences[url] << seqId;
}

void RandomFilterWorker::sendData() {
    int size = getMinSize();

    foreach (const QString &url, sequences.keys()) {
        QList<SharedDbiDataHandler> &seqs = sequences[url];
        int iter = size;
        while (iter > 0) {
            iter--;
            QVariantMap data;
            data[BaseSlots::DNA_SEQUENCE_SLOT().getId()] = qVariantFromValue<SharedDbiDataHandler>(takeRandomSequence(seqs));
            outPort->put(Message(outPort->getBusType(), data));
        }
    }
}

SharedDbiDataHandler RandomFilterWorker::takeRandomSequence(QList<SharedDbiDataHandler> &seqs) {
    int idx = qrand()%seqs.size();
    return seqs.takeAt(idx);
}

int RandomFilterWorker::getMinSize() {
    int min = INT_MAX;
    foreach (const QList<SharedDbiDataHandler> &seqs, sequences.values()) {
        if (min > seqs.size()) {
            min = seqs.size();
        }
    }
    return min;
}

void RandomFilterWorker::cleanup() {
    sequences.clear();
}

/******************************************
 * Factory
 *****************************************/
void RandomFilterWorkerFactory::init() {
    QList<PortDescriptor*> portDescs;
    {
        QMap<Descriptor, DataTypePtr> inMap;
        inMap[BaseSlots::DNA_SEQUENCE_SLOT()] = BaseTypes::DNA_SEQUENCE_TYPE();
        inMap[BaseSlots::URL_SLOT()] = BaseTypes::STRING_TYPE();
        portDescs << new PortDescriptor(BasePorts::IN_SEQ_PORT_ID(), DataTypePtr(new MapDataType("in.sequences", inMap)),
            true, false, IntegralBusPort::BLIND_INPUT);

        QMap<Descriptor, DataTypePtr> outMap;
        outMap[BaseSlots::DNA_SEQUENCE_SLOT()] = BaseTypes::DNA_SEQUENCE_TYPE();
        portDescs << new PortDescriptor(OUT_PORT_ID, DataTypePtr(new MapDataType("out.sequences", outMap)), false, true);
    }

    QList<Attribute*> attrs;
    QMap<QString, PropertyDelegate*> delegates;

    Descriptor protoD(ACTOR_ID,
        RandomFilterWorker::tr("Random seuquence filter"),
        RandomFilterWorker::tr("Random seuquence filter."));

    ActorPrototype *proto = new IntegralBusActorPrototype(protoD, portDescs, attrs);
    proto->setEditor(new DelegateEditor(delegates));
    proto->setPrompter(new RandomFilterPrompter());

    WorkflowEnv::getProtoRegistry()->registerProto(Constraints::WORKFLOW_CATEGORY, proto);
    DomainFactory *localDomain = WorkflowEnv::getDomainRegistry()->getById(LocalDomainFactory::ID);
    localDomain->registerEntry(new RandomFilterWorkerFactory());
}

Worker * RandomFilterWorkerFactory::createWorker(Actor *a) {
    return new RandomFilterWorker(a);
}

/******************************************
 * Prompter
 *****************************************/
QString RandomFilterPrompter::composeRichDoc() {
    return "";
}

} // SPB
