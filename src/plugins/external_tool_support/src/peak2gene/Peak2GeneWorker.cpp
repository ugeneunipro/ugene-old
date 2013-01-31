/**
 * UGENE - Integrated Bioinformatics Tools.
 * Copyright (C) 2008-2013 UniPro <ugene@unipro.ru>
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

#include <U2Core/FailTask.h>
#include <U2Core/U2OpStatusUtils.h>
#include <U2Core/U2SafePoints.h>
#include <U2Core/QVariantUtils.h>
#include <U2Core/AppContext.h>
#include <U2Core/DataPathRegistry.h>
#include <U2Formats/GenbankLocationParser.h>

#include <U2Designer/DelegateEditors.h>

#include <U2Gui/DialogUtils.h>

#include <U2Lang/ActorPrototypeRegistry.h>
#include <U2Lang/BaseActorCategories.h>
#include <U2Lang/BaseTypes.h>
#include <U2Lang/WorkflowEnv.h>

#include "Peak2GeneWorker.h"
#include "Peak2GeneSupport.h"

namespace U2 {
namespace LocalWorkflow {

const QString Peak2GeneWorkerFactory::ACTOR_ID("peak2gene-id");

static const QString TREAT_SLOT_ID("_treat-ann");

static const QString IN_TYPE_ID("peak2gene-data");

static const QString IN_PORT_DESCR("in-data");

static const QString OUTPUT_DIR("output-dir");
static const QString FILE_NAMES("file-names");
static const QString OUTPOS("outpos");
static const QString SYMBOL("symbol");
static const QString DISTANCE("distance");
static const QString GENOME("genome");


/************************************************************************/
/* Worker */
/************************************************************************/
Peak2GeneWorker::Peak2GeneWorker(Actor *p)
: BaseWorker(p)
, inChannel(NULL)
{

}

void Peak2GeneWorker::init() {
    inChannel = ports.value(IN_PORT_DESCR);
}

Task *Peak2GeneWorker::tick() {
    if (inChannel->hasMessage()) {
        U2OpStatus2Log os;

        Message m = getMessageAndSetupScriptValues(inChannel);
        QVariantMap data = m.getData().toMap();

        QVariant treatVar;
        if (!data.contains(TREAT_SLOT_ID)) {
            os.setError("Treatment slot is empty");
            return new FailTask(os.getError());
        }

        treatVar = data[TREAT_SLOT_ID];

        const QList<SharedAnnotationData>& treatData = QVariantUtils::var2ftl(treatVar.toList());

        Peak2GeneSettings settings = createPeak2GeneSettings(os);
        if (os.hasError()) {
            return new FailTask(os.getError());
        }

        Task* t = new Peak2GeneTask(settings, treatData);
        connect(t, SIGNAL(si_stateChanged()), SLOT(sl_taskFinished()));
        return t;
    }else if (inChannel->isEnded()) {
        setDone();
    }
    return NULL;
}

void Peak2GeneWorker::cleanup() {

}

void Peak2GeneWorker::sl_taskFinished() {
    Peak2GeneTask *t = dynamic_cast<Peak2GeneTask*>(sender());
    if (!t->isFinished() || t->hasError()) {
        return;
    }

    if (inChannel->isEnded() && !inChannel->hasMessage()) {
        setDone();
    }
}

U2::Peak2GeneSettings Peak2GeneWorker::createPeak2GeneSettings( U2OpStatus &os ){
    Peak2GeneSettings settings;

    settings.outDir = actor->getParameter(OUTPUT_DIR)->getAttributeValue<QString>(context);
    settings.fileNames = actor->getParameter(FILE_NAMES)->getAttributeValue<QString>(context);
    settings.outpos = actor->getParameter(OUTPOS)->getAttributeValue<QString>(context);
    settings.symbol = actor->getParameter(SYMBOL)->getAttributeValue<bool>(context);
    settings.distance = actor->getParameter(DISTANCE)->getAttributeValue<int>(context);
    settings.genomePath = actor->getParameter(GENOME)->getAttributeValue<QString>(context);

    return settings;
}


QStringList Peak2GeneWorker::getOutputFiles() {
    return QStringList();
}

/************************************************************************/
/* Factory */
/************************************************************************/


