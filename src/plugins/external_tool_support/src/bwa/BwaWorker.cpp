/**
 * UGENE - Integrated Bioinformatics Tools.
 * Copyright (C) 2008-2014 UniPro <ugene@unipro.ru>
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

#include <QtCore/QScopedPointer>

#include <U2Algorithm/DnaAssemblyTask.h>

#include <U2Core/AnnotationTableObject.h>
#include <U2Core/FailTask.h>
#include <U2Core/U2OpStatusUtils.h>
#include <U2Core/U2SafePoints.h>
#include <U2Core/QVariantUtils.h>
#include <U2Core/GUrlUtils.h>
#include <U2Formats/GenbankLocationParser.h>

#include <U2Designer/DelegateEditors.h>

#include <U2Gui/DialogUtils.h>

#include <U2Lang/ActorPrototypeRegistry.h>
#include <U2Lang/BaseActorCategories.h>
#include <U2Lang/BaseTypes.h>
#include <U2Lang/WorkflowEnv.h>
#include <U2Lang/WorkflowMonitor.h>

#include "BwaSupport.h"
#include "BwaWorker.h"
#include "BwaTask.h"

namespace U2 {
namespace LocalWorkflow {

const QString BWAWorkerFactory::ACTOR_ID("align-reads-with-bwa");

static const QString READS_URL_SLOT_ID("readsurl");
static const QString READS_PAIRED_URL_SLOT_ID("readspairedurl");

static const QString IN_TYPE_ID("bwa-data");
static const QString OUT_TYPE_ID("bwa-data-out");

static const QString ASSEBLY_OUT_SLOT_ID("assembly-out");

static const QString IN_PORT_DESCR("in-data");
static const QString OUT_PORT_DESCR("out-data");

static const QString OUTPUT_DIR("output-dir");
static const QString REFERENCE_GENOME("reference");

static const QString OUTPUT_NAME = "outname";

static const QString USE_MISS_PROB = "use-miss-prob";
static const QString MAX_DIFF = "max-diff";
static const QString MISSING_PROB = "missing-prob";
static const QString SEED_LENGTH = "seed-length";
static const QString INDEX_ALG = "index-alg";
static const QString BEST_HITS = "best-hits";
static const QString SCALED_GAP = "scaled-gap";
static const QString LONG_DELETIONS = "long-deletions";
static const QString NON_ITERATIVE = "non-iterative";
static const QString ENABLE_LONG_GAPS = "enable-long-gaps";
static const QString MAX_GAP = "max-gap";
static const QString GAP_EXTENSIONS = "gap-extensions";
static const QString INDEL_OFFSET = "indel-offset";
static const QString MAX_QUEUE_ENTRIES = "max-queue";
static const QString BARCODE_LENGTH = "barcode-length";
static const QString NUM_THREADS = "num-threads";
static const QString MAX_SEED_DIFF = "max-seed";
static const QString MISMATCH_PENALTY = "mistmatch-penalty";
static const QString GAP_OPEN_PENALTY = "gap-open-penalty";
static const QString GAP_EXTENSION_PENALTY = "gap-ext-penalty";
static const QString QUALITY_THRESHOLD = "quality-threshold";


static const QString BASE_BWA_SUBDIR("bwa");
static const QString BASE_BWA_OUTFILE("out.sam");


/************************************************************************/
/* Worker */
/************************************************************************/
BWAWorker::BWAWorker(Actor *p)
: BaseWorker(p)
, inChannel(NULL)
, output(NULL)
{

}

void BWAWorker::init() {
    inChannel = ports.value(IN_PORT_DESCR);
    output = ports.value(OUT_PORT_DESCR);
}

