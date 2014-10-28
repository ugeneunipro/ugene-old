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

#include <U2Algorithm/GenomeAssemblyMultiTask.h>

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

#include "SpadesSupport.h"
#include "SpadesWorker.h"
#include "SpadesTask.h"

namespace U2 {
namespace LocalWorkflow {

const QString SpadesWorkerFactory::ACTOR_ID("spades-id");

static const QString READS_URL_SLOT_ID("readsurl");
static const QString READS_PAIRED_URL_SLOT_ID("readspairedurl");

static const QString IN_TYPE_ID("spades-data");
static const QString OUT_TYPE_ID("spades-data-out");

static const QString SCAFFOLD_OUT_SLOT_ID("scaffolds-out");

static const QString IN_PORT_DESCR("in-data");
static const QString OUT_PORT_DESCR("out-data");

static const QString OUTPUT_DIR("output-dir");

static const QString BASE_SPADES_SUBDIR("spades");


/************************************************************************/
/* Worker */
/************************************************************************/
SpadesWorker::SpadesWorker(Actor *p)
: BaseWorker(p)
, inChannel(NULL)
, output(NULL)
{

}

void SpadesWorker::init() {
    inChannel = ports.value(IN_PORT_DESCR);
    output = ports.value(OUT_PORT_DESCR);
}

Task *SpadesWorker::tick() {
    if (inChannel->hasMessage()) {
        U2OpStatus2Log os;

        Message m = getMessageAndSetupScriptValues(inChannel);
        QVariantMap data = m.getData().toMap();

        GenomeAssemblyTaskSettings settings = getSettings(os);
        if (os.hasError()) {
            return new FailTask(os.getError());
        }

        QString readsUrl = data[READS_URL_SLOT_ID].toString();
        AssemblyReads read;
        read.left = readsUrl;
        read.libNumber = "1";
        read.orientation = ORIENTATION_FR;
        read.libType = PAIR_TYPE_DEFAULT;


        if(data.contains(READS_PAIRED_URL_SLOT_ID)){
            QString readsPairedUrl = data[READS_PAIRED_URL_SLOT_ID].toString();
            read.libType = PAIR_TYPE_DEFAULT;
            read.libName = LIBRARY_PAIRED;
            read.right = readsPairedUrl;
        }else {
            read.libName = LIBRARY_SINGLE;
        }
        settings.reads = QList<AssemblyReads>()<<read;

        Task* t = new GenomeAssemblyMultiTask(settings);
        connect(t, SIGNAL(si_stateChanged()), SLOT(sl_taskFinished()));
        return t;
    }else if (inChannel->isEnded()) {
        setDone();
        output->setEnded();
    }
    return NULL;
}

void SpadesWorker::cleanup() {

}

void SpadesWorker::sl_taskFinished() {
    GenomeAssemblyMultiTask *t = dynamic_cast<GenomeAssemblyMultiTask*>(sender());
    if (!t->isFinished() || t->hasError() || t->isCanceled() || t->getResultUrl().isEmpty()) {
        return;
    }


     QString url = t->getResultUrl();

     QVariantMap data;
     data[SCAFFOLD_OUT_SLOT_ID] =  qVariantFromValue<QString>(url);
     output->put(Message(output->getBusType(), data));

    context->getMonitor()->addOutputFile(url, getActor()->getId());

    if (inChannel->isEnded() && !inChannel->hasMessage()) {
        setDone();
        output->setEnded();
    }
}

GenomeAssemblyTaskSettings SpadesWorker::getSettings( U2OpStatus &os ){
    GenomeAssemblyTaskSettings settings;

    settings.algName = ET_SPADES;
    settings.openView = false;

    QString outDir = GUrlUtils::createDirectory(
        getValue<QString>(OUTPUT_DIR) + "/" + BASE_SPADES_SUBDIR,
        "_", os);
    CHECK_OP(os, settings);
    
    if (outDir.endsWith("/")){
        outDir.chop(1);
    }

    settings.outDir = outDir;

    QMap<QString, QVariant> customSettings;
    customSettings.insert(SpadesTask::OPTION_THREADS, getValue<int>(SpadesTask::OPTION_THREADS));
    customSettings.insert(SpadesTask::OPTION_MEMLIMIT, getValue<int>(SpadesTask::OPTION_MEMLIMIT));
    customSettings.insert(SpadesTask::OPTION_K_MER, getValue<QString>(SpadesTask::OPTION_K_MER));
    customSettings.insert(SpadesTask::OPTION_DATASET_TYPE, getValue<QString>(SpadesTask::OPTION_DATASET_TYPE));
    customSettings.insert(SpadesTask::OPTION_RUNNING_MODE, getValue<QString>(SpadesTask::OPTION_RUNNING_MODE));

    settings.setCustomSettings(customSettings);

    return settings;
}

/************************************************************************/
/* Factory */
/************************************************************************/
class SpadesInputSlotsValidator : public PortValidator {
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
            problemList.append(Problem(SpadesWorker::tr("Spades cannot recognize read pairs from the same file. Please, perform demultiplexing first.")));
            return false;
        }

        return true;
    }
    };

