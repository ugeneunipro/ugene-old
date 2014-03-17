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

const QString BWAMEMWorkerFactory::ACTOR_ID("bwamem-id");

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
static const QString THREADS = "threads";
static const QString MIN_SEED = "min-seed";
static const QString BAND_WIDTH = "band-width";
static const QString DROPOFF = "dropoff";
static const QString INTERNAL_SEED_LOOKUP = "seed-lookup";
static const QString SKIP_SEED_THRESHOLD = "seed-threshold";
static const QString DROP_CHAINS_THRESHOLD = "drop-chains";
static const QString MAX_MATE_RESCUES = "mate-rescue";
static const QString SKIP_MATE_RESCUES = "skip-mate-rescues";
static const QString SKIP_PAIRING = "skip-pairing";
static const QString SKIP_MISTMATCH_SCORE = "skip-score";
static const QString MATCH_SCORE = "match-score";
static const QString MISMATCH_PENALTY = "mistmatch-penalty";
static const QString GAP_OPEN_PENALTY = "gap-open-penalty";
static const QString GAP_EXTENSION_PENALTY = "gap-ext-penalty";
static const QString CLIPPING_PENALTY = "clipping-penalty";
static const QString UNPAIRED_PENALTY = "inpaired-panalty";
static const QString SCORE_THRESHOLD = "score-threshold";

static const QString BASE_BWA_SUBDIR("bwa");
static const QString BASE_BWA_OUTFILE("out.sam");


/************************************************************************/
/* Worker */
/************************************************************************/
BWAMEMWorker::BWAMEMWorker(Actor *p)
: BaseWorker(p)
, inChannel(NULL)
, output(NULL)
{

}

void BWAMEMWorker::init() {
    inChannel = ports.value(IN_PORT_DESCR);
    output = ports.value(OUT_PORT_DESCR);
}

Task *BWAMEMWorker::tick() {
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

void BWAMEMWorker::cleanup() {

}

void BWAMEMWorker::sl_taskFinished() {
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

DnaAssemblyToRefTaskSettings BWAMEMWorker::getSettings( U2OpStatus &os ){
    DnaAssemblyToRefTaskSettings settings;

    settings.prebuiltIndex = true;

    QString outDir = GUrlUtils::createDirectory(
        getValue<QString>(OUTPUT_DIR) + "/" + BASE_BWA_SUBDIR,
        "_", os);
    CHECK_OP(os, settings);
    
    if (!outDir.endsWith("/")){
        outDir  = outDir + "/";
    }

    QString outFileName = getValue<QString>(OUTPUT_NAME);
    if(outFileName.isEmpty()){
        outFileName = BASE_BWA_OUTFILE;
    }
    settings.resultFileName = outDir + outFileName;
    
    settings.indexFileName = getValue<QString>(REFERENCE_GENOME);
    settings.refSeqUrl = GUrl(settings.indexFileName).baseFileName();
    settings.algName = BwaTask::ALGORITHM_BWA_MEM;

    QMap<QString, QVariant> customSettings;

    customSettings.insert(BwaTask::OPTION_THREADS, getValue<int>(THREADS));
    customSettings.insert(BwaTask::OPTION_MIN_SEED, getValue<int>(MIN_SEED));
    customSettings.insert(BwaTask::OPTION_BAND_WIDTH, getValue<int>(BAND_WIDTH));
    customSettings.insert(BwaTask::OPTION_DROPOFF, getValue<int>(DROPOFF));
    customSettings.insert(BwaTask::OPTION_INTERNAL_SEED_LOOKUP, getValue<float>(INTERNAL_SEED_LOOKUP));
    customSettings.insert(BwaTask::OPTION_SKIP_SEED_THRESHOLD, getValue<int>(SKIP_SEED_THRESHOLD));
    customSettings.insert(BwaTask::OPTION_DROP_CHAINS_THRESHOLD, getValue<float>(DROP_CHAINS_THRESHOLD));
    customSettings.insert(BwaTask::OPTION_MAX_MATE_RESCUES, getValue<int>(MAX_MATE_RESCUES));

    if (getValue<bool>(SKIP_MATE_RESCUES)) {
        customSettings.insert(BwaTask::OPTION_SKIP_MATE_RESCUES, true);
    }
    if (getValue<bool>(SKIP_PAIRING)) {
        customSettings.insert(BwaTask::OPTION_SKIP_PAIRING, true);
    }

    customSettings.insert(BwaTask::OPTION_MATCH_SCORE, getValue<int>(MATCH_SCORE));
    customSettings.insert(BwaTask::OPTION_MISMATCH_PENALTY, getValue<int>(MISMATCH_PENALTY));
    customSettings.insert(BwaTask::OPTION_GAP_OPEN_PENALTY, getValue<int>(GAP_OPEN_PENALTY));
    customSettings.insert(BwaTask::OPTION_GAP_EXTENSION_PENALTY, getValue<int>(GAP_EXTENSION_PENALTY));
    customSettings.insert(BwaTask::OPTION_CLIPPING_PENALTY, getValue<int>(CLIPPING_PENALTY));
    customSettings.insert(BwaTask::OPTION_UNPAIRED_PENALTY, getValue<int>(UNPAIRED_PENALTY));
    customSettings.insert(BwaTask::OPTION_SCORE_THRESHOLD, getValue<int>(SCORE_THRESHOLD));

    customSettings.insert(BwaTask::OPTION_MEM_ALIGNMENT,true);

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
            problemList.append(Problem(BWAMEMWorker::tr("BWA MEM cannot recognize read pairs from the same file. Please, perform demultiplexing first.")));
            return false;
        }

        return true;
    }
    };