Task *BWAWorker::tick() {
    if (inChannel->hasMessage()) {
        U2OpStatus2Log os;

        Message m = getMessageAndSetupScriptValues(inChannel);
        QVariantMap data = m.getData().toMap();

        DnaAssemblyToRefTaskSettings settings = getSettings(os);
        if (os.hasError()) {
            return new FailTask(os.getError());
        }

        QString readsUrl = data[READS_URL_SLOT_ID].toString();

        if(data.contains(READS_PAIRED_URL_SLOT_ID)){
            //paired
            QString readsPairedUrl = data[READS_PAIRED_URL_SLOT_ID].toString();
            settings.shortReadSets.append(ShortReadSet(readsUrl, ShortReadSet::PairedEndReads, ShortReadSet::UpstreamMate));
            settings.shortReadSets.append(ShortReadSet(readsPairedUrl, ShortReadSet::PairedEndReads, ShortReadSet::DownstreamMate));
            settings.pairedReads = true;
        }else {
            //single
            settings.shortReadSets.append(ShortReadSet(readsUrl, ShortReadSet::SingleEndReads, ShortReadSet::UpstreamMate));
            settings.pairedReads = false;
        }

        BwaTask* t = new BwaTask(settings);
        connect(t, SIGNAL(si_stateChanged()), SLOT(sl_taskFinished()));
        return t;
    }else if (inChannel->isEnded()) {
        setDone();
        output->setEnded();
    }
    return NULL;
}

void BWAWorker::cleanup() {

}

void BWAWorker::sl_taskFinished() {
    BwaTask *t = dynamic_cast<BwaTask*>(sender());
    if (!t->isFinished() || t->hasError() || t->isCanceled()) {
        return;
    }

    QString url = t->getSettings().resultFileName.getURLString();

    QVariantMap data;
    data[ASSEBLY_OUT_SLOT_ID] =  qVariantFromValue<QString>(url);
    output->put(Message(output->getBusType(), data));

    context->getMonitor()->addOutputFile(url, getActor()->getId());

    if (inChannel->isEnded() && !inChannel->hasMessage()) {
        setDone();
        output->setEnded();
    }
}

DnaAssemblyToRefTaskSettings BWAWorker::getSettings( U2OpStatus &os ){
    DnaAssemblyToRefTaskSettings settings;

    settings.prebuiltIndex = true;

    QString outDir = GUrlUtils::createDirectory(
        getValue<QString>(OUTPUT_DIR) + QDir::separator() + BASE_BWA_SUBDIR,
        "_", os);
    CHECK_OP(os, settings);
    
    if (!outDir.endsWith(QDir::separator())){
        outDir  = outDir + QDir::separator();
    }

    QString outFileName = getValue<QString>(OUTPUT_NAME);
    if(outFileName.isEmpty()){
        outFileName = BASE_BWA_OUTFILE;
    }
    settings.resultFileName = outDir + outFileName;
    
    settings.indexFileName = getValue<QString>(REFERENCE_GENOME);
    settings.refSeqUrl = GUrl(settings.indexFileName).baseFileName();
    settings.algName = BwaTask::ALGORITHM_BWA_ALN;

    QMap<QString, QVariant> customSettings;

    if(getValue<bool>(USE_MISS_PROB)) {
        customSettings.insert(BwaTask::OPTION_N, getValue<double>(MISSING_PROB));
    } else {
        customSettings.insert(BwaTask::OPTION_N, getValue<int>(MAX_DIFF));
    }
    customSettings.insert(BwaTask::OPTION_SEED_LENGTH, getValue<int>(SEED_LENGTH));
    customSettings.insert(BwaTask::OPTION_INDEX_ALGORITHM, getValue<QString>(INDEX_ALG));
    customSettings.insert(BwaTask::OPTION_BEST_HITS, getValue<int>(BEST_HITS));
    customSettings.insert(BwaTask::OPTION_MAX_GAP_OPENS, getValue<int>(MAX_GAP));
    customSettings.insert(BwaTask::OPTION_LONG_SCALED_GAP_PENALTY_FOR_LONG_DELETIONS, getValue<bool>(SCALED_GAP));
    customSettings.insert(BwaTask::OPTION_MAX_LONG_DELETION_EXTENSIONS, getValue<int>(LONG_DELETIONS));
    customSettings.insert(BwaTask::OPTION_NON_ITERATIVE_MODE, getValue<bool>(NON_ITERATIVE));
    if(getValue<bool>(ENABLE_LONG_GAPS)) {
        customSettings.insert(BwaTask::OPTION_MAX_GAP_EXTENSIONS, getValue<int>(GAP_EXTENSIONS));
    }

    customSettings.insert(BwaTask::OPTION_INDEL_OFFSET, getValue<int>(INDEL_OFFSET));
    customSettings.insert(BwaTask::OPTION_MAX_QUEUE_ENTRIES, getValue<int>(MAX_QUEUE_ENTRIES));
    customSettings.insert(BwaTask::OPTION_BARCODE_LENGTH, getValue<int>(BARCODE_LENGTH));
    customSettings.insert(BwaTask::OPTION_THREADS, getValue<int>(NUM_THREADS));
    customSettings.insert(BwaTask::OPTION_MAX_SEED_DIFFERENCES, getValue<int>(MAX_SEED_DIFF));
    customSettings.insert(BwaTask::OPTION_MISMATCH_PENALTY, getValue<int>(MISMATCH_PENALTY));
    customSettings.insert(BwaTask::OPTION_GAP_OPEN_PENALTY, getValue<int>(GAP_OPEN_PENALTY));
    customSettings.insert(BwaTask::OPTION_GAP_EXTENSION_PENALTY, getValue<int>(GAP_EXTENSION_PENALTY));
    customSettings.insert(BwaTask::OPTION_QUALITY_THRESHOLD, getValue<int>(QUALITY_THRESHOLD));

    settings.setCustomSettings(customSettings);

    return settings;
}

