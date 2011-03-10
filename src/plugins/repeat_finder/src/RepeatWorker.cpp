#include "RepeatWorker.h"
#include "FindRepeatsDialog.h"

#include <U2Lang/IntegralBusModel.h>
#include <U2Lang/WorkflowEnv.h>
#include <U2Lang/ActorPrototypeRegistry.h>
#include <U2Lang/BaseTypes.h>
#include <U2Lang/BaseSlots.h>
#include <U2Lang/BasePorts.h>
#include <U2Lang/BaseActorCategories.h>
#include <U2Designer/DelegateEditors.h>
#include <U2Lang/CoreLibConstants.h>

#include <U2Core/DNASequence.h>
#include <U2Core/DNATranslation.h>
#include <U2Core/DNAAlphabet.h>
#include <U2Core/AppContext.h>
#include <U2Core/L10n.h>
#include <U2Core/Log.h>
#include <U2Core/GObjectReference.h>
#include <U2Core/FailTask.h>

#include <QtGui/QApplication>

/* TRANSLATOR U2::LocalWorkflow::RepeatWorker */

namespace U2 {
namespace LocalWorkflow {

/******************************
 * RepeatWorkerFactory
 ******************************/
static const QString NAME_ATTR("result-name");
static const QString LEN_ATTR("min-length");
static const QString IDENTITY_ATTR("identity");
static const QString MIN_DIST_ATTR("min-distance");
static const QString MAX_DIST_ATTR("max-distance");
static const QString INVERT_ATTR("inverted");
static const QString NESTED_ATTR("filter-nested");
static const QString ALGO_ATTR("algorithm");
static const QString THREADS_ATTR("threads");

const QString RepeatWorkerFactory::ACTOR_ID("repeats-search");

void RepeatWorkerFactory::init() {
    QList<PortDescriptor*> p; QList<Attribute*> a;

    {
        Descriptor id(BasePorts::IN_SEQ_PORT_ID(), RepeatWorker::tr("Input sequences"), 
                        RepeatWorker::tr("A nucleotide sequence to search repeats in."));
        Descriptor od(BasePorts::OUT_ANNOTATIONS_PORT_ID(), RepeatWorker::tr("Repeat annotations"), 
                        RepeatWorker::tr("A set of annotations marking repeats found in the sequence."));
        QMap<Descriptor, DataTypePtr> inM;
        inM[BaseSlots::DNA_SEQUENCE_SLOT()] = BaseTypes::DNA_SEQUENCE_TYPE();
        p << new PortDescriptor(id, DataTypePtr(new MapDataType("repeat.seq", inM)), true /*input*/);
        QMap<Descriptor, DataTypePtr> outM;
        outM[BaseSlots::ANNOTATION_TABLE_SLOT()] = BaseTypes::ANNOTATION_TABLE_TYPE();
        p << new PortDescriptor(od, DataTypePtr(new MapDataType("repeat.annotations", outM)), false /*input*/, true /*multi*/);
    }
    {
        Descriptor nd(NAME_ATTR, RepeatWorker::tr("Annotate as"), RepeatWorker::tr("Name of the result annotations marking found repeats."));
        Descriptor idd(IDENTITY_ATTR, RepeatWorker::tr("Identity"), RepeatWorker::tr("Repeats identity."));
        Descriptor ld(LEN_ATTR, RepeatWorker::tr("Min length"), RepeatWorker::tr("Minimum length of repeats."));
        Descriptor mid(MIN_DIST_ATTR, RepeatWorker::tr("Min distance"), RepeatWorker::tr("Minimum distance between repeats."));
        Descriptor mad(MAX_DIST_ATTR, RepeatWorker::tr("Max distance"), RepeatWorker::tr("Maximum distance between repeats."));
        Descriptor ind(INVERT_ATTR, RepeatWorker::tr("Inverted"), RepeatWorker::tr("Search for inverted repeats."));
        Descriptor nsd(NESTED_ATTR, RepeatWorker::tr("Filter nested"), RepeatWorker::tr("Filter nested repeats."));
        Descriptor ald(ALGO_ATTR, RepeatWorker::tr("Algorithm"), RepeatWorker::tr("Control over variations of algorithm."));
        Descriptor thd(THREADS_ATTR, RepeatWorker::tr("Parallel threads"), RepeatWorker::tr("Number of parallel threads used for the task."));

        FindRepeatsTaskSettings cfg = FindRepeatsDialog::defaultSettings();
        a << new Attribute(nd, BaseTypes::STRING_TYPE(), true, "repeat_unit");
        a << new Attribute(ld, BaseTypes::NUM_TYPE(), false, cfg.minLen);
        a << new Attribute(idd, BaseTypes::NUM_TYPE(), false, cfg.getIdentity());
        a << new Attribute(mid, BaseTypes::NUM_TYPE(), false, cfg.minDist);
        a << new Attribute(mad, BaseTypes::NUM_TYPE(), false, cfg.maxDist);
        a << new Attribute(ind, BaseTypes::BOOL_TYPE(), false, cfg.inverted);
        a << new Attribute(nsd, BaseTypes::BOOL_TYPE(), false, cfg.filterNested);
        a << new Attribute(ald, BaseTypes::NUM_TYPE(), false, cfg.algo);
        a << new Attribute(thd, BaseTypes::NUM_TYPE(), false, cfg.nThreads);
    }

    Descriptor desc(ACTOR_ID, RepeatWorker::tr("Find repeats"), 
        RepeatWorker::tr("Finds repeats in each supplied sequence, stores found regions as annotations.")
        );
    ActorPrototype* proto = new IntegralBusActorPrototype(desc, p, a);
    QMap<QString, PropertyDelegate*> delegates;    
    {
        QVariantMap m; m["minimum"] = 0; m["maximum"] = INT_MAX; m["suffix"] = L10N::suffixBp();
        delegates[MIN_DIST_ATTR] = new SpinBoxDelegate(m);
        m["specialValueText"] = RepeatWorker::tr("Any");
        delegates[MAX_DIST_ATTR] = new SpinBoxDelegate(m);
        m["minimum"] = 2;
        delegates[LEN_ATTR] = new SpinBoxDelegate(m);
    }
    {
        QVariantMap m; m["minimum"] = 50; m["maximum"] = 100; m["suffix"] = "%";
        delegates[IDENTITY_ATTR] = new SpinBoxDelegate(m);
    }
    {
        QVariantMap m; m["specialValueText"] = "Auto";
        delegates[THREADS_ATTR] = new SpinBoxDelegate(m);
    }
    {
        QVariantMap m; 
        m["Auto"] = RFAlgorithm_Auto;
        m["Diagonals"] = RFAlgorithm_Diagonal;
        m["Suffix index"] = RFAlgorithm_Suffix;
        delegates[ALGO_ATTR] = new ComboBoxDelegate(m);
    }

    proto->setPrompter(new RepeatPrompter());
    proto->setEditor(new DelegateEditor(delegates));
    proto->setIconPath(":repeat_finder/images/repeats.png");
    WorkflowEnv::getProtoRegistry()->registerProto(BaseActorCategories::CATEGORY_BASIC(), proto);

    DomainFactory* localDomain = WorkflowEnv::getDomainRegistry()->getById(LocalDomainFactory::ID);
    localDomain->registerEntry(new RepeatWorkerFactory());
}

/******************************
 * RepeatPrompter
 ******************************/
QString RepeatPrompter::composeRichDoc() {
    IntegralBusPort* input = qobject_cast<IntegralBusPort*>(target->getPort(BasePorts::IN_SEQ_PORT_ID()));
    Actor* producer = input->getProducer(BaseSlots::DNA_SEQUENCE_SLOT().getId());
    QString unsetStr = "<font color='red'>"+tr("unset")+"</font>";
    QString producerName = tr(" from <u>%1</u>").arg(producer ? producer->getLabel() : unsetStr);
    
    // TODO extend ?
    QString resultName = getRequiredParam(NAME_ATTR);
    QString inverted = getParameter(INVERT_ATTR).toBool() ? tr("inverted") : tr("direct");

    QString doc = tr("For each sequence%1, find <u>%2</u> repeats."
        "<br>Detect <u>%3% identical</u> repeats <u>not shorter than %4 bps</u>."
        "<br>Output the list of found regions annotated as <u>%5</u>.")
        .arg(producerName) //sequence from Read Fasta 1
        .arg(inverted) 
        .arg(getParameter(IDENTITY_ATTR).toInt())
        .arg(getParameter(LEN_ATTR).toInt())
        .arg(resultName);
    
    return doc;
}

/******************************
 * RepeatWorker
 ******************************/
RepeatWorker::RepeatWorker(Actor* a) : BaseWorker(a), input(NULL), output(NULL) {
}

void RepeatWorker::init() {
    input = ports.value(BasePorts::IN_SEQ_PORT_ID());
    output = ports.value(BasePorts::OUT_ANNOTATIONS_PORT_ID());
}

bool RepeatWorker::isReady() {
    return (input && input->hasMessage());
}

Task* RepeatWorker::tick() {
    Message inputMessage = getMessageAndSetupScriptValues(input);
    cfg.algo = RFAlgorithm(actor->getParameter(ALGO_ATTR)->getAttributeValue<int>());
    cfg.minLen = actor->getParameter(LEN_ATTR)->getAttributeValue<int>();
    cfg.minDist = actor->getParameter(MIN_DIST_ATTR)->getAttributeValue<int>();
    cfg.maxDist = actor->getParameter(MAX_DIST_ATTR)->getAttributeValue<int>();
    int identity = actor->getParameter(IDENTITY_ATTR)->getAttributeValue<int>();
    cfg.setIdentity(identity);
    cfg.nThreads = actor->getParameter(THREADS_ATTR)->getAttributeValue<int>();
    cfg.inverted = actor->getParameter(INVERT_ATTR)->getAttributeValue<bool>();
    cfg.filterNested = actor->getParameter(NESTED_ATTR)->getAttributeValue<bool>();
    resultName = actor->getParameter(NAME_ATTR)->getAttributeValue<QString>();
    if(resultName.isEmpty()){
        resultName = "repeat_unit";
        algoLog.error(tr("result name is empty, default name used"));
    }
    if(identity > 100 || identity < 0){
        algoLog.error(tr("Incorrect value: identity value must be between 0 and 100"));
        return new FailTask(tr("Incorrect value: identity value must be between 0 and 100"));
    }
    QVariantMap map = inputMessage.getData().toMap();
    DNASequence seq = map.value(BaseSlots::DNA_SEQUENCE_SLOT().getId()).value<DNASequence>();
    
    if(cfg.minDist < 0){
        algoLog.error(tr("Incorrect value: minimal distance must be greater then zero"));
        return new FailTask(tr("Incorrect value: minimal distance must be greater then zero"));
    }
    
    if (!seq.alphabet->isNucleic()) {
        QString err = tr("Sequence alphabet is not nucleic!");
        //if (failFast) {
            return new FailTask(err);
        /*} else {
            algoLog.error(err);
            return NULL;
        }*/
    }
    Task* t = new FindRepeatsToAnnotationsTask(cfg, seq, resultName, QString(), GObjectReference());
    connect(t, SIGNAL(si_stateChanged()), SLOT(sl_taskFinished()));
    return t;
}

void RepeatWorker::sl_taskFinished() {
    FindRepeatsToAnnotationsTask* t = qobject_cast<FindRepeatsToAnnotationsTask*>(sender());
    if (t->getState() != Task::State_Finished || t->hasErrors()) return;
    if (output) {
        const QList<SharedAnnotationData>& res = t->importAnnotations();
        QVariant v = qVariantFromValue<QList<SharedAnnotationData> >(res);
        output->put(Message(BaseTypes::ANNOTATION_TABLE_TYPE(), v));
        if (input->isEnded()) {
            output->setEnded();
        }
        algoLog.info(tr("Found %1 repeats").arg(res.size()));
    }
}

bool RepeatWorker::isDone() {
    return !input || input->isEnded();
}

void RepeatWorker::cleanup() {
}

} //namespace LocalWorkflow
} //namespace U2
