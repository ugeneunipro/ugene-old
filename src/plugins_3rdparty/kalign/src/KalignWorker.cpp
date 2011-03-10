#include "KalignWorker.h"
#include "KalignConstants.h"
#include "TaskLocalStorage.h"
#include "KalignDialogController.h"

#include <U2Lang/IntegralBusModel.h>
#include <U2Lang/WorkflowEnv.h>
#include <U2Lang/ActorPrototypeRegistry.h>
#include <U2Lang/BaseTypes.h>
#include <U2Lang/BaseSlots.h>
#include <U2Lang/BasePorts.h>
#include <U2Lang/BaseActorCategories.h>
#include <U2Designer/DelegateEditors.h>
#include <U2Lang/CoreLibConstants.h>
#include <U2Core/Log.h>

/* TRANSLATOR U2::LocalWorkflow::KalignWorker */

namespace U2 {
namespace LocalWorkflow {

/****************************
 * KalignWorkerFactory
 ****************************/
const QString KalignWorkerFactory::ACTOR_ID("kalign");
const QString GAP_OPEN_PENALTY("gap-open-penalty");
const QString GAP_EXT_PENALTY("gap-ext-penalty");
const QString TERM_GAP_PENALTY("terminal-gap-penalty");
const QString BONUS_SCORE("bonus-score");

void KalignWorkerFactory::init() {
    QList<PortDescriptor*> p; QList<Attribute*> a;
    Descriptor ind(BasePorts::IN_MSA_PORT_ID(), KalignWorker::tr("Input MSA"), KalignWorker::tr("Input MSA to process."));
    Descriptor oud(BasePorts::OUT_MSA_PORT_ID(), KalignWorker::tr("Kalign result MSA"), 
        KalignWorker::tr("The result of the Kalign alignment."));
    
    QMap<Descriptor, DataTypePtr> inM;
    inM[BaseSlots::MULTIPLE_ALIGNMENT_SLOT()] = BaseTypes::MULTIPLE_ALIGNMENT_TYPE();
    p << new PortDescriptor(ind, DataTypePtr(new MapDataType("kalign.in.msa", inM)), true /*input*/);
    QMap<Descriptor, DataTypePtr> outM;
    outM[BaseSlots::MULTIPLE_ALIGNMENT_SLOT()] = BaseTypes::MULTIPLE_ALIGNMENT_TYPE();
    p << new PortDescriptor(oud, DataTypePtr(new MapDataType("kalign.out.msa", outM)), false /*input*/, true /*multi*/);
    
    Descriptor gop(GAP_OPEN_PENALTY, KalignWorker::tr("Gap open penalty"),
                   KalignWorker::tr("The penalty for opening/closing a gap. Half the value will be subtracted from the alignment score when opening, and half when closing a gap."));
    Descriptor gep(GAP_EXT_PENALTY, KalignWorker::tr("Gap extension penalty"),
                   KalignWorker::tr("The penalty for extending a gap."));
    Descriptor tgp(TERM_GAP_PENALTY, KalignWorker::tr("Terminal gap penalty"),
                   KalignWorker::tr("The penalty to extend gaps from the N/C terminal of protein or 5'/3' terminal of nucleotide sequences."));
	Descriptor secret(BONUS_SCORE, KalignWorker::tr("Bonus score"),
				   KalignWorker::tr("A bonus score that is added to each pair of aligned residues."));

    a << new Attribute(gop, BaseTypes::NUM_TYPE(), false, QVariant(54.90));
    a << new Attribute(gep, BaseTypes::NUM_TYPE(), false, QVariant(8.52));
    a << new Attribute(tgp, BaseTypes::NUM_TYPE(), false, QVariant(4.42));
    a << new Attribute(secret, BaseTypes::NUM_TYPE(), false, QVariant(0.02));

    Descriptor desc(ACTOR_ID, KalignWorker::tr("Align with Kalign"),
        KalignWorker::tr("Aligns multiple sequence alignments (MSAs) supplied with Kalign."
		"<p>Kalign is a fast and accurate multiple sequence alignment tool. The original version of the tool can be found on <a href=\"http://msa.sbc.su.se\">http://msa.sbc.su.se</a>."));

    ActorPrototype* proto = new IntegralBusActorPrototype(desc, p, a);

    QMap<QString, PropertyDelegate*> delegates;
    {
        QVariantMap m; m["minimum"] = double(.00); m["maximum"] = double(100.00); m["decimals"] = 2;
        delegates[GAP_OPEN_PENALTY] = new DoubleSpinBoxDelegate(m);
    }
    {
        QVariantMap m; m["minimum"] = double(.00); m["maximum"] = double(10.00); m["decimals"] = 2;
        delegates[GAP_EXT_PENALTY] = new DoubleSpinBoxDelegate(m);
    }
    {
        QVariantMap m; m["minimum"] = double(.00); m["maximum"] = double(99.99); m["decimals"] = 2;
        delegates[TERM_GAP_PENALTY] = new DoubleSpinBoxDelegate(m);
    }
    {
        QVariantMap m; m["minimum"] = double(.00); m["maximum"] = double(99.99); m["decimals"] = 2;
        delegates[BONUS_SCORE] = new DoubleSpinBoxDelegate(m);
    }

    proto->setEditor(new DelegateEditor(delegates));
    proto->setPrompter(new KalignPrompter());
    proto->setIconPath(":kalign/images/kalign_16.png");
    WorkflowEnv::getProtoRegistry()->registerProto(BaseActorCategories::CATEGORY_ALIGNMENT(), proto);

    DomainFactory* localDomain = WorkflowEnv::getDomainRegistry()->getById(LocalDomainFactory::ID);
    localDomain->registerEntry(new KalignWorkerFactory());
}

/****************************
* KalignPrompter
****************************/
QString KalignPrompter::composeRichDoc() {
    IntegralBusPort* input = qobject_cast<IntegralBusPort*>(target->getPort(BasePorts::IN_MSA_PORT_ID()));
    Actor* producer = input->getProducer(BasePorts::IN_MSA_PORT_ID());
    QString producerName = producer ? tr(" from %1").arg(producer->getLabel()) : "";

    QString doc = tr("Aligns each MSA supplied <u>%1</u> with \"<u>Kalign</u>\".")
        .arg(producerName);

    return doc;
}

/****************************
* KalignWorker
****************************/
KalignWorker::KalignWorker(Actor* a) : BaseWorker(a), input(NULL), output(NULL) {
}

void KalignWorker::init() {
    input = ports.value(BasePorts::IN_MSA_PORT_ID());
    output = ports.value(BasePorts::OUT_MSA_PORT_ID());
}

bool KalignWorker::isReady() {
    return (input && input->hasMessage());
}

Task* KalignWorker::tick() {
    Message inputMessage = getMessageAndSetupScriptValues(input);
    cfg.gapOpenPenalty=actor->getParameter(GAP_OPEN_PENALTY)->getAttributeValue<float>();
    cfg.gapExtenstionPenalty=actor->getParameter(GAP_EXT_PENALTY)->getAttributeValue<float>();
    cfg.termGapPenalty=actor->getParameter(TERM_GAP_PENALTY)->getAttributeValue<float>();
	cfg.secret=actor->getParameter(BONUS_SCORE)->getAttributeValue<float>();

    MAlignment msa = inputMessage.getData().toMap().value(BaseSlots::MULTIPLE_ALIGNMENT_SLOT().getId()).value<MAlignment>();
    if( msa.isEmpty() ) {
        algoLog.error( tr("An empty MSA has been supplied to Kalign.") );
        return NULL;
    }
    Task* t = new KalignTask(msa, cfg);
    connect(t, SIGNAL(si_stateChanged()), SLOT(sl_taskFinished()));
    return t;
}

void KalignWorker::sl_taskFinished() {
    KalignTask* t = qobject_cast<KalignTask*>(sender());
    if (t->getState() != Task::State_Finished) return;
    QVariant v = qVariantFromValue<MAlignment>(t->resultMA);
    output->put(Message(BaseTypes::MULTIPLE_ALIGNMENT_TYPE(), v));
    if (input->isEnded()) {
        output->setEnded();
    }
    algoLog.info(tr("Aligned %1 with Kalign").arg(t->resultMA.getName()));
}

bool KalignWorker::isDone() {
    return !input || input->isEnded();
}

void KalignWorker::cleanup() {
}

} //namespace LocalWorkflow
} //namespace U2