/************************************************************************/
/* Factory */
/************************************************************************/
class BWAInputSlotsValidator : public PortValidator {
    public:

    bool validate(const IntegralBusPort *port, ProblemList &problemList) const {
        QVariant busMap = port->getParameter(Workflow::IntegralBusPort::BUS_MAP_ATTR_ID)->getAttributePureValue();
        bool data = isBinded(busMap.value<QStrStrMap>(), READS_URL_SLOT_ID);
        if (!data){
            QString dataName = slotName(port, READS_URL_SLOT_ID);
            problemList.append(Problem(IntegralBusPort::tr("The slot must be not empty: '%1'").arg(dataName)));
            return false;
        }

        QString slot1Val = busMap.value<QStrStrMap>().value(READS_URL_SLOT_ID);
        QString slot2Val = busMap.value<QStrStrMap>().value(READS_PAIRED_URL_SLOT_ID);
        U2OpStatusImpl os;
        const QList<IntegralBusSlot>& slots1 = IntegralBusSlot::listFromString(slot1Val, os);
        const QList<IntegralBusSlot>& slots2 = IntegralBusSlot::listFromString(slot2Val, os);

        bool hasCommonElements = false;

        foreach(const IntegralBusSlot& ibsl1, slots1){
            if (hasCommonElements){
                break;
            }
            foreach(const IntegralBusSlot& ibsl2, slots2){
                if (ibsl1 == ibsl2){
                    hasCommonElements = true;
                    break;
                }
            }
        }

        if (hasCommonElements){
            problemList.append(Problem(BWAWorker::tr("BWA cannot recognize read pairs from the same file. Please, perform demultiplexing first.")));
            return false;
        }

        return true;
    }
    };

