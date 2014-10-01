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

#include "Bowtie2Support.h"
#include "Bowtie2Worker.h"
#include "Bowtie2Task.h"

namespace U2 {
namespace LocalWorkflow {

const QString Bowtie2WorkerFactory::ACTOR_ID("align-reads-with-bowtie2");

static const QString READS_URL_SLOT_ID("readsurl");
static const QString READS_PAIRED_URL_SLOT_ID("readspairedurl");

static const QString IN_TYPE_ID("Bowtie2-data");
static const QString OUT_TYPE_ID("Bowtie2-data-out");

static const QString ASSEBLY_OUT_SLOT_ID("assembly-out");

static const QString IN_PORT_DESCR("in-data");
static const QString OUT_PORT_DESCR("out-data");

static const QString OUTPUT_DIR("output-dir");
static const QString REFERENCE_GENOME("reference");

static const QString OUTPUT_NAME = "outname";

static const QString MODE = "mode";
static const QString MISMATCHES_NUMBER = "mismatches_number";
static const QString SEED_LEN = "seed_len";
static const QString DPAD = "dpad";
static const QString GBAR = "gbar";
static const QString SEED = "seed";
static const QString THREADS = "threads";
static const QString NOMIXED = "nomixed";
static const QString NODISCORDANT = "nodiscordant";
static const QString NOFW = "nofw";
static const QString NORC = "norc";
static const QString NOOVERLAP = "nooverlap";
static const QString NOCONTAIN = "nocontain";

static const QString BASE_Bowtie2_SUBDIR("bowtie");
static const QString BASE_Bowtie2_OUTFILE("out.sam");


/************************************************************************/
/* Worker */
/************************************************************************/
Bowtie2Worker::Bowtie2Worker(Actor *p)
: BaseWorker(p)
, inChannel(NULL)
, output(NULL)
{

}

void Bowtie2Worker::init() {
    inChannel = ports.value(IN_PORT_DESCR);
    output = ports.value(OUT_PORT_DESCR);
}

Task *Bowtie2Worker::tick() {
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

        Bowtie2Task* t = new Bowtie2Task(settings);
        connect(t, SIGNAL(si_stateChanged()), SLOT(sl_taskFinished()));
        return t;
    }else if (inChannel->isEnded()) {
        setDone();
        output->setEnded();
    }
    return NULL;
}

void Bowtie2Worker::cleanup() {

}

void Bowtie2Worker::sl_taskFinished() {
    Bowtie2Task *t = dynamic_cast<Bowtie2Task*>(sender());
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

DnaAssemblyToRefTaskSettings Bowtie2Worker::getSettings( U2OpStatus &os ){
    DnaAssemblyToRefTaskSettings settings;

    QString outDir = GUrlUtils::createDirectory(
        getValue<QString>(OUTPUT_DIR) + QDir::separator() + BASE_Bowtie2_SUBDIR,
        "_", os);
    CHECK_OP(os, settings);
    
    if (!outDir.endsWith(QDir::separator())){
        outDir  = outDir + QDir::separator();
    }

    QString outFileName = getValue<QString>(OUTPUT_NAME);
    if(outFileName.isEmpty()){
        outFileName = BASE_Bowtie2_OUTFILE;
    }
    settings.resultFileName = outDir + outFileName;
    
    QString refGenome = getValue<QString>(REFERENCE_GENOME);
    settings.prebuiltIndex = !refGenome.contains(".fa");
    settings.indexFileName = refGenome;
    settings.refSeqUrl = refGenome;

    settings.algName = Bowtie2Task::taskName;

    QMap<QString, QVariant> customSettings;

    customSettings.insert(Bowtie2Task::OPTION_MODE, getValue<QString>(MODE));
    customSettings.insert(Bowtie2Task::OPTION_MISMATCHES, getValue<QString>(MISMATCHES_NUMBER));
    customSettings.insert(Bowtie2Task::OPTION_SEED_LEN, getValue<int>(SEED_LEN));
    customSettings.insert(Bowtie2Task::OPTION_DPAD, getValue<int>(DPAD));
    customSettings.insert(Bowtie2Task::OPTION_GBAR, getValue<int>(GBAR));
    customSettings.insert(Bowtie2Task::OPTION_SEED, getValue<int>(SEED));
    customSettings.insert(Bowtie2Task::OPTION_THREADS, getValue<int>(THREADS));

    customSettings.insert(Bowtie2Task::OPTION_NOMIXED, getValue<bool>(NOMIXED));
    customSettings.insert(Bowtie2Task::OPTION_NODISCORDANT, getValue<bool>(NODISCORDANT));
    customSettings.insert(Bowtie2Task::OPTION_NOFW, getValue<bool>(NOFW));
    customSettings.insert(Bowtie2Task::OPTION_NORC, getValue<bool>(NORC));
    customSettings.insert(Bowtie2Task::OPTION_NOOVERLAP, getValue<bool>(NOOVERLAP));
    customSettings.insert(Bowtie2Task::OPTION_NOCONTAIN, getValue<bool>(NOCONTAIN));

    settings.setCustomSettings(customSettings);

    return settings;
}

/************************************************************************/
/* Factory */
/************************************************************************/
class Bowtie2InputSlotsValidator : public PortValidator {
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
            problemList.append(Problem(Bowtie2Worker::tr("Bowtie2 cannot recognize read pairs from the same file. Please, perform demultiplexing first.")));
            return false;
        }

        return true;
    }
    };

