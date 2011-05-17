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

#include "GenomeAlignerPlugin.h"

namespace U2 {
namespace LocalWorkflow {


static const QString INDEX_PORT_ID("in-gen-al-index");
static const QString INDEX_OUT_PORT_ID("out-gen-al-index");

const QString GenomeAlignerWorkerFactory::ACTOR_ID("genome-aligner");
const QString GenomeAlignerBuildWorkerFactory::ACTOR_ID("gen-al-build-index");
const QString GenomeAlignerIndexReaderWorkerFactory::ACTOR_ID("gen-al-read-index");

const QString ABS_OR_PERC_MISMATCHES_ATTR("if-absolute-mismatches-value");
const QString MISMATCHES_ATTR("absolute-mismatches");
const QString PERCENT_MISMATCHES_ATTR("percentage-mismatches");
const QString REVERSE_ATTR("reverse");
const QString BEST_ATTR("best");
const QString GPU_ATTR("gpu");
const QString QUAL_ATTR("quality-threshold");
const QString REFSEQ_URL_ATTR("url-reference");
const QString INDEX_URL_ATTR("url-index");

/************************************************************************/
/* Genome aligner                                                       */
/************************************************************************/
GenomeAlignerWorker::GenomeAlignerWorker(Actor* a)
: BaseWorker(a, false), reads(NULL), index(NULL), output(NULL), reader(NULL), writer(NULL)
{
    done = false;
}

static const Descriptor INDEX_SLOT("gen-al-index-slot", QString("Genome aligner index"), QString("Index for genome aligner"));

void GenomeAlignerWorkerFactory::init() {
    QList<PortDescriptor*> p; QList<Attribute*> a;
    Descriptor readsd(BasePorts::IN_SEQ_PORT_ID(), GenomeAlignerWorker::tr("Short read sequences"), GenomeAlignerWorker::tr("Short reads to be aligned."));
    Descriptor indexd(INDEX_PORT_ID, QString("Genome aligner index"), QString("Genome aligner index of reference sequence."));
    Descriptor oud(BasePorts::OUT_MSA_PORT_ID(), GenomeAlignerWorker::tr("Short reads alignment"), GenomeAlignerWorker::tr("Result of alignment."));

    QMap<Descriptor, DataTypePtr> inSeqM;
    inSeqM[BaseSlots::DNA_SEQUENCE_SLOT()] = BaseTypes::DNA_SEQUENCE_TYPE();
    p << new PortDescriptor(readsd, DataTypePtr(new MapDataType("genome.aligner.in.reads", inSeqM)), true /*input*/);
    QMap<Descriptor, DataTypePtr> inIndexM;
    inIndexM[INDEX_SLOT] = GenomeAlignerPlugin::GENOME_ALIGNER_INDEX_TYPE();
    p << new PortDescriptor(indexd, DataTypePtr(new MapDataType("genome.aligner.in.index", inIndexM)), true /*input*/, false /*multi*/, IntegralBusPort::BLIND_INPUT);
    QMap<Descriptor, DataTypePtr> outM;
    outM[BaseSlots::MULTIPLE_ALIGNMENT_SLOT()] = BaseTypes::MULTIPLE_ALIGNMENT_TYPE();
    p << new PortDescriptor(oud, DataTypePtr(new MapDataType("genome.aligner.out.ma", outM)), false /*input*/, false /*multi*/);

    Descriptor absMismatches(ABS_OR_PERC_MISMATCHES_ATTR, GenomeAlignerWorker::tr("Is absolute mismatches values?"),
        GenomeAlignerWorker::tr("<html><body><p><b>true</b> - absolute mismatches mode is used</p><p><b>false</b> - percentage mismatches mode is used</p>\
                                You can choose absolute or percentage mismatches values mode.</body></html>"));
    Descriptor mismatches(MISMATCHES_ATTR, GenomeAlignerWorker::tr("Absolute mismatches"), 
        GenomeAlignerWorker::tr("<html><body>Number of mismatches allowed while aligning reads.</body></html>"));
    Descriptor ptMismatches(PERCENT_MISMATCHES_ATTR, GenomeAlignerWorker::tr("Percentage mismatches"),
        GenomeAlignerWorker::tr("<html><body>Percentage of mismatches allowed while aligning reads.</body></html>"));
    Descriptor reverse(REVERSE_ATTR, GenomeAlignerWorker::tr("Align reverse complement reads"), 
        GenomeAlignerWorker::tr("<html><body>Set this option to align both direct and reverse complement reads.</body></html>"));
    Descriptor best(BEST_ATTR, GenomeAlignerWorker::tr("Use \"best\"-mode"), 
        GenomeAlignerWorker::tr("<html><body>Report only the best alignment for each read (in terms of mismatches).</body></html>"));
    Descriptor qual(QUAL_ATTR, GenomeAlignerWorker::tr("Omit reads with qualities lower than"), 
        GenomeAlignerWorker::tr("<html><body>Omit reads with qualities lower than the specified value. Reads that have no qualities are not omited.\
                                <p>Set <b>\"0\"</b> to switch off this option.</p></body></html>"));
    Descriptor gpu(GPU_ATTR, GenomeAlignerWorker::tr("Use GPU-optimization"), 
        GenomeAlignerWorker::tr("<html><body>Use GPU-calculatings while aligning reads. This option requires OpenCL-enable GPU-device.</body></html>"));

    a << new Attribute(absMismatches, BaseTypes::BOOL_TYPE(), true/*required*/, false);
    a << new Attribute(mismatches, BaseTypes::NUM_TYPE(), false, 0);
    a << new Attribute(ptMismatches, BaseTypes::NUM_TYPE(), false, 0);
    a << new Attribute(reverse, BaseTypes::BOOL_TYPE(), false/*required*/, true);
    a << new Attribute(best, BaseTypes::BOOL_TYPE(), false/*required*/, false);
    a << new Attribute(qual, BaseTypes::BOOL_TYPE(), false/*required*/, -1);
    a << new Attribute(gpu, BaseTypes::BOOL_TYPE(), false/*required*/, false);

    Descriptor desc(ACTOR_ID, GenomeAlignerWorker::tr("UGENE genome aligner"), 
        GenomeAlignerWorker::tr("Unique UGENE algorithm for aligning short reads to reference genome"));
    ActorPrototype* proto = new IntegralBusActorPrototype(desc, p, a);

    QMap<QString, PropertyDelegate*> delegates;    

    {
        QVariantMap m; m["minimum"] = 0; m["maximum"] = 3;
        delegates[MISMATCHES_ATTR] = new SpinBoxDelegate(m);

        QVariantMap ptM; ptM["minimum"] = 0; ptM["maximum"] = 10;
        delegates[PERCENT_MISMATCHES_ATTR] = new SpinBoxDelegate(ptM);

        QVariantMap q; q["minimum"] = 0; q["maximum"] = 70;
        delegates[QUAL_ATTR] = new SpinBoxDelegate(q);
    }

    proto->setEditor(new DelegateEditor(delegates));
    proto->setPrompter(new GenomeAlignerPrompter());
    proto->setIconPath(":core/images/align.png");
    WorkflowEnv::getProtoRegistry()->registerProto(BaseActorCategories::CATEGORY_ASSEMBLY(), proto);

    DomainFactory* localDomain = WorkflowEnv::getDomainRegistry()->getById(LocalDomainFactory::ID);
    localDomain->registerEntry(new GenomeAlignerWorkerFactory());
}

void GenomeAlignerWorker::init() {
    reader = NULL;
    writer = NULL;
    reads = ports.value(BasePorts::IN_SEQ_PORT_ID());
    index = ports.value(INDEX_PORT_ID);
    output = ports.value(BasePorts::OUT_MSA_PORT_ID());

    //TODO: PREBUILT INDEX
    //settings.setCustomValue(GenomeAlignerTask::OPTION_PREBUILT_INDEX, true);
    settings.prebuiltIndex = true;
    bool absMismatches = actor->getParameter(ABS_OR_PERC_MISMATCHES_ATTR)->getAttributeValue<bool>();
    settings.setCustomValue(GenomeAlignerTask::OPTION_IF_ABS_MISMATCHES, absMismatches);
    int nMismatches = actor->getParameter(MISMATCHES_ATTR)->getAttributeValue<int>();
    settings.setCustomValue(GenomeAlignerTask::OPTION_MISMATCHES, nMismatches);
    int ptMismatches = actor->getParameter(PERCENT_MISMATCHES_ATTR)->getAttributeValue<int>();
    settings.setCustomValue(GenomeAlignerTask::OPTION_PERCENTAGE_MISMATCHES, ptMismatches);
    bool alignReverse = actor->getParameter(REVERSE_ATTR)->getAttributeValue<bool>();
    settings.setCustomValue(GenomeAlignerTask::OPTION_ALIGN_REVERSED, alignReverse);
    bool best = actor->getParameter(BEST_ATTR)->getAttributeValue<bool>();
    settings.setCustomValue(GenomeAlignerTask::OPTION_BEST, best);
    int qual = actor->getParameter(QUAL_ATTR)->getAttributeValue<int>();
    settings.setCustomValue(GenomeAlignerTask::OPTION_QUAL_THRESHOLD, qual);
    bool gpu = actor->getParameter(GPU_ATTR)->getAttributeValue<bool>();
    settings.setCustomValue(GenomeAlignerTask::OPTION_OPENCL, gpu);
}

bool GenomeAlignerWorker::isReady() {
    return (reads && reads->hasMessage() && index && index->hasMessage());
}

Task* GenomeAlignerWorker::tick() {
    if (reads->hasMessage()) {
        if(reads->isEnded()) {
            algoLog.error(GenomeAlignerWorker::tr("Short reads list is empty."));
            return NULL;
        }

        reader = new GenomeAlignerCommunicationChanelReader(reads);
        writer = new GenomeAlignerMAlignmentWriter();

        QString indexFile = index->get().getData().toMap().value(INDEX_SLOT.getId()).value<QString>();
        settings.refSeqUrl = indexFile;
        settings.setCustomValue(GenomeAlignerTask::OPTION_READS_READER, QVariant::fromValue(GenomeAlignerReaderContainer(reader)));
        settings.setCustomValue(GenomeAlignerTask::OPTION_READS_WRITER, QVariant::fromValue(GenomeAlignerWriterContainer(writer)));
        Task* t = new GenomeAlignerTask(settings);
        connect(t, SIGNAL(si_stateChanged()), SLOT(sl_taskFinished()));
        return t;
    }
    return NULL;
}


void GenomeAlignerWorker::cleanup() {
    delete reader;
    delete writer;
    writer = NULL;
    reader = NULL;
}

void GenomeAlignerWorker::sl_taskFinished() {
    GenomeAlignerTask* t = qobject_cast<GenomeAlignerTask*>(sender());
    if (t->getState() != Task::State_Finished) {
        return;
    }
    
    QVariant v = qVariantFromValue<MAlignment>(dynamic_cast<GenomeAlignerMAlignmentWriter*>(writer)->getResult());
    output->put(Message(BaseTypes::MULTIPLE_ALIGNMENT_TYPE(), v));
    if (reads->isEnded()) {
        output->setEnded();
    }
    done = true;
}

QString GenomeAlignerPrompter::composeRichDoc() {
    Actor* readsProducer = qobject_cast<IntegralBusPort*>(target->getPort(BasePorts::IN_SEQ_PORT_ID()))->getProducer(BasePorts::IN_SEQ_PORT_ID());
    Actor* indexProducer = qobject_cast<IntegralBusPort*>(target->getPort(INDEX_PORT_ID))->getProducer(INDEX_PORT_ID);

    QString readsName = readsProducer ? tr(" from <u>%1</u>").arg(readsProducer->getLabel()) : "";
    QString indexName = indexProducer ? tr(" from <u>%1</u>").arg(indexProducer->getLabel()) : "";

    QString doc = tr("Align short reads %1 to the reference genome %2 and send it to output.")
        .arg(readsName).arg(indexName);

    return doc;
}

/************************************************************************/
/* Genome aligner index build                                           */
/************************************************************************/
void GenomeAlignerBuildWorkerFactory::init() {
    QList<PortDescriptor*> p; QList<Attribute*> a;
    Descriptor oud(INDEX_OUT_PORT_ID, QString("Genome aligner index"), QString("Result genome aligner index of reference sequence."));

    QMap<Descriptor, DataTypePtr> outM;
    outM[INDEX_SLOT] = GenomeAlignerPlugin::GENOME_ALIGNER_INDEX_TYPE();
    p << new PortDescriptor(oud, DataTypePtr(new MapDataType("gen.al.build.index.out", outM)), false /*input*/, true /*multi*/);

    Descriptor refseq(REFSEQ_URL_ATTR, GenomeAlignerBuildWorker::tr("Reference"), 
        GenomeAlignerBuildWorker::tr("Reference sequence url. The short reads will be aligned to this reference genome."));
    Descriptor desc(ACTOR_ID, GenomeAlignerBuildWorker::tr("Genome aligner build indexer"), 
        GenomeAlignerBuildWorker::tr("GenomeAlignerBuild builds an index from a set of DNA sequences. GenomeAlignerBuild outputs a set of 3 files with suffixes .idx, .ref, .sarr. These files together constitute the index: they are all that is needed to align reads to that reference."));
    Descriptor index(INDEX_URL_ATTR, GenomeAlignerBuildWorker::tr("Index"), 
        GenomeAlignerBuildWorker::tr("Output index url."));

    a << new Attribute(refseq, BaseTypes::STRING_TYPE(), true /*required*/, QString());
    a << new Attribute(index, BaseTypes::STRING_TYPE(), true /*required*/, QString());

    ActorPrototype* proto = new IntegralBusActorPrototype(desc, p, a);

    QMap<QString, PropertyDelegate*> delegates;    

    delegates[REFSEQ_URL_ATTR] = new URLDelegate(DialogUtils::prepareDocumentsFileFilter(true), QString(), true);
    delegates[INDEX_URL_ATTR] = new URLDelegate(DialogUtils::prepareDocumentsFileFilter(true), QString(), false);

    proto->setEditor(new DelegateEditor(delegates));
    proto->setPrompter(new GenomeAlignerBuildPrompter());
    proto->setIconPath(":core/images/align.png");
    WorkflowEnv::getProtoRegistry()->registerProto(BaseActorCategories::CATEGORY_ASSEMBLY(), proto);

    DomainFactory* localDomain = WorkflowEnv::getDomainRegistry()->getById(LocalDomainFactory::ID);
    localDomain->registerEntry(new GenomeAlignerBuildWorkerFactory());
}

void GenomeAlignerBuildWorker::init() {
    output = ports.value(INDEX_OUT_PORT_ID);
    refSeqUrl = actor->getParameter(REFSEQ_URL_ATTR)->getAttributeValue<QString>();
    indexUrl = actor->getParameter(INDEX_URL_ATTR)->getAttributeValue<QString>();
}

bool GenomeAlignerBuildWorker::isReady() {
    return !isDone();
}

Task* GenomeAlignerBuildWorker::tick() {
    if( refSeqUrl.isEmpty()) {
        algoLog.trace(GenomeAlignerBuildWorker::tr("Reference sequence URL is empty")); 
        return NULL;
    }
    if( indexUrl.isEmpty()) {
        algoLog.trace(GenomeAlignerBuildWorker::tr("Result index URL is empty")); 
        return NULL;
    }

    settings.refSeqUrl = refSeqUrl;
    settings.resultFileName = indexUrl;
    Task* t = new GenomeAlignerTask(settings, true);
    connect(t, SIGNAL(si_stateChanged()), SLOT(sl_taskFinished()));
    return t;
}

void GenomeAlignerBuildWorker::sl_taskFinished() {
    GenomeAlignerTask* t = qobject_cast<GenomeAlignerTask*>(sender());
    if (t->getState() != Task::State_Finished) {
        return;
    }

    done = true;

    QVariant v = qVariantFromValue<QString>(t->getIndexPath());
    output->put(Message(GenomeAlignerPlugin::GENOME_ALIGNER_INDEX_TYPE(), v));
    output->setEnded();
    algoLog.trace(tr("Genome aligner index building finished. Result name is %1").arg(t->getIndexPath()));
}

bool GenomeAlignerBuildWorker::isDone() {
    return done;
}

QString GenomeAlignerBuildPrompter::composeRichDoc() {
    QString refSeqUrl = getParameter(REFSEQ_URL_ATTR).toString();
    QString refSeq = (refSeqUrl.isEmpty() ? "" : QString("<u>%1</u>").arg(GUrl(refSeqUrl).fileName()) );

    QString doc = tr("Build genome aligner index from %1 and send it url to output.").arg(refSeq);

    return doc;
}

/************************************************************************/
/* Genome aligner index read                                            */
/************************************************************************/
void GenomeAlignerIndexReaderWorkerFactory::init() {
    QList<PortDescriptor*> p; QList<Attribute*> a;
    Descriptor oud(INDEX_OUT_PORT_ID, GenomeAlignerIndexReaderWorker::tr("Genome aligner index"), GenomeAlignerIndexReaderWorker::tr("Result of genome aligner index builder."));

    QMap<Descriptor, DataTypePtr> outM;
    outM[INDEX_SLOT] = GenomeAlignerPlugin::GENOME_ALIGNER_INDEX_TYPE();
    p << new PortDescriptor(oud, DataTypePtr(new MapDataType("gen.al.index.reader.out", outM)), false /*input*/, true /*multi*/);

    Descriptor desc(ACTOR_ID, GenomeAlignerIndexReaderWorker::tr("Genome aligner index reader"), 
       GenomeAlignerIndexReaderWorker::tr("Read a set of several files with extensions .idx, .ref, .X.sarr. These files together constitute the index: they are all that is needed to align reads to that reference."));
    Descriptor index(INDEX_URL_ATTR, GenomeAlignerIndexReaderWorker::tr("Index"), 
        GenomeAlignerIndexReaderWorker::tr("Select an index file with the .idx extension"));

    a << new Attribute(index, BaseTypes::STRING_TYPE(), true /*required*/, QString());

    ActorPrototype* proto = new IntegralBusActorPrototype(desc, p, a);

    QMap<QString, PropertyDelegate*> delegates;    

    delegates[INDEX_URL_ATTR] = new URLDelegate(DialogUtils::prepareDocumentsFileFilter(true), QString(), false, false, false);

    proto->setEditor(new DelegateEditor(delegates));
    proto->setPrompter(new GenomeAlignerIndexReaderPrompter());
    proto->setIconPath(":core/images/align.png");
    WorkflowEnv::getProtoRegistry()->registerProto(BaseActorCategories::CATEGORY_ASSEMBLY(), proto);

    DomainFactory* localDomain = WorkflowEnv::getDomainRegistry()->getById(LocalDomainFactory::ID);
    localDomain->registerEntry(new GenomeAlignerIndexReaderWorkerFactory());
}

void GenomeAlignerIndexReaderWorker::init() {
    output = ports.value(INDEX_OUT_PORT_ID);
    indexUrl = actor->getParameter(INDEX_URL_ATTR)->getAttributeValue<QString>();
}

bool GenomeAlignerIndexReaderWorker::isReady() {
    return !isDone();
}

Task *GenomeAlignerIndexReaderWorker::tick() {

    if(indexUrl.isEmpty()) {
        algoLog.trace(GenomeAlignerIndexReaderWorker::tr("Index URL is empty")); 
        return NULL;
    }
    Task* t = new Task("Genome aligner index reader", TaskFlags_NR_FOSCOE);
    connect(t, SIGNAL(si_stateChanged()), SLOT(sl_taskFinished()));
    return t;
}

void GenomeAlignerIndexReaderWorker::sl_taskFinished() {
    QVariant v = qVariantFromValue<QString>(indexUrl.getURLString());
    output->put(Message(GenomeAlignerPlugin::GENOME_ALIGNER_INDEX_TYPE(), v));
    output->setEnded();
    done = true;
    algoLog.trace(tr("Reading genome aligner index finished. Result name is %1").arg(indexUrl.getURLString()));
}

bool GenomeAlignerIndexReaderWorker::isDone() {
    return done;
}

QString GenomeAlignerIndexReaderPrompter::composeRichDoc() {
    QString indexUrl = getParameter(INDEX_URL_ATTR).toString();
    QString index = (indexUrl.isEmpty() ? "" : QString("<u>%1</u>").arg(GUrl(indexUrl).fileName()));

    QString doc = tr("Read genome aligner index from %1 and send it url to output.").arg(index);

    return doc;
}
} //namespace LocalWorkflow
} //namespace U2