void Peak2GeneWorkerFactory::init() {

    //init data path
    U2DataPath* dataPath = NULL;
    U2DataPathRegistry* dpr =  AppContext::getDataPathRegistry();
    if (dpr){
        U2DataPath* dp = dpr->getDataPathByName(Peak2GeneSupport::REF_GENES_DATA_NAME);
        if (dp && dp->isValid()){
            dataPath = dp;
        }
    }

    QList<PortDescriptor*> portDescs;
    
    //in port
    QMap<Descriptor, DataTypePtr> inTypeMap;
    Descriptor treatDesc(TREAT_SLOT_ID,
        Peak2GeneWorker::tr("Treatment features"),
        Peak2GeneWorker::tr("Result peaks of MACS."));
    inTypeMap[treatDesc] = BaseTypes::ANNOTATION_TABLE_LIST_TYPE();

    Descriptor inPortDesc(IN_PORT_DESCR,
        Peak2GeneWorker::tr("Peak2gene data"),
        Peak2GeneWorker::tr("MACS peaks to get the refgenes near the peak summit/center."));

    DataTypePtr inTypeSet(new MapDataType(IN_TYPE_ID, inTypeMap));
    portDescs << new PortDescriptor(inPortDesc, inTypeSet, true);

     QList<Attribute*> attrs;
     {
         Descriptor outDir(OUTPUT_DIR,
             Peak2GeneWorker::tr("Output directory"),
             Peak2GeneWorker::tr("Directory to save peak2gene result output files with: genes containing all the peaks nearby and peaks containing all the genes nearby."));
         Descriptor fileNames(FILE_NAMES,
             Peak2GeneWorker::tr("Name"),
             Peak2GeneWorker::tr("Name of the result file, it will output two file NAME_peaks_annotation.txt,"
             "NAME_gene_annotation.txt. (--name)"));
         Descriptor outpos(OUTPOS,
             Peak2GeneWorker::tr("Output type"),
             Peak2GeneWorker::tr("Select which type of genes need to output. "
             "<b>up</b> for genes upstream to peak summit, "
             "<b>down</b> for genes downstream to peak summit, "
             "<b>all</b> for both <b>up</b> and <b>down</b>. (--op)"));
         Descriptor symbol(SYMBOL,
             Peak2GeneWorker::tr("Gene names type"),
             Peak2GeneWorker::tr("Output gene symbol instead of refseq name. (--symbol)"));
         Descriptor p2g_distance(DISTANCE,
             Peak2GeneWorker::tr("Distance"),
             Peak2GeneWorker::tr("Set a number which unit is base. It will get the refGenes in n bases from peak center. (--distance)"));
         Descriptor p2g_genome(GENOME,
             Peak2GeneWorker::tr("Genome file"),
             Peak2GeneWorker::tr("Select a genome file (sqlite3 file) to search refGenes. (--genome)"));

         Attribute* annGrAttr = NULL;
         if (dataPath){
             const QList<QString>& dataNames = dataPath->getDataNames();
             if (!dataNames.isEmpty()){
                 annGrAttr = new Attribute(p2g_genome, BaseTypes::STRING_TYPE(), true, dataPath->getPathByName(dataNames.first()));
             }else{
                 annGrAttr = new Attribute(p2g_genome, BaseTypes::STRING_TYPE(), true);
             }
         }else{
             annGrAttr = new Attribute(p2g_genome, BaseTypes::STRING_TYPE(), true);
        }

        attrs << new Attribute(outDir, BaseTypes::STRING_TYPE(), true, QVariant(""));
        attrs << new Attribute(fileNames, BaseTypes::STRING_TYPE(), true, QVariant("Default"));
        attrs << annGrAttr;
        attrs << new Attribute(outpos, BaseTypes::STRING_TYPE(), false, QVariant(Peak2GeneSettings::OUT_TYPE_UPSTREAM));
        attrs << new Attribute(symbol, BaseTypes::BOOL_TYPE(), false, QVariant(false));
        attrs << new Attribute(p2g_distance, BaseTypes::NUM_TYPE(), false, QVariant(3000));
     }

     QMap<QString, PropertyDelegate*> delegates;
     {
         delegates[OUTPUT_DIR] = new URLDelegate("", "", false, true);
         {
            QVariantMap vm;
            vm["minimum"] = QVariant(0);
            vm["maximum"] = INT_MAX;
            vm["singleStep"] = QVariant(100);

            delegates[DISTANCE] = new SpinBoxDelegate(vm);
         }

         {
             QVariantMap vm;
             if (dataPath){
                 if (dataPath){
                     vm = dataPath->getDataItemsVariantMap();
                 }
             }
             delegates[GENOME] = new ComboBoxWithUrlsDelegate(vm);
         }


         QVariantMap contentMap;
         contentMap[Peak2GeneSettings::OUT_TYPE_ALL] = Peak2GeneSettings::OUT_TYPE_ALL;
         contentMap[Peak2GeneSettings::OUT_TYPE_DOWNSTREAM] = Peak2GeneSettings::OUT_TYPE_DOWNSTREAM;
         contentMap[Peak2GeneSettings::OUT_TYPE_UPSTREAM] = Peak2GeneSettings::OUT_TYPE_UPSTREAM;
         delegates[OUTPOS] = new ComboBoxDelegate(contentMap);
         
     }

    Descriptor protoDesc(Peak2GeneWorkerFactory::ACTOR_ID,
        Peak2GeneWorker::tr("Annotate peaks with peak2gene"),
        Peak2GeneWorker::tr("Gets refGenes near the ChIP regions identified by a peak-caller."));

    ActorPrototype *proto = new IntegralBusActorPrototype(protoDesc, portDescs, attrs);
    proto->setPrompter(new Peak2GenePrompter());
    proto->setEditor(new DelegateEditor(delegates));
    WorkflowEnv::getProtoRegistry()->registerProto(BaseActorCategories::CATEGORY_CHIP_SEQ(), proto);
    WorkflowEnv::getDomainRegistry()->getById(LocalDomainFactory::ID)->registerEntry(new Peak2GeneWorkerFactory());
}

Worker *Peak2GeneWorkerFactory::createWorker(Actor *a) {
    return new Peak2GeneWorker(a);
}

QString Peak2GenePrompter::composeRichDoc() {
    QString res = ""; 

    Actor* treatProducer = qobject_cast<IntegralBusPort*>(target->getPort(IN_PORT_DESCR))->getProducer(TREAT_SLOT_ID);

    QString unsetStr = "<font color='red'>"+tr("unset")+"</font>";
    QString treatUrl = treatProducer ? treatProducer->getLabel() : unsetStr;
    
    QString dir = getHyperlink(OUTPUT_DIR, getURL(OUTPUT_DIR));
    QString dbUrl = getHyperlink(GENOME, getURL(GENOME));

    res.append(tr("Uses annotations from <u>%1</u> as peak regions ").arg(treatUrl));
    res.append(tr(" to annotate with genes from <u>%1</u>.").arg(dbUrl));
 
    res.append(tr(" Outputs all result files in <u>%1</u> directory").arg(dir));
    res.append(".");

    return res;
}

} // LocalWorkflow
} // U2