void Bowtie2WorkerFactory::init() {
    QList<PortDescriptor*> portDescs;
    
    //in port
    QMap<Descriptor, DataTypePtr> inTypeMap;
    Descriptor readsDesc(READS_URL_SLOT_ID,
        Bowtie2Worker::tr("URL of a file with reads"),
        Bowtie2Worker::tr("Input reads to be aligned."));
    Descriptor readsPairedDesc(READS_PAIRED_URL_SLOT_ID,
        Bowtie2Worker::tr("URL of a file with mate reads"),
        Bowtie2Worker::tr("Input mate reads to be aligned."));

    inTypeMap[readsDesc] = BaseTypes::STRING_TYPE();
    inTypeMap[readsPairedDesc] = BaseTypes::STRING_TYPE();

    Descriptor inPortDesc(IN_PORT_DESCR,
        Bowtie2Worker::tr("Bowtie2 data"),
        Bowtie2Worker::tr("Input reads to be aligned with Bowtie2."));

    DataTypePtr inTypeSet(new MapDataType(IN_TYPE_ID, inTypeMap));
    portDescs << new PortDescriptor(inPortDesc, inTypeSet, true);

    //out port
    QMap<Descriptor, DataTypePtr> outTypeMap;
    Descriptor assemblyOutDesc(ASSEBLY_OUT_SLOT_ID,
        Bowtie2Worker::tr("Assembly URL"),
        Bowtie2Worker::tr("Output assembly URL."));
    
    Descriptor outPortDesc(OUT_PORT_DESCR,
        Bowtie2Worker::tr("Bowtie2 output data"),
        Bowtie2Worker::tr("Output assembly files."));

    outTypeMap[assemblyOutDesc] = BaseTypes::STRING_TYPE();

    DataTypePtr outTypeSet(new MapDataType(OUT_TYPE_ID, outTypeMap));
    portDescs << new PortDescriptor(outPortDesc, outTypeSet, false, true);
    
 
     QList<Attribute*> attrs;
     {
         Descriptor outDir(OUTPUT_DIR,
             Bowtie2Worker::tr("Output directory"),
             Bowtie2Worker::tr("Directory to save Bowtie2 output files."));

         Descriptor refGenome(REFERENCE_GENOME,
             Bowtie2Worker::tr("Reference genome"),
             Bowtie2Worker::tr("Path to indexed reference genome."));

         Descriptor outName(OUTPUT_NAME,
             Bowtie2Worker::tr("Output file name"),
             Bowtie2Worker::tr("Base name of the output file. 'out.sam' by default"));

         Descriptor mode(MODE ,
             Bowtie2Worker::tr("Mode"),
             Bowtie2Worker::tr("When the -n option is specified (which is the default), bowtie determines which alignments \
                              \nare valid according to the following policy, which is similar to Maq's default policy. \
                              \nIn -v mode, alignments may have no more than V mismatches, where V may be a number from 0 \
                              \nthrough 3 set using the -v option. Quality values are ignored. The -v option is mutually exclusive with the -n option."));

        Descriptor mismatchesNumber(MISMATCHES_NUMBER,
            Bowtie2Worker::tr("Number of mismatches"),
            Bowtie2Worker::tr("Sets the number of mismatches to allowed in a seed alignment. Can be set to 0 or 1. \
                              \nSetting this higher makes alignment slower (often much slower) but increases sensitivity."));

        Descriptor seedLen(SEED_LEN,
            Bowtie2Worker::tr("Seed length (--L)"),
            Bowtie2Worker::tr("Sets the length of the seed substrings to align. Smaller values make alignment slower but more senstive."));

        Descriptor dpad(DPAD,
            Bowtie2Worker::tr("Add columns to allow gaps (--dpad)"),
            Bowtie2Worker::tr("\"Pads\" dynamic programming problems by specified number of columns on either side to allow gaps."));

        Descriptor gbar(GBAR,
            Bowtie2Worker::tr("Disallow gaps (--gbar)"),
            Bowtie2Worker::tr("Disallow gaps within specified number of positions of the beginning or end of the read."));

        Descriptor seed(SEED,
            Bowtie2Worker::tr("Seed (--seed)"),
            Bowtie2Worker::tr("Use specified value as the seed for pseudo-random number generator."));

        Descriptor threads(THREADS,
            Bowtie2Worker::tr("Threads"),
            Bowtie2Worker::tr("Launch specified number of parallel search threads. Threads will run on separate processors/cores and synchronize \
                              \nwhen parsing reads and outputting alignments. Searching for alignments is highly parallel, and speedup is close to linear."));

        Descriptor nomixed(NOMIXED,
            Bowtie2Worker::tr("No unpaired alignments (--no-mixed)"),
            Bowtie2Worker::tr("If Bowtie2 cannot find a paired-end alignment for a pair, by default it will go on to look for unpaired alignments \
                              \nfor the constituent mates. This is called \"mixed mode.\" To disable mixed mode, set this option. Bowtie2 runs a little \
                              \nfaster in the mixed mode, but will only consider alignment status of pairs per se, not individual mates."));

        Descriptor nodiscordant(NODISCORDANT,
            Bowtie2Worker::tr("No discordant alignments (--no-discordant)"),
            Bowtie2Worker::tr("By default, Bowtie2 looks for discordant alignments if it cannot find any concordant alignments. A discordant alignment \
                              \nis an alignment where both mates align uniquely, but that does not satisfy the paired-end constraints. This option disables that behavior."));

        Descriptor noForward(NOFW,
             Bowtie2Worker::tr("No forward orientation (--nofw)"),
             Bowtie2Worker::tr("If --nofw is specified, bowtie will not attempt to align against the forward reference strand."));

         Descriptor noReverse(NORC,
             Bowtie2Worker::tr("No reverse-complement orientation (--norc)"),
             Bowtie2Worker::tr("If --norc is specified, bowtie will not attempt to align against the reverse-complement reference strand."));

         Descriptor nooverlap(NOOVERLAP,
             Bowtie2Worker::tr("No overlapping mates (--no-overlap)"),
             Bowtie2Worker::tr("If one mate alignment overlaps the other at all, consider that to be non-concordant. Default: mates can overlap in \
                                \na concordant alignment."));

         Descriptor nocontain(NOCONTAIN,
             Bowtie2Worker::tr("No mates containing one another (--no-contain)"),
             Bowtie2Worker::tr("If one mate alignment contains the other, consider that to be non-concordant. Default: a mate can contain the \
                               \nother in a concordant alignment."));


        attrs << new Attribute(outDir, BaseTypes::STRING_TYPE(), true, QVariant(""));
        attrs << new Attribute(refGenome, BaseTypes::STRING_TYPE(), true, QVariant(""));
        attrs << new Attribute(outName, BaseTypes::STRING_TYPE(), true, QVariant(BASE_Bowtie2_OUTFILE));

        attrs << new Attribute(mode, BaseTypes::STRING_TYPE(), false, QVariant("--end-to-end"));
        attrs << new Attribute(mismatchesNumber, BaseTypes::NUM_TYPE(), false, QVariant(0));
        attrs << new Attribute(seedLen, BaseTypes::NUM_TYPE(), false, QVariant(20));

        attrs << new Attribute(dpad, BaseTypes::NUM_TYPE(), false, QVariant(15));
        attrs << new Attribute(gbar, BaseTypes::NUM_TYPE(), false, QVariant(4));
        attrs << new Attribute(seed, BaseTypes::NUM_TYPE(), false, QVariant(0));
        attrs << new Attribute(threads, BaseTypes::NUM_TYPE(), false, QVariant(1));

        attrs << new Attribute(nomixed, BaseTypes::BOOL_TYPE(), false, QVariant(false));
        attrs << new Attribute(nodiscordant, BaseTypes::BOOL_TYPE(), false, QVariant(false));
        attrs << new Attribute(noForward, BaseTypes::BOOL_TYPE(), false, QVariant(false));
        attrs << new Attribute(noReverse, BaseTypes::BOOL_TYPE(), false, QVariant(false));
        attrs << new Attribute(nooverlap, BaseTypes::BOOL_TYPE(), false, QVariant(false));
        attrs << new Attribute(nocontain, BaseTypes::BOOL_TYPE(), false, QVariant(false));
     }

     QMap<QString, PropertyDelegate*> delegates;
     {
         delegates[OUTPUT_DIR] = new URLDelegate("", "", false, true, false);
         delegates[REFERENCE_GENOME] = new URLDelegate("", "", false, false, false);

         QVariantMap spinMap;
         spinMap["minimum"] = QVariant(0);
         spinMap["maximum"] = QVariant(INT_MAX);
         delegates[DPAD]  = new SpinBoxDelegate(spinMap);
         delegates[GBAR] = new SpinBoxDelegate(spinMap);
         delegates[SEED] = new SpinBoxDelegate(spinMap);

         QVariantMap mismatchesLimitation;
         mismatchesLimitation["minimum"] = QVariant(0);
         mismatchesLimitation["maximum"] = QVariant(1);
         delegates[MISMATCHES_NUMBER] = new SpinBoxDelegate(mismatchesLimitation);

         QVariantMap seedLengthLimitation;
         seedLengthLimitation["minimum"] = QVariant(4);
         seedLengthLimitation["maximum"] = QVariant(31);
         delegates[SEED_LEN] = new SpinBoxDelegate(seedLengthLimitation);

         QVariantMap threadsLimitation;
         threadsLimitation["minimum"] = QVariant(1);
         threadsLimitation["maximum"] = QVariant(99);
         delegates[THREADS] = new SpinBoxDelegate(threadsLimitation);

         QVariantMap modeMap;
         modeMap["--end-to-end"] = "--end-to-end";
         modeMap["--local"] = "--local";
         delegates[MODE] = new ComboBoxDelegate(modeMap);
    }

    Descriptor protoDesc(Bowtie2WorkerFactory::ACTOR_ID,
        Bowtie2Worker::tr("Align Reads with Bowtie2"),
        Bowtie2Worker::tr("Performs alignment of short reads with Bowtie2."));

    ActorPrototype *proto = new IntegralBusActorPrototype(protoDesc, portDescs, attrs);
    proto->setPrompter(new Bowtie2Prompter());
    proto->setEditor(new DelegateEditor(delegates));
    proto->setPortValidator(IN_PORT_DESCR, new Bowtie2InputSlotsValidator());
    proto->addExternalTool(ET_BOWTIE2_ALIGN);
    WorkflowEnv::getProtoRegistry()->registerProto(BaseActorCategories::CATEGORY_NGS_ALIGN_SHORT_READS(), proto);
    WorkflowEnv::getDomainRegistry()->getById(LocalDomainFactory::ID)->registerEntry(new Bowtie2WorkerFactory());
}

Worker *Bowtie2WorkerFactory::createWorker(Actor *a) {
    return new Bowtie2Worker(a);
}

QString Bowtie2Prompter::composeRichDoc() {
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

