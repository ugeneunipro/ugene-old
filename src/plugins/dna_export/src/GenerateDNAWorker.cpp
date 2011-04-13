#include "GenerateDNAWorker.h"
#include "DNASequenceGenerator.h"

#include <U2Lang/BaseSlots.h>
#include <U2Lang/BasePorts.h>
#include <U2Lang/BaseTypes.h>
#include <U2Lang/BaseAttributes.h>
#include <U2Lang/BaseActorCategories.h>
#include <U2Lang/ActorModel.h>
#include <U2Lang/WorkflowEnv.h>
#include <U2Lang/CoreLibConstants.h>
#include <U2Lang/ActorPrototypeRegistry.h>

#include <U2Designer/DelegateEditors.h>

#include <U2Core/L10n.h>
#include <U2Core/FailTask.h>
#include <U2Core/AppContext.h>
#include <U2Core/DNAAlphabet.h>
#include <U2Core/TaskSignalMapper.h>


namespace U2 {
namespace LocalWorkflow {

/**************************
* GenerateDNAWorkerFactory 
/**************************/

static const QString LENGHT_ATTR("length");
static const QString SEQ_NUM_ATTR("count");
static const QString CONTENT_ATTR("content");
static const QString REFERENCE_ATTR("reference-url");
static const QString A_PERCENT_ATTR("percent-a");
static const QString C_PERCENT_ATTR("percent-c");
static const QString G_PERCENT_ATTR("percent-g");
static const QString T_PERCENT_ATTR("percent-t");

const QString GenerateDNAWorkerFactory::ACTOR_ID("generate-dna");

class ContentIds {
public:
    static const QString REFERENCE;
    static const QString MANUAL;
};

const QString ContentIds::REFERENCE("reference");
const QString ContentIds::MANUAL("manual");

void GenerateDNAWorkerFactory::init() {
    QList<PortDescriptor*> p;
    {
        Descriptor d(BasePorts::OUT_SEQ_PORT_ID(), GenerateDNAWorker::tr("Sequences"), GenerateDNAWorker::tr("Generated sequences"));

        QMap<Descriptor, DataTypePtr> outM;
        outM[BaseSlots::DNA_SEQUENCE_SLOT()] = BaseTypes::DNA_SEQUENCE_TYPE();
        p << new PortDescriptor(d, DataTypePtr(new MapDataType("random.sequence", outM)), false, true);
    }

    QList<Attribute*> a;
    {
        Descriptor ld(LENGHT_ATTR, GenerateDNAWorker::tr("Length"), GenerateDNAWorker::tr("Length of the resulted sequence(s)."));
        Descriptor nd(SEQ_NUM_ATTR, GenerateDNAWorker::tr("Count"), GenerateDNAWorker::tr("Number of sequences to generate."));
        Descriptor cd(CONTENT_ATTR, GenerateDNAWorker::tr("Content"), GenerateDNAWorker::tr("Specifies if the nucleotide content of generated sequence(s) will be taken from reference or specified manually (A, G, C, T parameters)."));
        Descriptor rd(REFERENCE_ATTR, GenerateDNAWorker::tr("Reference"), GenerateDNAWorker::tr("Path to the reference file (could be a sequence or an alignment)."));
        Descriptor apd(A_PERCENT_ATTR, GenerateDNAWorker::tr("A"), GenerateDNAWorker::tr("Adenine content."));
        Descriptor cpd(C_PERCENT_ATTR, GenerateDNAWorker::tr("C"), GenerateDNAWorker::tr("Cytosine content."));
        Descriptor gpd(G_PERCENT_ATTR, GenerateDNAWorker::tr("G"), GenerateDNAWorker::tr("Guanine content."));
        Descriptor tpd(T_PERCENT_ATTR, GenerateDNAWorker::tr("T"), GenerateDNAWorker::tr("Thymine content."));

        a << new Attribute(ld, BaseTypes::NUM_TYPE(), false, 1000);
        a << new Attribute(nd, BaseTypes::NUM_TYPE(), false, 1);
        a << new Attribute(cd, BaseTypes::STRING_TYPE(), false, ContentIds::MANUAL);
        a << new Attribute(rd, BaseTypes::STRING_TYPE(), false);
        a << new Attribute(apd, BaseTypes::NUM_TYPE(), false, 25);
        a << new Attribute(cpd, BaseTypes::NUM_TYPE(), false, 25);
        a << new Attribute(gpd, BaseTypes::NUM_TYPE(), false, 25);
        a << new Attribute(tpd, BaseTypes::NUM_TYPE(), false, 25);
    }

    QMap<QString, PropertyDelegate*> delegates;
    {
        QVariantMap lenMap;
        lenMap["minimum"] = 10;
        lenMap["maximum"] = INT_MAX;
        lenMap["suffix"] = L10N::suffixBp();
        delegates[LENGHT_ATTR] = new SpinBoxDelegate(lenMap);

        QVariantMap countMap;
        countMap["minimum"] = 1;
        countMap["maximum"] = 1000;
        delegates[SEQ_NUM_ATTR] = new SpinBoxDelegate(countMap);

        QVariantMap contentMap;
        contentMap[ContentIds::REFERENCE] = ContentIds::REFERENCE;
        contentMap[ContentIds::MANUAL] = ContentIds::MANUAL;
        delegates[CONTENT_ATTR] = new ComboBoxDelegate(contentMap);

        delegates[REFERENCE_ATTR] = new URLDelegate(DNASequenceGenerator::prepareReferenceFileFilter(), DNASequenceGenerator::ID);

        QVariantMap percentMap;
        percentMap["minimum"] = 0;
        percentMap["maximum"] = 100;
        percentMap["suffix"] = " %";
        delegates[A_PERCENT_ATTR] = new SpinBoxDelegate(percentMap);
        delegates[C_PERCENT_ATTR] = new SpinBoxDelegate(percentMap);
        delegates[G_PERCENT_ATTR] = new SpinBoxDelegate(percentMap);
        delegates[T_PERCENT_ATTR] = new SpinBoxDelegate(percentMap);
    }

    Descriptor desc(ACTOR_ID, GenerateDNAWorker::tr("Generate DNA"),
        GenerateDNAWorker::tr("Generates random DNA sequences with given nucleotide content"
        " that can be specified manually or evaluated from the reference file."));

    ActorPrototype* proto = new IntegralBusActorPrototype(desc, p, a);
    proto->setPrompter(new GenerateDNAPrompter());
    proto->setEditor(new DelegateEditor(delegates));
    WorkflowEnv::getProtoRegistry()->registerProto(BaseActorCategories::CATEGORY_DATASRC(), proto);

    DomainFactory* localDomain = WorkflowEnv::getDomainRegistry()->getById(LocalDomainFactory::ID);
    localDomain->registerEntry(new GenerateDNAWorkerFactory());
}

/**************************
* GenerateDNAPrompter 
/**************************/

QString GenerateDNAPrompter::composeRichDoc() {
    return tr("Generates random DNA sequence(s)");
}

/**************************
* GenerateDNAWorker 
/**************************/

void GenerateDNAWorker::init() {
    done = false;
    ch = ports.value(BasePorts::OUT_SEQ_PORT_ID());
}

bool GenerateDNAWorker::isReady() {
    return !isDone();
}

Task* GenerateDNAWorker::tick() {
    done = true;
    DNASequenceGeneratorConfig cfg;
    cfg.sequenceName = "Sequence ";

    QString cs = actor->getParameter(CONTENT_ATTR)->getAttributeValue<QString>();
    if (cs == ContentIds::REFERENCE) {
        cfg.useRef = true;
    } else if (cs == ContentIds::MANUAL) {
        cfg.useRef = false;
    } else {
        QString err = tr("Unexpected value of 'content' parameter");
        return new FailTask(err);
    }

    if (cfg.useRef) {
        cfg.refUrl = actor->getParameter(REFERENCE_ATTR)->getAttributeValue<QString>();
        if (cfg.refUrl.isEmpty()) {
            QString err = tr("Reference url is not set.");
            return new FailTask(err);
        }
    } else {
        int percentA = actor->getParameter(A_PERCENT_ATTR)->getAttributeValue<int>();
        int percentC = actor->getParameter(C_PERCENT_ATTR)->getAttributeValue<int>();
        int percentG = actor->getParameter(G_PERCENT_ATTR)->getAttributeValue<int>();
        int percentT = actor->getParameter(T_PERCENT_ATTR)->getAttributeValue<int>();
        if (percentA<0 || percentC<0 || percentG<0 || percentT<0) {
            QString err = tr("Base content must be between 0 and 100");
            return new FailTask(err);
        }
        int total = percentA + percentC + percentG + percentT;
        if (total > 100) {
            QString err = tr("Total content percentage is more than 100");
            return new FailTask(err);
        }
        cfg.content['A'] = percentA / 100.0;
        cfg.content['C'] = percentC / 100.0;
        cfg.content['G'] = percentG / 100.0;
        cfg.content['T'] = percentT / 100.0;

        cfg.alphabet = AppContext::getDNAAlphabetRegistry()->findById(BaseDNAAlphabetIds::NUCL_DNA_DEFAULT());
    }
    
    cfg.length = actor->getParameter(LENGHT_ATTR)->getAttributeValue<int>();

    if (cfg.length < 10) {
        QString err = "'length' parameter value must be not less than 10";
        return new FailTask(err);
    }

    cfg.numSeqs = actor->getParameter(SEQ_NUM_ATTR)->getAttributeValue<int>();

    if (cfg.numSeqs < 1) {
        QString err = "Number of sequences to generate is less than 1";
        return new FailTask(err);
    }

    // true is used when executed from the dialog window
    cfg.saveDoc = false;

    Task* t = new DNASequenceGeneratorTask(cfg);
    connect(new TaskSignalMapper(t), SIGNAL(si_taskFinished(Task*)), SLOT(sl_taskFinished(Task*)));
    return t;
}

bool GenerateDNAWorker::isDone() {
    return done;
}

void GenerateDNAWorker::sl_taskFinished(Task* t) {
    DNASequenceGeneratorTask* task = qobject_cast<DNASequenceGeneratorTask*>(t);
    if (ch) {
        foreach(DNASequence seq, task->getSequences()) {
            ch->put(Message(BaseTypes::DNA_SEQUENCE_TYPE(), qVariantFromValue<DNASequence>(seq)));
        }
        ch->setEnded();
    }
}

} //namespace LocalWorkflow
} // U2 namespace
