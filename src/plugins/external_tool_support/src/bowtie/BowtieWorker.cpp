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

#include <U2Algorithm/DnaAssemblyTask.h>

#include <U2Core/FailTask.h>
#include <U2Core/U2OpStatusUtils.h>
#include <U2Core/U2SafePoints.h>
#include <U2Core/QVariantUtils.h>
#include <U2Core/GUrlUtils.h>

#include <U2Designer/DelegateEditors.h>

#include <U2Gui/DialogUtils.h>

#include <U2Lang/ActorPrototypeRegistry.h>
#include <U2Lang/BaseActorCategories.h>
#include <U2Lang/BaseTypes.h>
#include <U2Lang/WorkflowEnv.h>
#include <U2Lang/WorkflowMonitor.h>

#include "BowtieSupport.h"
#include "BowtieWorker.h"
#include "BowtieTask.h"

namespace U2 {
namespace LocalWorkflow {

const QString BowtieWorkerFactory::ACTOR_ID("align-reads-with-bowtie");

static const QString READS_URL_SLOT_ID("readsurl");
static const QString READS_PAIRED_URL_SLOT_ID("readspairedurl");

static const QString IN_TYPE_ID("Bowtie-data");
static const QString OUT_TYPE_ID("Bowtie-data-out");

static const QString ASSEBLY_OUT_SLOT_ID("assembly-out");

static const QString IN_PORT_DESCR("in-data");
static const QString OUT_PORT_DESCR("out-data");

static const QString OUTPUT_DIR("output-dir");
static const QString REFERENCE_GENOME("reference");

static const QString OUTPUT_NAME = "outname";

static const QString MISMATCHES_TYPE = "mismatches_type";
static const QString MISMATCHES_NUMBER = "mismatches_number";
static const QString MAQERR = "maqerr";
static const QString SEED_LEN = "seedLen";
static const QString NOFW = "nofw";
static const QString NORC = "norc";
static const QString MAXBTS = "maxbts";
static const QString TRYHARD = "tryhard";
static const QString CHUNKMBS = "chunkmbs";
static const QString NOMAQROUND = "nomaqround";
static const QString SEED = "seed";
static const QString BEST = "best";
static const QString ALL = "all";
static const QString COLORSPACE = "colorspace";

static const QString BASE_Bowtie_SUBDIR("bowtie");
static const QString BASE_Bowtie_OUTFILE("out.sam");


/************************************************************************/
/* Worker */
/************************************************************************/
BowtieWorker::BowtieWorker(Actor *p)
: BaseWorker(p)
, inChannel(NULL)
, output(NULL)
{

}

void BowtieWorker::init() {
    inChannel = ports.value(IN_PORT_DESCR);
    output = ports.value(OUT_PORT_DESCR);
}

Task *BowtieWorker::tick() {
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

        BowtieTask* t = new BowtieTask(settings);
        connect(t, SIGNAL(si_stateChanged()), SLOT(sl_taskFinished()));
        return t;
    }else if (inChannel->isEnded()) {
        setDone();
        output->setEnded();
    }
    return NULL;
}

void BowtieWorker::cleanup() {

}

void BowtieWorker::sl_taskFinished() {
    BowtieTask *t = dynamic_cast<BowtieTask*>(sender());
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

DnaAssemblyToRefTaskSettings BowtieWorker::getSettings( U2OpStatus &os ){
    DnaAssemblyToRefTaskSettings settings;

    QString outDir = GUrlUtils::createDirectory(
        getValue<QString>(OUTPUT_DIR) + QDir::separator() + BASE_Bowtie_SUBDIR,
        "_", os);
    CHECK_OP(os, settings);
    
    if (!outDir.endsWith(QDir::separator())){
        outDir  = outDir + QDir::separator();
    }

    QString outFileName = getValue<QString>(OUTPUT_NAME);
    if(outFileName.isEmpty()){
        outFileName = BASE_Bowtie_OUTFILE;
    }
    settings.resultFileName = outDir + outFileName;
    
    QString refGenome = getValue<QString>(REFERENCE_GENOME);
    settings.prebuiltIndex = !refGenome.contains(".fa");
    settings.indexFileName = refGenome;
    settings.refSeqUrl = refGenome;

    settings.algName = BowtieTask::taskName;

    QMap<QString, QVariant> customSettings;

    QString mismatchesType = getValue<QString>(MISMATCHES_TYPE);
    if(mismatchesType == "-n mode") {
        customSettings.insert(BowtieTask::OPTION_N_MISMATCHES, getValue<int>(MISMATCHES_NUMBER));
    } else {
        customSettings.insert(BowtieTask::OPTION_V_MISMATCHES, getValue<int>(MISMATCHES_NUMBER));
    }

    customSettings.insert(BowtieTask::OPTION_MAQERR, getValue<int>(MAQERR));
    customSettings.insert(BowtieTask::OPTION_MAXBTS, getValue<int>(MAXBTS));
    customSettings.insert(BowtieTask::OPTION_SEED_LEN, getValue<int>(SEED_LEN));
    customSettings.insert(BowtieTask::OPTION_CHUNKMBS, getValue<int>(CHUNKMBS));
    customSettings.insert(BowtieTask::OPTION_SEED, getValue<int>(SEED));

    customSettings.insert(BowtieTask::OPTION_NOFW, getValue<bool>(NOFW));
    customSettings.insert(BowtieTask::OPTION_NORC, getValue<bool>(NORC));
    customSettings.insert(BowtieTask::OPTION_TRYHARD, getValue<bool>(TRYHARD));
    customSettings.insert(BowtieTask::OPTION_BEST, getValue<bool>(BEST));
    customSettings.insert(BowtieTask::OPTION_ALL, getValue<bool>(ALL));
    customSettings.insert(BowtieTask::OPTION_NOMAQROUND, getValue<bool>(NOMAQROUND));
    customSettings.insert(BowtieTask::OPTION_COLORSPACE, getValue<bool>(COLORSPACE));
    customSettings.insert(BowtieTask::OPTION_THREADS, 4);

    settings.setCustomSettings(customSettings);

    return settings;
}

/************************************************************************/
/* Factory */
/************************************************************************/
class BowtieInputSlotsValidator : public PortValidator {
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
            problemList.append(Problem(BowtieWorker::tr("Bowtie cannot recognize read pairs from the same file. Please, perform demultiplexing first.")));
            return false;
        }

        return true;
    }
    };