void SpadesWorkerFactory::init() {
    QList<PortDescriptor*> portDescs;
    
    //in port
    QMap<Descriptor, DataTypePtr> inTypeMap;
    Descriptor readsDesc(READS_URL_SLOT_ID,
        SpadesWorker::tr("URL of a file with reads"),
        SpadesWorker::tr("Input reads to be assembled."));
    Descriptor readsPairedDesc(READS_PAIRED_URL_SLOT_ID,
        SpadesWorker::tr("URL of a file with right pair reads"),
        SpadesWorker::tr("Input right pair reads to be assembled."));

    inTypeMap[readsDesc] = BaseTypes::STRING_TYPE();
    inTypeMap[readsPairedDesc] = BaseTypes::STRING_TYPE();

    Descriptor inPortDesc(IN_PORT_DESCR,
        SpadesWorker::tr("Spades data"),
        SpadesWorker::tr("Input reads to be assembled with Spades."));

    DataTypePtr inTypeSet(new MapDataType(IN_TYPE_ID, inTypeMap));
    portDescs << new PortDescriptor(inPortDesc, inTypeSet, true);

    //out port
    QMap<Descriptor, DataTypePtr> outTypeMap;
    Descriptor scaffoldOutDesc(SCAFFOLD_OUT_SLOT_ID,
        SpadesWorker::tr("Scaffolds URL"),
        SpadesWorker::tr("Output scaffolds URL."));
    
    Descriptor outPortDesc(OUT_PORT_DESCR,
        SpadesWorker::tr("Spades output data"),
        SpadesWorker::tr("Output assembly files."));

    outTypeMap[scaffoldOutDesc] = BaseTypes::STRING_TYPE();

    DataTypePtr outTypeSet(new MapDataType(OUT_TYPE_ID, outTypeMap));
    portDescs << new PortDescriptor(outPortDesc, outTypeSet, false, true);
    
 
     QList<Attribute*> attrs;
     {
         Descriptor outDir(OUTPUT_DIR,
             SpadesWorker::tr("Output directory"),
             SpadesWorker::tr("Directory to save Spades output files."));

         Descriptor threads(SpadesTask::OPTION_THREADS,
             SpadesWorker::tr("Number of threads"),
             SpadesWorker::tr("Number of threads (-t)."));

         Descriptor memLim(SpadesTask::OPTION_MEMLIMIT,
             SpadesWorker::tr("Memory limit (GB)"),
             SpadesWorker::tr("Memory limit (-m)."));

         Descriptor datasetType(SpadesTask::OPTION_DATASET_TYPE,
             SpadesWorker::tr("Dataset type"),
             SpadesWorker::tr("Input dataset type."));

         Descriptor rMode(SpadesTask::OPTION_RUNNING_MODE,
             SpadesWorker::tr("Running mode"),
             SpadesWorker::tr("Running mode."));

         Descriptor kMer(SpadesTask::OPTION_K_MER,
             SpadesWorker::tr("K-mers"),
             SpadesWorker::tr("k-mer sizes (-k)."));


        attrs << new Attribute(outDir, BaseTypes::STRING_TYPE(), true, QVariant(""));

        attrs << new Attribute(datasetType, BaseTypes::STRING_TYPE(), true, QVariant("Multi Cell"));
        attrs << new Attribute(rMode, BaseTypes::STRING_TYPE(), true, QVariant("Error Correction and Assembly"));
        attrs << new Attribute(kMer, BaseTypes::STRING_TYPE(), true, QVariant("auto"));

        attrs << new Attribute(threads, BaseTypes::NUM_TYPE(), false, QVariant(16));
        attrs << new Attribute(memLim, BaseTypes::NUM_TYPE(), false, QVariant(250));

     }

     QMap<QString, PropertyDelegate*> delegates;
     {
         delegates[OUTPUT_DIR] = new URLDelegate("", "", false, true, false);

         QVariantMap spinMap; spinMap["minimum"] = QVariant(1); spinMap["maximum"] = QVariant(INT_MAX);
         delegates[SpadesTask::OPTION_THREADS]  = new SpinBoxDelegate(spinMap);
         delegates[SpadesTask::OPTION_MEMLIMIT]  = new SpinBoxDelegate(spinMap);

         QVariantMap contentMap;
         contentMap["Multi Cell"] = "Multi Cell";
         contentMap["Single Cell"] = "Single Cell";
         delegates[SpadesTask::OPTION_DATASET_TYPE] = new ComboBoxDelegate(contentMap);

         QVariantMap contentMap2;
         contentMap2["Error Correction and Assembly"] = "Error Correction and Assembly";
         contentMap2["Assembly only"] = "Assembly only";
         contentMap2["Error correction only"] = "Error correction only";
         delegates[SpadesTask::OPTION_RUNNING_MODE] = new ComboBoxDelegate(contentMap2);
    }

    Descriptor protoDesc(SpadesWorkerFactory::ACTOR_ID,
        SpadesWorker::tr("Assemble genomes with Spades"),
        SpadesWorker::tr("Performes assembly of input short reads."));

    ActorPrototype *proto = new IntegralBusActorPrototype(protoDesc, portDescs, attrs);
    proto->setPrompter(new SpadesPrompter());
    proto->setEditor(new DelegateEditor(delegates));
    proto->setPortValidator(IN_PORT_DESCR, new SpadesInputSlotsValidator());
    proto->addExternalTool(ET_SPADES);
    WorkflowEnv::getProtoRegistry()->registerProto(BaseActorCategories::CATEGORY_NGS_BASIC(), proto);
    WorkflowEnv::getDomainRegistry()->getById(LocalDomainFactory::ID)->registerEntry(new SpadesWorkerFactory());
}

Worker *SpadesWorkerFactory::createWorker(Actor *a) {
    return new SpadesWorker(a);
}

QString SpadesPrompter::composeRichDoc() {
    QString res = ""; 

    Actor* readsProducer = qobject_cast<IntegralBusPort*>(target->getPort(IN_PORT_DESCR))->getProducer(READS_URL_SLOT_ID);

    QString unsetStr = "<font color='red'>"+tr("unset")+"</font>";
    QString readsUrl = readsProducer ? readsProducer->getLabel() : unsetStr;

    res.append(tr("Assemble reads from <u>%1</u>.").arg(readsUrl));
 
    return res;
}

} // LocalWorkflow
} // U2

