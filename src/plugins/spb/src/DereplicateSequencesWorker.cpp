#include <U2Core/FailTask.h>
#include <U2Core/Timer.h>

#include <U2Designer/DelegateEditors.h>

#include <U2Lang/ActorPrototypeRegistry.h>
#include <U2Lang/BasePorts.h>
#include <U2Lang/BaseSlots.h>
#include <U2Lang/BaseTypes.h>
#include <U2Lang/WorkflowEnv.h>

#include "ComparingAlgorithm.h"
#include "DereplicationTask.h"
#include "SpbPlugin.h"

#include "DereplicateSequencesWorker.h"

namespace SPB {

const QString DereplicateSequencesWorkerFactory::ACTOR_ID("dereplicate-sequences");

static const QString OUT_PORT_ID("dereplicated-sequences");

static const QString COMPARING_ALGO_ATTR_ID("comparing-algorithm");
static const QString ACCURACY_ATTR_ID("comparing-accuracy");
static const QString DEREPLICATION_ALGO_ATTR_ID("dereplication-algorithm");

/************************************************************************/
/* Worker */
/************************************************************************/
DereplicateSequencesWorker::DereplicateSequencesWorker(Actor *a)
: BaseWorker(a, false), inPort(NULL), outPort(NULL), accuracy(0), taskInRunning(false)
{

}

void DereplicateSequencesWorker::init() {
    inPort = ports.value(BasePorts::IN_SEQ_PORT_ID());
    outPort = ports.value(OUT_PORT_ID);

    derepAlgoId = actor->getParameter(DEREPLICATION_ALGO_ATTR_ID)->getAttributeValue<QString>(context);
    compAlgoId = actor->getParameter(COMPARING_ALGO_ATTR_ID)->getAttributeValue<QString>(context);
    accuracy = actor->getParameter(ACCURACY_ATTR_ID)->getAttributeValue<double>(context);
}

bool DereplicateSequencesWorker::isReady() {
    return BaseWorker::isReady() && !taskInRunning;
}

Task * DereplicateSequencesWorker::tick() {
    while (inPort->hasMessage()) {
        Message m = getMessageAndSetupScriptValues(inPort);
        QVariantMap data = m.getData().toMap();
        SharedDbiDataHandler seqId = data.value(BaseSlots::DNA_SEQUENCE_SLOT().getId()).value<SharedDbiDataHandler>();
        sequences << seqId;
    }
    if (inPort->isEnded()) {
        taskInRunning = true;
        DereplicationData data(sequences, context->getDataStorage(), compAlgoId, accuracy);
        Task *t = DereplicationTaskFactory::createTask(derepAlgoId, data);
        if (NULL == t) {
            return new FailTask(tr("Unknown dereplication algorithm: %1").arg(derepAlgoId));
        }
        connect(t, SIGNAL(si_stateChanged()), SLOT(sl_taskFinished()));
        return t;
    }

    return NULL;
}

void DereplicateSequencesWorker::sl_taskFinished() {
    DereplicationTask *t = dynamic_cast<DereplicationTask*>(sender());
    if (!t->isFinished() || t->hasError()) {
        return;
    }

    quint64 timeMicSec = GTimer::currentTimeMicros() - t->getTimeInfo().startTime;
    quint64 time = timeMicSec / (1000*1000);
    taskLog.details(tr("Dereplication task time: %1 sec").arg(time));

    foreach (const SharedDbiDataHandler &seqId, t->takeResult()) {
        QVariantMap data;
        data[BaseSlots::DNA_SEQUENCE_SLOT().getId()] = qVariantFromValue<SharedDbiDataHandler>(seqId);
        outPort->put(Message(outPort->getBusType(), data));
    }

    setDone();
    outPort->setEnded();
    taskInRunning = false;
}

void DereplicateSequencesWorker::cleanup() {
    sequences.clear();
}

/************************************************************************/
/* Factory */
/************************************************************************/
void DereplicateSequencesWorkerFactory::init() {
    QList<PortDescriptor*> portDescs;
    {
        QMap<Descriptor, DataTypePtr> inMap;
        inMap[BaseSlots::DNA_SEQUENCE_SLOT()] = BaseTypes::DNA_SEQUENCE_TYPE();
        portDescs << new PortDescriptor(BasePorts::IN_SEQ_PORT_ID(), DataTypePtr(new MapDataType("in.sequences", inMap)),
            true, false, IntegralBusPort::BLIND_INPUT);

        QMap<Descriptor, DataTypePtr> outMap;
        outMap[BaseSlots::DNA_SEQUENCE_SLOT()] = BaseTypes::DNA_SEQUENCE_TYPE();
        portDescs << new PortDescriptor(OUT_PORT_ID, DataTypePtr(new MapDataType("out.sequences", outMap)), false, true);
    }

    QList<Attribute*> attrs;
    {
        Descriptor dAlgoD(DEREPLICATION_ALGO_ATTR_ID,
            DereplicateSequencesWorker::tr("Dereplication algorithm"),
            DereplicateSequencesWorker::tr("Dereplication algorithm."));
        Descriptor cAlgoD(COMPARING_ALGO_ATTR_ID,
            DereplicateSequencesWorker::tr("Sequence comparing algorithm"),
            DereplicateSequencesWorker::tr("Sequence comparing algorithm."));
        Descriptor accD(ACCURACY_ATTR_ID,
            DereplicateSequencesWorker::tr("Comparing accuracy"),
            DereplicateSequencesWorker::tr("Comparing accuracy."));

        attrs << new Attribute(dAlgoD, BaseTypes::STRING_TYPE(), true, "Random");
        attrs << new Attribute(cAlgoD, BaseTypes::STRING_TYPE(), true, ComparingAlgorithmFactory::DEFAULT);
        attrs << new Attribute(accD, BaseTypes::NUM_TYPE(), true, 99);
    }

    QMap<QString, PropertyDelegate*> delegates;
    {
        QVariantMap percMap;
        percMap["minimum"] = QVariant(0.0); 
        percMap["maximum"] = QVariant(100.0); 
        percMap["suffix"] = "%";
        delegates[ACCURACY_ATTR_ID] = new DoubleSpinBoxDelegate(percMap);

        QVariantMap cAlgos;
        cAlgos[ComparingAlgorithmFactory::DEFAULT] = ComparingAlgorithmFactory::DEFAULT;
        delegates[COMPARING_ALGO_ATTR_ID] = new ComboBoxDelegate(cAlgos);

        QVariantMap dAlgos;
        dAlgos["Random"] = "Random";
        delegates[DEREPLICATION_ALGO_ATTR_ID] = new ComboBoxDelegate(dAlgos);
    }

    Descriptor protoD(ACTOR_ID,
        DereplicateSequencesWorker::tr("Dereplicate sequences"),
        DereplicateSequencesWorker::tr("Dereplicate sequences."));

    ActorPrototype *proto = new IntegralBusActorPrototype(protoD, portDescs, attrs);
    proto->setEditor(new DelegateEditor(delegates));
    proto->setPrompter(new DereplicateSequencesPrompter());

    WorkflowEnv::getProtoRegistry()->registerProto(Constraints::WORKFLOW_CATEGORY, proto);
    DomainFactory *localDomain = WorkflowEnv::getDomainRegistry()->getById(LocalDomainFactory::ID);
    localDomain->registerEntry(new DereplicateSequencesWorkerFactory());
}

Worker * DereplicateSequencesWorkerFactory::createWorker(Actor *a) {
    return new DereplicateSequencesWorker(a);
}

/************************************************************************/
/* Prompter */
/************************************************************************/
QString DereplicateSequencesPrompter::composeRichDoc() {
    return "";
}

} // SPB