void BowtieWorkerFactory::init() {
    QList<PortDescriptor*> portDescs;
    
    //in port
    QMap<Descriptor, DataTypePtr> inTypeMap;
    Descriptor readsDesc(READS_URL_SLOT_ID,
        BowtieWorker::tr("URL of a file with reads"),
        BowtieWorker::tr("Input reads to be aligned."));
    Descriptor readsPairedDesc(READS_PAIRED_URL_SLOT_ID,
        BowtieWorker::tr("URL of a file with mate reads"),
        BowtieWorker::tr("Input mate reads to be aligned."));

    inTypeMap[readsDesc] = BaseTypes::STRING_TYPE();
    inTypeMap[readsPairedDesc] = BaseTypes::STRING_TYPE();

    Descriptor inPortDesc(IN_PORT_DESCR,
        BowtieWorker::tr("Bowtie data"),
        BowtieWorker::tr("Input reads to be aligned with Bowtie."));

    DataTypePtr inTypeSet(new MapDataType(IN_TYPE_ID, inTypeMap));
    portDescs << new PortDescriptor(inPortDesc, inTypeSet, true);

    //out port
    QMap<Descriptor, DataTypePtr> outTypeMap;
    Descriptor assemblyOutDesc(ASSEBLY_OUT_SLOT_ID,
        BowtieWorker::tr("Assembly URL"),
        BowtieWorker::tr("Output assembly URL."));
    
    Descriptor outPortDesc(OUT_PORT_DESCR,
        BowtieWorker::tr("Bowtie output data"),
        BowtieWorker::tr("Output assembly files."));

    outTypeMap[assemblyOutDesc] = BaseTypes::STRING_TYPE();

    DataTypePtr outTypeSet(new MapDataType(OUT_TYPE_ID, outTypeMap));
    portDescs << new PortDescriptor(outPortDesc, outTypeSet, false, true);
    
 
     QList<Attribute*> attrs;
     {
         Descriptor outDir(OUTPUT_DIR,
             BowtieWorker::tr("Output directory"),
             BowtieWorker::tr("Directory to save Bowtie output files."));

         Descriptor refGenome(REFERENCE_GENOME,
             BowtieWorker::tr("Reference genome"),
             BowtieWorker::tr("Path to indexed reference genome."));

         Descriptor outName(OUTPUT_NAME,
             BowtieWorker::tr("Output file name"),
             BowtieWorker::tr("Base name of the output file. 'out.sam' by default"));

         static const QString MISMATCHES_TYPE = "mismatches_type";
         static const QString N_MISMATCHES = "n-mismatches";
         static const QString V_MISMATCHES = "v-mismatches";
         static const QString MAQERR = "maqerr";
         static const QString SEED_LEN = "seedLen";
         static const QString NOFW = "nofw";
         static const QString NORC = "norc";
         static const QString MAXBTS = "maxbts";
         static const QString TRYHARD = "tryhard";
         static const QString CHUNKMBS = "chunkmbs";
         static const QString NOMAQROUND = "nomaqround";
         static const QString SEED = "seed";
         static const QString BEST = "best";
         static const QString ALL = "all";
         static const QString COLORSPACE = "colorspace";
         static const QString THREADS = "threads";


         Descriptor mismatchesType(MISMATCHES_TYPE ,
             BowtieWorker::tr("Mode:"),
             BowtieWorker::tr("When the -n option is specified (which is the default), bowtie determines which alignments \
                              are valid according to the following policy, which is similar to Maq's default policy. \
                              In -v mode, alignments may have no more than V mismatches, where V may be a number from 0 \
                              through 3 set using the -v option. Quality values are ignored. The -v option is mutually exclusive with the -n option."));

        Descriptor mismatchesNumber(MISMATCHES_NUMBER,
            BowtieWorker::tr("Mismatches number"),
            BowtieWorker::tr("Mismatches number."));

        Descriptor maqError(MAQERR,
            BowtieWorker::tr("Mismatches number"),
            BowtieWorker::tr("Maximum permitted total of quality values at all mismatched read positions throughout the entire alignment, \
                             not just in the seed. The default is 70. Like Maq, bowtie rounds quality values to the nearest 10 and saturates at 30; \
                             rounding can be disabled with --nomaqround."));

         Descriptor seedLen(SEED_LEN,
             BowtieWorker::tr("Seed length"),
             BowtieWorker::tr("The ""seed length""; i.e., the number of bases on the high-quality end of the read to which the \
                              -n ceiling applies. The lowest permitted setting is 5 and the default is 28. bowtie is faster for larger values of -l."));

         Descriptor noForward(NOFW,
             BowtieWorker::tr("No forward orientation"),
             BowtieWorker::tr("If --nofw is specified, bowtie will not attempt to align against the forward reference strand."));

         Descriptor noReverse(NORC,
             BowtieWorker::tr("No reverse-complement orientation"),
             BowtieWorker::tr("If --norc is specified, bowtie will not attempt to align against the reverse-complement reference strand."));

         Descriptor maxBacktracks(MAXBTS,
             BowtieWorker::tr("Maximum of backtracks"),
             BowtieWorker::tr("The maximum insert size for valid paired-end alignments. E.g. if -X 100 is specified and a paired-end alignment \
                              consists of two 20-bp alignments in the proper orientation with a 60-bp gap between them, that alignment is \
                              considered valid (as long as -I is also satisfied). A 61-bp gap would not be valid in that case.  \
                              If trimming options -3 or -5 are also used, the -X constraint is applied with respect to the untrimmed mates, \
                              not the trimmed mates. Default: 250."));

         Descriptor tryHard(TRYHARD,
             BowtieWorker::tr("Try as hard"),
             BowtieWorker::tr("Try as hard as possible to find valid alignments when they exist, including paired-end alignments. \
                               This is equivalent to specifying very high values for the --maxbts and --pairtries options. This mode is generally much \
                               slower than the default settings, but can be useful for certain problems. This mode is slower when (a) the reference is \
                               very repetitive, (b) the reads are low quality, or (c) not many reads have valid alignments."));

         Descriptor chunkmbs(CHUNKMBS,
             BowtieWorker::tr("Best hits"),
             BowtieWorker::tr("The number of megabytes of memory a given thread is given to store path descriptors in --best mode. Best-first \
                               search must keep track of many paths at once to ensure it is always extending the path with the lowest cumulative cost. \
                               Bowtie tries to minimize the memory impact of the descriptors, but they can still grow very large in some cases. \
                               If you receive an error message saying that chunk memory has been exhausted in --best mode, \
                               try adjusting this parameter up to dedicate more memory to the descriptors. Default: 64."));

         Descriptor noMaqRounding(NOMAQROUND,
             BowtieWorker::tr("No Maq rounding"),
             BowtieWorker::tr("Maq accepts quality values in the Phred quality scale, but internally rounds values to the nearest 10, \
             with a maximum of 30. By default, bowtie also rounds this way. --nomaqround prevents this rounding in bowtie."));

         Descriptor seed(SEED,
             BowtieWorker::tr("Seed"),
             BowtieWorker::tr("Use <int> as the seed for pseudo-random number generator."));

         Descriptor best(BEST,
             BowtieWorker::tr("Best alignments"),
             BowtieWorker::tr("Make Bowtie guarantee that reported singleton alignments are ""best"" in terms of stratum \
                             (i.e. number of mismatches, or mismatches in the seed in the case of -n mode) and in terms of \
                             the quality values at the mismatched position(s). bowtie is somewhat slower when --best is specified."));

         Descriptor allAln(ALL,
             BowtieWorker::tr("All alignments"),
             BowtieWorker::tr("Report all valid alignments per read or pair."));

         Descriptor colorspace(COLORSPACE,
             BowtieWorker::tr("Colorspace"),
             BowtieWorker::tr("When -C is specified, read sequences are treated as colors. Colors may be encoded either as numbers \
                              (0=blue, 1=green, 2=orange, 3=red) or as characters A/C/G/T (A=blue, C=green, G=orange, T=red)."));

        attrs << new Attribute(outDir, BaseTypes::STRING_TYPE(), true, QVariant(""));
        attrs << new Attribute(refGenome, BaseTypes::STRING_TYPE(), true, QVariant(""));
        attrs << new Attribute(outName, BaseTypes::STRING_TYPE(), true, QVariant(BASE_Bowtie_OUTFILE));

        attrs << new Attribute(mismatchesType, BaseTypes::STRING_TYPE(), false, QVariant("-n mode"));
        attrs << new Attribute(mismatchesNumber, BaseTypes::NUM_TYPE(), false, QVariant(2));
        attrs << new Attribute(maqError, BaseTypes::NUM_TYPE(), false, QVariant(70));

        attrs << new Attribute(seedLen, BaseTypes::NUM_TYPE(), false, QVariant(28));
        attrs << new Attribute(maxBacktracks, BaseTypes::NUM_TYPE(), false, QVariant(800));
        attrs << new Attribute(chunkmbs, BaseTypes::NUM_TYPE(), false, QVariant(64));
        attrs << new Attribute(seed, BaseTypes::NUM_TYPE(), false, QVariant(0));

        attrs << new Attribute(colorspace, BaseTypes::BOOL_TYPE(), false, QVariant(false));
        attrs << new Attribute(noMaqRounding, BaseTypes::BOOL_TYPE(), false, QVariant(false));
        attrs << new Attribute(noForward, BaseTypes::BOOL_TYPE(), false, QVariant(false));
        attrs << new Attribute(noReverse, BaseTypes::BOOL_TYPE(), false, QVariant(false));
        attrs << new Attribute(tryHard, BaseTypes::BOOL_TYPE(), false, QVariant(false));
        attrs << new Attribute(best, BaseTypes::BOOL_TYPE(), false, QVariant(false));
        attrs << new Attribute(allAln, BaseTypes::BOOL_TYPE(), false, QVariant(false));
     }

     QMap<QString, PropertyDelegate*> delegates;
     {
         delegates[OUTPUT_DIR] = new URLDelegate("", "", false, true, false);
         delegates[REFERENCE_GENOME] = new URLDelegate("", "", false, false, false);

         QVariantMap spinMap; spinMap["minimum"] = QVariant(0); spinMap["maximum"] = QVariant(INT_MAX);
         delegates[MISMATCHES_NUMBER]  = new SpinBoxDelegate(spinMap);
         delegates[MAQERR] = new SpinBoxDelegate(spinMap);
         delegates[SEED_LEN] = new SpinBoxDelegate(spinMap);
         delegates[SEED] = new SpinBoxDelegate(spinMap);
         delegates[MAXBTS] = new SpinBoxDelegate(spinMap);
         delegates[CHUNKMBS] = new SpinBoxDelegate(spinMap);

         QVariantMap vm;
         vm["-n mode"] = "-n mode";
         vm["-v mode"] = "-v mode";
         delegates[MISMATCHES_TYPE] = new ComboBoxDelegate(vm);
    }

    Descriptor protoDesc(BowtieWorkerFactory::ACTOR_ID,
        BowtieWorker::tr("Align Reads with Bowtie"),
        BowtieWorker::tr("Performs alignment of short reads with Bowtie."));

    ActorPrototype *proto = new IntegralBusActorPrototype(protoDesc, portDescs, attrs);
    proto->setPrompter(new BowtiePrompter());
    proto->setEditor(new DelegateEditor(delegates));
    proto->setPortValidator(IN_PORT_DESCR, new BowtieInputSlotsValidator());
    proto->addExternalTool(ET_BOWTIE);
    WorkflowEnv::getProtoRegistry()->registerProto(BaseActorCategories::CATEGORY_NGS_ALIGN_SHORT_READS(), proto);
    WorkflowEnv::getDomainRegistry()->getById(LocalDomainFactory::ID)->registerEntry(new BowtieWorkerFactory());
}

Worker *BowtieWorkerFactory::createWorker(Actor *a) {
    return new BowtieWorker(a);
}

QString BowtiePrompter::composeRichDoc() {
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