void BWAMEMWorkerFactory::init() {
    QList<PortDescriptor*> portDescs;
    
    //in port
    QMap<Descriptor, DataTypePtr> inTypeMap;
    Descriptor readsDesc(READS_URL_SLOT_ID,
        BWAMEMWorker::tr("URL of a file with reads"),
        BWAMEMWorker::tr("Input reads to be aligned."));
    Descriptor readsPairedDesc(READS_PAIRED_URL_SLOT_ID,
        BWAMEMWorker::tr("URL of a file with mate reads"),
        BWAMEMWorker::tr("Input mate reads to be aligned."));

    inTypeMap[readsDesc] = BaseTypes::STRING_TYPE();
    inTypeMap[readsPairedDesc] = BaseTypes::STRING_TYPE();

    Descriptor inPortDesc(IN_PORT_DESCR,
        BWAMEMWorker::tr("BWA data"),
        BWAMEMWorker::tr("Input reads to be aligned with BWA MEM."));

    DataTypePtr inTypeSet(new MapDataType(IN_TYPE_ID, inTypeMap));
    portDescs << new PortDescriptor(inPortDesc, inTypeSet, true);

    //out port
    QMap<Descriptor, DataTypePtr> outTypeMap;
    Descriptor assemblyOutDesc(ASSEBLY_OUT_SLOT_ID,
        BWAMEMWorker::tr("Assembly URL"),
        BWAMEMWorker::tr("Output assembly URL."));
    
    Descriptor outPortDesc(OUT_PORT_DESCR,
        BWAMEMWorker::tr("BWA MEM output data"),
        BWAMEMWorker::tr("Output assembly files."));

    outTypeMap[assemblyOutDesc] = BaseTypes::STRING_TYPE();

    DataTypePtr outTypeSet(new MapDataType(OUT_TYPE_ID, outTypeMap));
    portDescs << new PortDescriptor(outPortDesc, outTypeSet, false, true);
    
 
     QList<Attribute*> attrs;
     {
         Descriptor outDir(OUTPUT_DIR,
             BWAMEMWorker::tr("Output directory"),
             BWAMEMWorker::tr("Directory to save BWA MEM output files."));

         Descriptor refGenome(REFERENCE_GENOME,
             BWAMEMWorker::tr("Reference genome"),
             BWAMEMWorker::tr("Path to indexed reference genome."));

         Descriptor outName(OUTPUT_NAME,
             BWAMEMWorker::tr("Output file name"),
             BWAMEMWorker::tr("Base name of the output file. 'out.sam' by default"));

         Descriptor threads(THREADS,
             BWAMEMWorker::tr("Number of threads"),
             BWAMEMWorker::tr("Number of threads (-t)."));

         Descriptor minSeed(MIN_SEED,
             BWAMEMWorker::tr("Min seed length"),
             BWAMEMWorker::tr("Path to indexed reference genome (-k)."));

         Descriptor bandWidth(BAND_WIDTH,
             BWAMEMWorker::tr("Band width"),
             BWAMEMWorker::tr("Band width for banded alignment (-w)."));

         Descriptor dropoff(DROPOFF,
             BWAMEMWorker::tr("Dropoff"),
             BWAMEMWorker::tr("Off-diagonal X-dropoff (-d)."));

         Descriptor internalSeed(INTERNAL_SEED_LOOKUP,
             BWAMEMWorker::tr("Internal seed length"),
             BWAMEMWorker::tr("Look for internal seeds inside a seed longer than {-k} (-r)."));

         Descriptor skipSeed(SKIP_SEED_THRESHOLD,
             BWAMEMWorker::tr("Skip seed threshold"),
             BWAMEMWorker::tr("Skip seeds with more than INT occurrences (-c)."));

         Descriptor dropChains(DROP_CHAINS_THRESHOLD,
             BWAMEMWorker::tr("Drop chain threshold"),
             BWAMEMWorker::tr("Drop chains shorter than FLOAT fraction of the longest overlapping chain (-D)."));

         Descriptor maxMate(MAX_MATE_RESCUES,
             BWAMEMWorker::tr("Rounds of mate rescues"),
             BWAMEMWorker::tr("Perform at most INT rounds of mate rescues for each read (-m)."));

         Descriptor skipMate(SKIP_MATE_RESCUES,
             BWAMEMWorker::tr("Skip mate rescue"),
             BWAMEMWorker::tr("Skip mate rescue (-S)."));

         Descriptor skipPairing(SKIP_PAIRING,
             BWAMEMWorker::tr("Skip pairing"),
             BWAMEMWorker::tr("Skip pairing; mate rescue performed unless -S also in use (-P)."));

         Descriptor matchScore(MATCH_SCORE,
             BWAMEMWorker::tr("Mismatch penalty"),
             BWAMEMWorker::tr("Score for a sequence match (-A)."));

         Descriptor mismatchPenalty(MISMATCH_PENALTY,
             BWAMEMWorker::tr("Mismatch penalty"),
             BWAMEMWorker::tr("Penalty for a mismatch (-B)."));

         Descriptor gapOpen(GAP_OPEN_PENALTY,
             BWAMEMWorker::tr("Gap open penalty"),
             BWAMEMWorker::tr("Gap open penalty (-O)."));

         Descriptor gapExt(GAP_EXTENSION_PENALTY,
             BWAMEMWorker::tr("Gap extension penalty"),
             BWAMEMWorker::tr("Gap extension penalty; a gap of size k cost {-O} (-E)."));

         Descriptor clippingPen(CLIPPING_PENALTY,
             BWAMEMWorker::tr("Penalty for clipping"),
             BWAMEMWorker::tr("Penalty for clipping (-L)."));

         Descriptor unpairedPenalty(UNPAIRED_PENALTY,
             BWAMEMWorker::tr("Penalty unpaired"),
             BWAMEMWorker::tr("Penalty for an unpaired read pair (-U)."));

         Descriptor scoreThreshold(SCORE_THRESHOLD,
             BWAMEMWorker::tr("Score threshold"),
             BWAMEMWorker::tr("Minimum score to output (-T)."));

        attrs << new Attribute(outDir, BaseTypes::STRING_TYPE(), true, QVariant(""));
        attrs << new Attribute(refGenome, BaseTypes::STRING_TYPE(), true, QVariant(""));
        attrs << new Attribute(outName, BaseTypes::STRING_TYPE(), true, QVariant(BASE_BWA_OUTFILE));

        attrs << new Attribute(threads, BaseTypes::NUM_TYPE(), false, QVariant(1));
        attrs << new Attribute(minSeed, BaseTypes::NUM_TYPE(), false, QVariant(19));
        attrs << new Attribute(bandWidth, BaseTypes::NUM_TYPE(), false, QVariant(100));
        attrs << new Attribute(dropoff, BaseTypes::NUM_TYPE(), false, QVariant(100));
        attrs << new Attribute(internalSeed, BaseTypes::NUM_TYPE(), false, QVariant(1.5));
        attrs << new Attribute(skipSeed, BaseTypes::NUM_TYPE(), false, QVariant(10000));
        attrs << new Attribute(dropChains, BaseTypes::NUM_TYPE(), false, QVariant(0.5));
        attrs << new Attribute(maxMate, BaseTypes::NUM_TYPE(), false, QVariant(100));
        attrs << new Attribute(skipMate, BaseTypes::BOOL_TYPE(), false, QVariant(false));
        attrs << new Attribute(skipPairing, BaseTypes::BOOL_TYPE(), false, QVariant(false));
        attrs << new Attribute(matchScore, BaseTypes::NUM_TYPE(), false, QVariant(1));
        attrs << new Attribute(mismatchPenalty, BaseTypes::NUM_TYPE(), false, QVariant(4));
        attrs << new Attribute(gapOpen, BaseTypes::NUM_TYPE(), false, QVariant(6));
        attrs << new Attribute(gapExt, BaseTypes::NUM_TYPE(), false, QVariant(1));
        attrs << new Attribute(clippingPen, BaseTypes::NUM_TYPE(), false, QVariant(5));
        attrs << new Attribute(unpairedPenalty, BaseTypes::NUM_TYPE(), false, QVariant(17));
        attrs << new Attribute(scoreThreshold, BaseTypes::NUM_TYPE(), false, QVariant(30));
     }

     QMap<QString, PropertyDelegate*> delegates;
     {
         delegates[OUTPUT_DIR] = new URLDelegate("", "", false, true, false);
         delegates[REFERENCE_GENOME] = new URLDelegate("", "", false, false, false);

         QVariantMap spinMap; spinMap["minimum"] = QVariant(1); spinMap["maximum"] = QVariant(INT_MAX);
         delegates[THREADS]  = new SpinBoxDelegate(spinMap);
         delegates[MIN_SEED] = new SpinBoxDelegate(spinMap);
         delegates[BAND_WIDTH] = new SpinBoxDelegate(spinMap);
         delegates[DROPOFF] = new SpinBoxDelegate(spinMap);
         delegates[INTERNAL_SEED_LOOKUP] = new DoubleSpinBoxDelegate(spinMap);
         delegates[SKIP_MATE_RESCUES] = new SpinBoxDelegate(spinMap);
         delegates[DROP_CHAINS_THRESHOLD] = new DoubleSpinBoxDelegate(spinMap);
         delegates[MAX_MATE_RESCUES] = new SpinBoxDelegate(spinMap);
         delegates[MATCH_SCORE] = new SpinBoxDelegate(spinMap);
         delegates[MISMATCH_PENALTY] = new SpinBoxDelegate(spinMap);
         delegates[GAP_OPEN_PENALTY] = new SpinBoxDelegate(spinMap);
         delegates[GAP_EXTENSION_PENALTY] = new SpinBoxDelegate(spinMap);
         delegates[CLIPPING_PENALTY] = new SpinBoxDelegate(spinMap);
         delegates[UNPAIRED_PENALTY] = new SpinBoxDelegate(spinMap);
         delegates[SCORE_THRESHOLD] = new SpinBoxDelegate(spinMap);
    }

    Descriptor protoDesc(BWAMEMWorkerFactory::ACTOR_ID,
        BWAMEMWorker::tr("Align reads with BWA MEM"),
        BWAMEMWorker::tr("Performs alignment of short reads with BWA MEM."));

    ActorPrototype *proto = new IntegralBusActorPrototype(protoDesc, portDescs, attrs);
    proto->setPrompter(new BWAMEMPrompter());
    proto->setEditor(new DelegateEditor(delegates));
    proto->setPortValidator(IN_PORT_DESCR, new BWAInputSlotsValidator());
    proto->addExternalTool(ET_BWA);
    WorkflowEnv::getProtoRegistry()->registerProto(BaseActorCategories::CATEGORY_ASSEMBLY(), proto);
    WorkflowEnv::getDomainRegistry()->getById(LocalDomainFactory::ID)->registerEntry(new BWAMEMWorkerFactory());
}

Worker *BWAMEMWorkerFactory::createWorker(Actor *a) {
    return new BWAMEMWorker(a);
}

QString BWAMEMPrompter::composeRichDoc() {
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

