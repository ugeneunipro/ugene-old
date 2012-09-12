#include <U2Designer/DelegateEditors.h>

#include <U2Lang/ActorPrototypeRegistry.h>
#include <U2Lang/BaseTypes.h>
#include <U2Lang/WorkflowEnv.h>

#include "ComparingAlgorithm.h"
#include "SpbPlugin.h"

#include "FilterSequencesWorker.h"

namespace SPB {

const QString FilterSequencesWorkerFactory::ACTOR_ID("filter-sequences");
static const QString IN_SEQ_1_SLOT_ID("input-seq-1");
static const QString IN_SEQ_2_SLOT_ID("input-seq-2");
static const QString FILTERED_SEQ_SLOT_ID("filtered-seq");

static const QString IN_PORT_1_ID("in-data-1");
static const QString IN_PORT_2_ID("in-data-2");
static const QString OUT_PORT_ID("filtered-data");

static const QString ALGO_ATTR_ID("comparing-algorithm");
static const QString ACCURACY_ATTR_ID("comparing-accuracy");

/************************************************************************/
/* Worker */
/************************************************************************/
FilterSequencesWorker::FilterSequencesWorker(Actor *a)
: BaseWorker(a, false)
{

}

void FilterSequencesWorker::init() {
    inPort1 = ports.value(IN_PORT_1_ID);
    inPort2 = ports.value(IN_PORT_2_ID);
    outPort = ports.value(OUT_PORT_ID);

    inPort1->addComplement(outPort);
    outPort->addComplement(inPort1);
}

bool FilterSequencesWorker::isReady() {
    if (isDone()) {
        return false;
    }
    bool in1Ended = inPort1->isEnded();
    bool in2Ended = inPort2->isEnded();
    int in1HasMes = inPort1->hasMessage();
    int in2HasMes = inPort2->hasMessage();
    return in2HasMes || (in2Ended && (in1HasMes || in1Ended));
}

Task * FilterSequencesWorker::tick() {
    while (inPort2->hasMessage()) {
        Message m = getMessageAndSetupScriptValues(inPort2);
        QVariantMap data = m.getData().toMap();
        SharedDbiDataHandler seqId = data.value(IN_SEQ_2_SLOT_ID).value<SharedDbiDataHandler>();
        sequencesToFind << seqId;
    }
    if (!inPort2->isEnded()) {
        return NULL;
    }

    if (inPort1->hasMessage()) {
        Message m = getMessageAndSetupScriptValues(inPort1);
        QVariantMap data = m.getData().toMap();
        SharedDbiDataHandler seqId = data.value(IN_SEQ_1_SLOT_ID).value<SharedDbiDataHandler>();

        double accuracy = actor->getParameter(ACCURACY_ATTR_ID)->getAttributeValue<double>(context);
        QString algoId = actor->getParameter(ALGO_ATTR_ID)->getAttributeValue<QString>(context);

        Task *t = new FilterSequenceTask(seqId, sequencesToFind, accuracy, algoId, context);
        connect(t, SIGNAL(si_stateChanged()), SLOT(sl_taskFinished()));
        return t;
    } else if (inPort1->isEnded()) {
        setDone();
        outPort->setEnded();
    }

    return NULL;
}

void FilterSequencesWorker::cleanup() {
    sequencesToFind.clear();
}

void FilterSequencesWorker::sl_taskFinished() {
    FilterSequenceTask *t = dynamic_cast<FilterSequenceTask*>(sender());
    if (!t->isFinished() || t->hasError()) {
        return;
    }

    if (!t->isFiltered()) {
        outPort->put(Message(outPort->getBusType(), QVariantMap()));
    }
}

/************************************************************************/
/* Task */
/************************************************************************/
FilterSequenceTask::FilterSequenceTask(const SharedDbiDataHandler &_srcSeq,
                  const QList<SharedDbiDataHandler> &_sequencesToFind,
                  double _accuracy, const QString &_algoId, WorkflowContext *_ctx)
: Task("Filter sequence", TaskFlag_None), srcSeq(_srcSeq),
sequencesToFind(_sequencesToFind), accuracy(_accuracy), algoId(_algoId),
ctx(_ctx), result(false)
{

}

bool FilterSequenceTask::isFiltered() const {
    return result;
}

void FilterSequenceTask::run() {
    QScopedPointer<ComparingAlgorithm> algo(ComparingAlgorithmFactory::createAlgorithm(algoId));
    if (NULL == algo.data()) {
        stateInfo.setError(tr("Unknown comparing algorithm").arg(algoId));
        cleanup();
        return;
    }

    QScopedPointer<U2SequenceObject> srcSeqObj(
        StorageUtils::getSequenceObject(ctx->getDataStorage(), srcSeq));
    if (NULL == srcSeqObj.data()) {
        stateInfo.setError(tr("SPB: NULL sequence object"));
        cleanup();
        return;
    }

    foreach (const SharedDbiDataHandler &seqId, sequencesToFind) {
        QScopedPointer<U2SequenceObject> seqObj(
            StorageUtils::getSequenceObject(ctx->getDataStorage(), seqId));
        if (NULL == seqObj.data()) {
            taskLog.error("SPB: NULL sequence object");
            continue;
        }
        double res = algo->compare(srcSeqObj.data(), seqObj.data());
        if (res >= accuracy) { // sequences are equal
            result = true;
            break;
        }
    }
    cleanup();
}

void FilterSequenceTask::cleanup() {
    sequencesToFind.clear();
}

/************************************************************************/
/* Factory */
/************************************************************************/
void FilterSequencesWorkerFactory::init() {
    QList<PortDescriptor*> portDescs;
    {
        Descriptor inSeq1D(IN_SEQ_1_SLOT_ID,
            FilterSequencesWorker::tr("Input sequences 1"), 
            FilterSequencesWorker::tr("Input sequences 1"));
        QMap<Descriptor, DataTypePtr> inMap1;
        inMap1[inSeq1D] = BaseTypes::DNA_SEQUENCE_TYPE();
        portDescs << new PortDescriptor(IN_PORT_1_ID, DataTypePtr(new MapDataType("in.sequences.1", inMap1)), true);


        Descriptor inSeq2D(IN_SEQ_2_SLOT_ID,
            FilterSequencesWorker::tr("Input sequences 2"), 
            FilterSequencesWorker::tr("Input sequences 2"));
        QMap<Descriptor, DataTypePtr> inMap2;
        inMap2[inSeq2D] = BaseTypes::DNA_SEQUENCE_TYPE();
        portDescs << new PortDescriptor(IN_PORT_2_ID, DataTypePtr(new MapDataType("in.sequences.2", inMap2)), true,
            false, IntegralBusPort::BLIND_INPUT);

        Descriptor outSeqD(FILTERED_SEQ_SLOT_ID,
            FilterSequencesWorker::tr("Filtered sequences"), 
            FilterSequencesWorker::tr("Filtered sequences"));
        QMap<Descriptor, DataTypePtr> outMap;
        portDescs << new PortDescriptor(OUT_PORT_ID, DataTypePtr(new MapDataType("out.sequences", outMap)), false, true);
    }

    QList<Attribute*> attrs;
    {
        Descriptor algoD(ALGO_ATTR_ID,
            FilterSequencesWorker::tr("Sequence comparing algorithm"),
            FilterSequencesWorker::tr("Sequence comparing algorithm."));
        Descriptor accD(ACCURACY_ATTR_ID,
            FilterSequencesWorker::tr("Comparing accuracy"),
            FilterSequencesWorker::tr("Comparing accuracy."));

        attrs << new Attribute(algoD, BaseTypes::STRING_TYPE(), true, ComparingAlgorithmFactory::DEFAULT);
        attrs << new Attribute(accD, BaseTypes::NUM_TYPE(), true, 99);
    }

    QMap<QString, PropertyDelegate*> delegates;
    {
        QVariantMap percMap;
        percMap["minimum"] = QVariant(0.0); 
        percMap["maximum"] = QVariant(100.0); 
        percMap["suffix"] = "%";
        delegates[ACCURACY_ATTR_ID] = new DoubleSpinBoxDelegate(percMap);

        QVariantMap algos;
        algos[ComparingAlgorithmFactory::DEFAULT] = ComparingAlgorithmFactory::DEFAULT;
        delegates[ALGO_ATTR_ID] = new ComboBoxDelegate(algos);
    }

    Descriptor protoD(ACTOR_ID,
        FilterSequencesWorker::tr("Filter sequences"),
        FilterSequencesWorker::tr("Filter sequences."));

    ActorPrototype *proto = new IntegralBusActorPrototype(protoD, portDescs, attrs);
    proto->setEditor(new DelegateEditor(delegates));
    proto->setPrompter(new FilterSequencesPrompter());
    proto->setInfluenceOnPathFlag(true);

    WorkflowEnv::getProtoRegistry()->registerProto(Constraints::WORKFLOW_CATEGORY, proto);
    DomainFactory *localDomain = WorkflowEnv::getDomainRegistry()->getById(LocalDomainFactory::ID);
    localDomain->registerEntry(new FilterSequencesWorkerFactory());
}

Worker * FilterSequencesWorkerFactory::createWorker(Actor *a) {
    return new FilterSequencesWorker(a);
}

/************************************************************************/
/* Prompter */
/************************************************************************/
QString FilterSequencesPrompter::composeRichDoc() {
    return FilterSequencesWorker::tr("Filters sequences from the 1st sequences channel"
        " by sequences from 2nd channel using comparing algorithm and accuracy");
}

} // SPB
