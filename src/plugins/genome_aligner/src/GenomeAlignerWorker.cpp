#include "GenomeAlignerWorker.h"

#include <U2Core/Log.h>
#include <U2Lang/IntegralBusModel.h>
#include <U2Lang/WorkflowEnv.h>
#include <U2Lang/ActorPrototypeRegistry.h>
#include <U2Lang/BaseTypes.h>
#include <U2Lang/BaseSlots.h>
#include <U2Lang/BasePorts.h>
#include <U2Lang/BaseAttributes.h>
#include <U2Lang/BaseActorCategories.h>
#include <U2Designer/DelegateEditors.h>
#include <U2Lang/CoreLibConstants.h>
#include <U2Misc/DialogUtils.h>



/* TRANSLATOR U2::LocalWorkflow::GenomeAlignerWorker */

namespace U2 {
namespace LocalWorkflow {

const QString GenomeAlignerWorkerFactory::ACTOR_ID("genome.assembly");
const QString ABS_OR_PERC_MISMATCHES_ATTR("if.absolute.mismatches.value");
const QString MISMATCHES_ATTR("mismatches");
const QString PERCENT_MISMATCHES_ATTR("mismatches.percentage");
const QString REVERSE_ATTR("reverse");
const QString PREBUILT_IDX_ATTR("prebuilt.index");
const QString INDEX_FILE_ATTR("index.file.name");

void GenomeAlignerWorkerFactory::init() {
    QList<PortDescriptor*> p; QList<Attribute*> a;
    Descriptor ind(BasePorts::IN_SEQ_PORT_ID(), GenomeAlignerWorker::tr("Short read sequences"), GenomeAlignerWorker::tr("Short reads to be aligned."));
    Descriptor oud(BasePorts::OUT_MSA_PORT_ID(), GenomeAlignerWorker::tr("Short reads alignment"), GenomeAlignerWorker::tr("Result of alignment."));
    p << new PortDescriptor(ind, BaseTypes::DNA_SEQUENCE_TYPE(), true /*input*/);
    p << new PortDescriptor(oud, BaseTypes::MULTIPLE_ALIGNMENT_TYPE(), false /*input*/, true /*multi*/);
    Descriptor refseq(BaseAttributes::URL_IN_ATTRIBUTE().getId(), GenomeAlignerWorker::tr("Reference"), 
         GenomeAlignerWorker::tr("Reference sequence url. The short reads will be aligned to this genome."));
    Descriptor absMismatches(ABS_OR_PERC_MISMATCHES_ATTR, GenomeAlignerWorker::tr("Absolute of percentage mismatches' values"),
        GenomeAlignerWorker::tr("Choose absolute of percentage mismatches' values"));
    Descriptor mismatches(MISMATCHES_ATTR, GenomeAlignerWorker::tr("Mismatches"), 
        GenomeAlignerWorker::tr("Number of mismatches allowed while aligning reads."));
    Descriptor ptMismatches(PERCENT_MISMATCHES_ATTR, GenomeAlignerWorker::tr("Mismatches percentage"),
        GenomeAlignerWorker::tr("Percentage of mismatches allowed while aligning reads."));
    Descriptor reverse(REVERSE_ATTR, GenomeAlignerWorker::tr("Align reversed"), 
        GenomeAlignerWorker::tr("Set this option to align both direct and reverse reads."));
    Descriptor prebuiltIdx(PREBUILT_IDX_ATTR, GenomeAlignerWorker::tr("Prebuilt index"),
        GenomeAlignerWorker::tr("Set TRUE if you wish to use prebuilt index for this reference"));
    Descriptor indexFile(INDEX_FILE_ATTR, GenomeAlignerWorker::tr("Index file"),
        GenomeAlignerWorker::tr("Path to the index file"));

    a << new Attribute(refseq, BaseTypes::STRING_TYPE(), true /*required*/, QString());
    a << new Attribute(absMismatches, BaseTypes::BOOL_TYPE(), true/*required*/, true);
    a << new Attribute(mismatches, BaseTypes::NUM_TYPE(), false, 0);
    a << new Attribute(ptMismatches, BaseTypes::NUM_TYPE(), false, 0);
    a << new Attribute(reverse, BaseTypes::BOOL_TYPE(), false/*required*/, true); 
    a << new Attribute(prebuiltIdx, BaseTypes::BOOL_TYPE(), true/*required*/, false);
    a << new Attribute(indexFile, BaseTypes::STRING_TYPE(), true/*required*/, "");

    Descriptor desc(ACTOR_ID, GenomeAlignerWorker::tr("UGENE genome aligner"), 
        GenomeAlignerWorker::tr("Unique UGENE algorithm for aligning short reads to reference genome"));
    ActorPrototype* proto = new IntegralBusActorPrototype(desc, p, a);

    QMap<QString, PropertyDelegate*> delegates;    

    delegates[BaseAttributes::URL_IN_ATTRIBUTE().getId()] = new URLDelegate(DialogUtils::prepareDocumentsFileFilter(true), QString(), true);
    {
        QVariantMap m; m["minimum"] = 0; m["maximum"] = 3;
        delegates[MISMATCHES_ATTR] = new SpinBoxDelegate(m);

        QVariantMap ptM; ptM["minimum"] = 0; ptM["maximum"] = 10;
        delegates[PERCENT_MISMATCHES_ATTR] = new SpinBoxDelegate(ptM);
    }

    proto->setEditor(new DelegateEditor(delegates));
    proto->setPrompter(new GenomeAlignerPrompter());
    proto->setIconPath(":core/images/align.png");
    WorkflowEnv::getProtoRegistry()->registerProto(BaseActorCategories::CATEGORY_ASSEMBLY(), proto);

    DomainFactory* localDomain = WorkflowEnv::getDomainRegistry()->getById(LocalDomainFactory::ID);
    localDomain->registerEntry(new GenomeAlignerWorkerFactory());
}

QString GenomeAlignerPrompter::composeRichDoc() {
    IntegralBusPort* input = qobject_cast<IntegralBusPort*>(target->getPort(BasePorts::IN_SEQ_PORT_ID()));
    Actor* producer = input->getProducer(BasePorts::IN_SEQ_PORT_ID());
    QString producerName = producer ? tr(" from <u>%1</u>").arg(producer->getLabel()) : "";
    QString refSeqUrl = getParameter(BaseAttributes::URL_IN_ATTRIBUTE().getId()).toString();
    QString refSeq = (refSeqUrl.isEmpty() ? "" : QString("<u>%1</u>").arg(GUrl(refSeqUrl).fileName()) );

    QString doc = tr("Align short reads %1 to the reference genome %2 and send it to output.")
        .arg(producerName).arg(refSeq);

    return doc;
}

void GenomeAlignerWorker::init() {
    input = ports.value(BasePorts::IN_SEQ_PORT_ID());
    output = ports.value(BasePorts::OUT_MSA_PORT_ID());
   
}

bool GenomeAlignerWorker::isReady() {
    return (input && input->hasMessage());
}

Task* GenomeAlignerWorker::tick() {

     //TODO: bug-0001958
    assert(0);
    if (input->hasMessage())  {  
        DNASequence read = input->get().getData().value<DNASequence>();
        algoLog.trace(QString("Loaded short read %1").arg(read.getName())); 
        settings.shortReads.append(read);
        if (input->isEnded()) {
            if( settings.shortReads.isEmpty() ) {
                algoLog.error( tr("Short reads list is empty.") );
                return NULL;
            }

            settings.refSeqUrl = actor->getParameter(BaseAttributes::URL_IN_ATTRIBUTE().getId())->getAttributeValue<QString>();
            bool absMismatches = actor->getParameter(ABS_OR_PERC_MISMATCHES_ATTR)->getAttributeValue<bool>();
            settings.setCustomValue(GenomeAlignerTask::OPTION_IF_ABS_MISMATCHES, absMismatches);
            int nMismatches = actor->getParameter(MISMATCHES_ATTR)->getAttributeValue<int>();
            settings.setCustomValue(GenomeAlignerTask::OPTION_MISMATCHES, nMismatches);
            int ptMismatches = actor->getParameter(PERCENT_MISMATCHES_ATTR)->getAttributeValue<int>();
            settings.setCustomValue(GenomeAlignerTask::OPTION_PERCENTAGE_MISMATCHES, ptMismatches);
            bool alignReverse = actor->getParameter(REVERSE_ATTR)->getAttributeValue<bool>();
            settings.setCustomValue(GenomeAlignerTask::OPTION_ALIGN_REVERSED, alignReverse);
            bool prebuiltIdx = actor->getParameter(PREBUILT_IDX_ATTR)->getAttributeValue<bool>();
            settings.setCustomValue(GenomeAlignerTask::OPTION_PREBUILT_INDEX, prebuiltIdx);
            QString indexFile = actor->getParameter(INDEX_FILE_ATTR)->getAttributeValue<QString>();
            settings.setCustomValue(GenomeAlignerTask::OPTION_INDEX_URL, indexFile);
            Task* t = new GenomeAlignerTask(settings);
            connect(t, SIGNAL(si_stateChanged()), SLOT(sl_taskFinished()));
            return t;
        } 
    }
    return NULL;
}

void GenomeAlignerWorker::sl_taskFinished() {
    GenomeAlignerTask* t = qobject_cast<GenomeAlignerTask*>(sender());
    if (t->getState() != Task::State_Finished) {
        return;
    }
    
    QVariant v = qVariantFromValue<MAlignment>(t->getResult());
    output->put(Message(BaseTypes::MULTIPLE_ALIGNMENT_TYPE(), v));
    if (input->isEnded()) {
        output->setEnded();
    }
    algoLog.trace(tr("Genome aligner finished. Result name is %1").arg(t->getResult().getName()));
}

bool GenomeAlignerWorker::isDone() {
    return !input || input->isEnded();
}

} //namespace LocalWorkflow
} //namespace U2