void BWAWorkerFactory::init() {
    QList<PortDescriptor*> portDescs;
    
    //in port
    QMap<Descriptor, DataTypePtr> inTypeMap;
    Descriptor readsDesc(READS_URL_SLOT_ID,
        BWAWorker::tr("URL of a file with reads"),
        BWAWorker::tr("Input reads to be aligned."));
    Descriptor readsPairedDesc(READS_PAIRED_URL_SLOT_ID,
        BWAWorker::tr("URL of a file with mate reads"),
        BWAWorker::tr("Input mate reads to be aligned."));

    inTypeMap[readsDesc] = BaseTypes::STRING_TYPE();
    inTypeMap[readsPairedDesc] = BaseTypes::STRING_TYPE();

    Descriptor inPortDesc(IN_PORT_DESCR,
        BWAWorker::tr("BWA data"),
        BWAWorker::tr("Input reads to be aligned with BWA."));

    DataTypePtr inTypeSet(new MapDataType(IN_TYPE_ID, inTypeMap));
    portDescs << new PortDescriptor(inPortDesc, inTypeSet, true);

    //out port
    QMap<Descriptor, DataTypePtr> outTypeMap;
    Descriptor assemblyOutDesc(ASSEBLY_OUT_SLOT_ID,
        BWAWorker::tr("Assembly URL"),
        BWAWorker::tr("Output assembly URL."));
    
    Descriptor outPortDesc(OUT_PORT_DESCR,
        BWAWorker::tr("BWA output data"),
        BWAWorker::tr("Output assembly files."));

    outTypeMap[assemblyOutDesc] = BaseTypes::STRING_TYPE();

    DataTypePtr outTypeSet(new MapDataType(OUT_TYPE_ID, outTypeMap));
    portDescs << new PortDescriptor(outPortDesc, outTypeSet, false, true);
    
 
     QList<Attribute*> attrs;
     {
         Descriptor outDir(OUTPUT_DIR,
             BWAWorker::tr("Output directory"),
             BWAWorker::tr("Directory to save BWA output files."));

         Descriptor refGenome(REFERENCE_GENOME,
             BWAWorker::tr("Reference genome"),
             BWAWorker::tr("Path to indexed reference genome."));

         Descriptor outName(OUTPUT_NAME,
             BWAWorker::tr("Output file name"),
             BWAWorker::tr("Base name of the output file. 'out.sam' by default"));

         Descriptor useMissProb(USE_MISS_PROB ,
             BWAWorker::tr("Use missing prob"),
             BWAWorker::tr("Use missing prob instead maximum edit distance."));

         Descriptor maxDiff(MAX_DIFF ,
             BWAWorker::tr("Max #diff"),
             BWAWorker::tr("Max #diff (-n)"));

         Descriptor missProb(MISSING_PROB,
             BWAWorker::tr("Missing prob"),
             BWAWorker::tr("Missing prob (-n)"));

         Descriptor seedLength(SEED_LENGTH,
             BWAWorker::tr("Seed length"),
             BWAWorker::tr("Seed length (-l)."));

         Descriptor maxGap(MAX_GAP,
             BWAWorker::tr("Max gap opens"),
             BWAWorker::tr("Max gap opens (-o)."));

         Descriptor indexAlg(INDEX_ALG,
             BWAWorker::tr("Index algorithm"),
             BWAWorker::tr("Index algorithm (-a)."));

         Descriptor bestHits(BEST_HITS,
             BWAWorker::tr("Best hits"),
             BWAWorker::tr("Best hits (-R)."));

         Descriptor longGapPenalty(SCALED_GAP,
             BWAWorker::tr("Long-scaled gap penalty for long deletions"),
             BWAWorker::tr("Long-scaled gap penalty for long deletions (-L)"));

         Descriptor nonIterative(NON_ITERATIVE,
             BWAWorker::tr("Non iterative mode"),
             BWAWorker::tr("Non iterative mode (-N)."));

         Descriptor enableLongGaps(ENABLE_LONG_GAPS,
             BWAWorker::tr("Enable long gaps"),
             BWAWorker::tr("Enable long gaps"));

         Descriptor maxGapExtensions(GAP_EXTENSIONS,
             BWAWorker::tr("Max gap extensions"),
             BWAWorker::tr("Max gap extensions (-e)"));

         Descriptor indelOffset(INDEL_OFFSET,
             BWAWorker::tr("Indel offset"),
             BWAWorker::tr("Indel offset (-i)"));

         Descriptor maxLongDeletions(LONG_DELETIONS, 
             BWAWorker::tr("Max long deletions extensions"),
             BWAWorker::tr("Max long deletions extensions(-d)"));

         Descriptor maxQueue(MAX_QUEUE_ENTRIES,
             BWAWorker::tr("Max queue entries"),
             BWAWorker::tr("Max queue entries (-m)"));

         Descriptor barcodeLength(BARCODE_LENGTH,
             BWAWorker::tr("Barcode length"),
             BWAWorker::tr("Barcode length (-B)"));

         Descriptor numThreads(NUM_THREADS,
             BWAWorker::tr("Threads"),
             BWAWorker::tr("Threads (-t)"));

         Descriptor maxSeedDiff(MAX_SEED_DIFF,
             BWAWorker::tr("Max seed differencies"),
             BWAWorker::tr("Max seed differencies (-k)"));

         Descriptor mismatchPenalty(MISMATCH_PENALTY,
             BWAWorker::tr("Mismatch penalty"),
             BWAWorker::tr("Mismatch penalty (-M)"));

         Descriptor gapOpen(GAP_OPEN_PENALTY,
             BWAWorker::tr("Gap open penalty"),
             BWAWorker::tr("Gap open penalty (-O)."));

         Descriptor gapExt(GAP_EXTENSION_PENALTY,
             BWAWorker::tr("Gap extension penalty"),
             BWAWorker::tr("Gap extension penalty; a gap of size k cost (-E)."));

         Descriptor quolityThreshold(QUALITY_THRESHOLD,
             BWAWorker::tr("Quolity threshold"),
             BWAWorker::tr("Quolity threshold (-q)."));

        attrs << new Attribute(outDir, BaseTypes::STRING_TYPE(), true, QVariant(""));
        attrs << new Attribute(refGenome, BaseTypes::STRING_TYPE(), true, QVariant(""));
        attrs << new Attribute(outName, BaseTypes::STRING_TYPE(), true, QVariant(BASE_BWA_OUTFILE));

        attrs << new Attribute(useMissProb, BaseTypes::BOOL_TYPE(), false, QVariant(true));

        Attribute* maxDiffAttr = new Attribute(maxDiff, BaseTypes::NUM_TYPE(), false, QVariant(0));
        maxDiffAttr->addRelation(new VisibilityRelation(USE_MISS_PROB, QVariant(false)));
        attrs << maxDiffAttr;

        Attribute* missProbAttr = new Attribute(missProb, BaseTypes::NUM_TYPE(), false, QVariant(0.04));
        missProbAttr->addRelation(new VisibilityRelation(USE_MISS_PROB, QVariant(true)));
        attrs << missProbAttr;

        attrs << new Attribute(seedLength, BaseTypes::NUM_TYPE(), false, QVariant(32));
        attrs << new Attribute(maxGap, BaseTypes::NUM_TYPE(), false, QVariant(1));
        attrs << new Attribute(indexAlg, BaseTypes::STRING_TYPE(), false, QVariant("is"));
        attrs << new Attribute(bestHits, BaseTypes::NUM_TYPE(), false, QVariant(30));
        attrs << new Attribute(longGapPenalty, BaseTypes::BOOL_TYPE(), false, QVariant(false));
        attrs << new Attribute(nonIterative, BaseTypes::BOOL_TYPE(), false, QVariant(false));
        attrs << new Attribute(enableLongGaps, BaseTypes::BOOL_TYPE(), false, QVariant(true));

        Attribute* maxGapExtensionsAttr = new Attribute(maxGapExtensions, BaseTypes::NUM_TYPE(), false, QVariant(0));
        maxGapExtensionsAttr->addRelation(new VisibilityRelation(ENABLE_LONG_GAPS, QVariant(true)));
        attrs << maxGapExtensionsAttr;

        attrs << new Attribute(indelOffset, BaseTypes::NUM_TYPE(), false, QVariant(5));
        attrs << new Attribute(maxLongDeletions, BaseTypes::NUM_TYPE(), false, QVariant(10));
        attrs << new Attribute(barcodeLength, BaseTypes::NUM_TYPE(), false, QVariant(0));
        attrs << new Attribute(maxQueue, BaseTypes::NUM_TYPE(), false, QVariant(2000000));
        attrs << new Attribute(numThreads, BaseTypes::NUM_TYPE(), false, QVariant(4));
        attrs << new Attribute(maxSeedDiff, BaseTypes::NUM_TYPE(), false, QVariant(2));
        attrs << new Attribute(mismatchPenalty, BaseTypes::NUM_TYPE(), false, QVariant(3));
        attrs << new Attribute(gapOpen, BaseTypes::NUM_TYPE(), false, QVariant(11));
        attrs << new Attribute(gapExt, BaseTypes::NUM_TYPE(), false, QVariant(4));
        attrs << new Attribute(quolityThreshold, BaseTypes::NUM_TYPE(), false, QVariant(0));
     }

     QMap<QString, PropertyDelegate*> delegates;
     {
         delegates[OUTPUT_DIR] = new URLDelegate("", "", false, true, false);
         delegates[REFERENCE_GENOME] = new URLDelegate("", "", false, false, false);

         QVariantMap spinMap; spinMap["minimum"] = QVariant(0); spinMap["maximum"] = QVariant(INT_MAX);
         delegates[MISSING_PROB]  = new DoubleSpinBoxDelegate(spinMap);
         delegates[MAX_DIFF]  = new SpinBoxDelegate(spinMap);
         delegates[SEED_LENGTH] = new SpinBoxDelegate(spinMap);
         delegates[MAX_GAP] = new SpinBoxDelegate(spinMap);
         delegates[BEST_HITS] = new SpinBoxDelegate(spinMap);
         delegates[GAP_EXTENSIONS] = new DoubleSpinBoxDelegate(spinMap);
         delegates[INDEL_OFFSET] = new SpinBoxDelegate(spinMap);
         delegates[LONG_DELETIONS] = new SpinBoxDelegate(spinMap);
         delegates[MAX_QUEUE_ENTRIES] = new SpinBoxDelegate(spinMap);
         delegates[BARCODE_LENGTH] = new DoubleSpinBoxDelegate(spinMap);
         delegates[NUM_THREADS] = new SpinBoxDelegate(spinMap);
         delegates[MAX_SEED_DIFF] = new SpinBoxDelegate(spinMap);
         delegates[MISMATCH_PENALTY] = new SpinBoxDelegate(spinMap);
         delegates[GAP_OPEN_PENALTY] = new SpinBoxDelegate(spinMap);
         delegates[GAP_EXTENSION_PENALTY] = new SpinBoxDelegate(spinMap);
         delegates[QUALITY_THRESHOLD] = new SpinBoxDelegate(spinMap);

         QVariantMap vm;
         vm["bwtsw"] = "bwtsw";
         vm["div"] = "div";
         vm["is"] = "is";
         delegates[INDEX_ALG] = new ComboBoxDelegate(vm);
    }

    Descriptor protoDesc(BWAWorkerFactory::ACTOR_ID,
        BWAWorker::tr("Align reads with BWA"),
        BWAWorker::tr("Performs alignment of short reads with BWA."));

    ActorPrototype *proto = new IntegralBusActorPrototype(protoDesc, portDescs, attrs);
    proto->setPrompter(new BWAPrompter());
    proto->setEditor(new DelegateEditor(delegates));
    proto->setPortValidator(IN_PORT_DESCR, new BWAInputSlotsValidator());
    proto->addExternalTool(ET_BWA);
    WorkflowEnv::getProtoRegistry()->registerProto(BaseActorCategories::CATEGORY_ASSEMBLY(), proto);
    WorkflowEnv::getDomainRegistry()->getById(LocalDomainFactory::ID)->registerEntry(new BWAWorkerFactory());
}

Worker *BWAWorkerFactory::createWorker(Actor *a) {
    return new BWAWorker(a);
}

QString BWAPrompter::composeRichDoc() {
    QString res = ""; 

    Actor* readsProducer = qobject_cast<IntegralBusPort*>(target->getPort(IN_PORT_DESCR))->getProducer(READS_URL_SLOT_ID);

    QString unsetStr = "<font color='red'>"+tr("unset")+"</font>";
    QString readsUrl = readsProducer ? readsProducer->getLabel() : unsetStr;
    QString genome = getHyperlink(REFERENCE_GENOME, getURL(REFERENCE_GENOME));

    res.append(tr("Aligns reads from <u>%1</u> ").arg(readsUrl));
    res.append(tr(" to reference genome <u>%1</u>.").arg(genome));
 
    return res;
}

} // LocalWorkflow
} // U2

