/**
 * UGENE - Integrated Bioinformatics Tools.
 * Copyright (C) 2008-2012 UniPro <ugene@unipro.ru>
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
#include <U2Formats/GenbankLocationParser.h>

#include <U2Designer/DelegateEditors.h>

#include <U2Gui/DialogUtils.h>

#include <U2Lang/ActorPrototypeRegistry.h>
#include <U2Lang/BaseActorCategories.h>
#include <U2Lang/BaseTypes.h>
#include <U2Lang/WorkflowEnv.h>

#include "SeqPosWorker.h"

namespace U2 {
namespace LocalWorkflow {

const QString SeqPosWorkerFactory::ACTOR_ID("seqpos-id");

static const QString ANNOT_SLOT_ID("cp_treat-ann");

static const QString IN_TYPE_ID("seqpos-data");

static const QString IN_PORT_DESCR("in-data");

static const QString OUTPUT_DIR("output-dir");

static const QString GENOME_ASSEMBLY("assembly");
static const QString FIND_DE_NOVO("de_novo");
static const QString MOTIF_DB("motif_db");
static const QString OUT_NAME("out_name");
static const QString REG_WIDTH("reg_width");
static const QString P_VAL("p_val");

/************************************************************************/
/* Worker */
/************************************************************************/
SeqPosWorker::SeqPosWorker(Actor *p)
: BaseWorker(p)
, inChannel(NULL)
{

}

void SeqPosWorker::init() {
    inChannel = ports.value(IN_PORT_DESCR);
}

Task *SeqPosWorker::tick() {
    if (inChannel->hasMessage()) {
        U2OpStatus2Log os;

        Message m = getMessageAndSetupScriptValues(inChannel);
        QVariantMap data = m.getData().toMap();

        QVariant treatVar;
        if (!data.contains(ANNOT_SLOT_ID)) {
            os.setError("Annotations slot is empty");
            return new FailTask(os.getError());
        }

        treatVar = data[ANNOT_SLOT_ID];

        const QList<SharedAnnotationData>& treatData = QVariantUtils::var2ftl(treatVar.toList());

        SeqPosSettings settings = createSeqPosSettings(os);
        if (os.hasError()) {
            return new FailTask(os.getError());
        }

        Task* t = new SeqPosTask(settings, treatData);
        connect(t, SIGNAL(si_stateChanged()), SLOT(sl_taskFinished()));
        return t;
    }else if (inChannel->isEnded()) {
        setDone();
    }
    return NULL;
}

void SeqPosWorker::cleanup() {

}

void SeqPosWorker::sl_taskFinished() {
    SeqPosTask *t = dynamic_cast<SeqPosTask*>(sender());
    if (!t->isFinished() || t->hasError()) {
        return;
    }

    if (inChannel->isEnded() && !inChannel->hasMessage()) {
        setDone();
    }
}

U2::SeqPosSettings SeqPosWorker::createSeqPosSettings( U2OpStatus &os ){
    SeqPosSettings settings;

    settings.outDir = actor->getParameter(OUTPUT_DIR)->getAttributeValue<QString>(context);
    settings.genomeAssembly = actor->getParameter(GENOME_ASSEMBLY)->getAttributeValue<QString>(context);
    settings.findDeNovo = actor->getParameter(FIND_DE_NOVO)->getAttributeValue<bool>(context);
    settings.motifDB = actor->getParameter(MOTIF_DB)->getAttributeValue<QString>(context);
    settings.outName = actor->getParameter(OUT_NAME)->getAttributeValue<QString>(context);
    settings.regWidth = actor->getParameter(REG_WIDTH)->getAttributeValue<int>(context);
    settings.pVal = actor->getParameter(P_VAL)->getAttributeValue<float>(context);

    return settings;
}


QStringList SeqPosWorker::getOutputFiles() {
    return QStringList();
}

/************************************************************************/
/* Factory */
/************************************************************************/


void SeqPosWorkerFactory::init() {
    QList<PortDescriptor*> portDescs;
    
    //in port
    QMap<Descriptor, DataTypePtr> inTypeMap;
    Descriptor treatDesc(ANNOT_SLOT_ID,
        SeqPosWorker::tr("Input regions"),
        SeqPosWorker::tr("Peak summits."));
    inTypeMap[treatDesc] = BaseTypes::ANNOTATION_TABLE_LIST_TYPE();

    Descriptor inPortDesc(IN_PORT_DESCR,
        SeqPosWorker::tr("SeqPos data"),
        SeqPosWorker::tr("Finds motifs enriched in a set of regions."));

    DataTypePtr inTypeSet(new MapDataType(IN_TYPE_ID, inTypeMap));
    portDescs << new PortDescriptor(inPortDesc, inTypeSet, true);

    QList<Attribute*> attrs;
    {
         Descriptor outDir(OUTPUT_DIR,
             SeqPosWorker::tr("Output directory"),
             SeqPosWorker::tr("The directory to store seqpos results."));
         Descriptor genAssembly(GENOME_ASSEMBLY,
             SeqPosWorker::tr("Genome assembly version"),
             SeqPosWorker::tr("UCSC database version (GENOME)"));
         Descriptor findDeNovo(FIND_DE_NOVO,
             SeqPosWorker::tr("De novo motifs"),
             SeqPosWorker::tr("Run de novo motif search (-d)"));
         Descriptor motifDB(MOTIF_DB,
             SeqPosWorker::tr("Motif database"),
             SeqPosWorker::tr("Known motif collections. (-m)"));
         Descriptor outName(OUT_NAME,
             SeqPosWorker::tr("Output file name"),
             SeqPosWorker::tr("Name of the output file which stores new motifs found during a de novo search. (-n)"));
         Descriptor regW(REG_WIDTH,
             SeqPosWorker::tr("Region width"),
             SeqPosWorker::tr("Width of the region to be scanned for motifs; depends on a resolution of assay. (-w)"));
         Descriptor pVal(P_VAL,
             SeqPosWorker::tr("Pvalue cutoff"),
             SeqPosWorker::tr("Pvalue cutoff for the motif significance. (-p)"));


        attrs << new Attribute(outDir, BaseTypes::STRING_TYPE(), true, QVariant(""));
        attrs << new Attribute(genAssembly, BaseTypes::STRING_TYPE(), true, QVariant(""));
        attrs << new Attribute(findDeNovo, BaseTypes::BOOL_TYPE(), true, QVariant(false));
        attrs << new Attribute(motifDB, BaseTypes::STRING_TYPE(), true, QVariant("Cistrome curated motif database"));
        attrs << new Attribute(outName, BaseTypes::STRING_TYPE(), true, QVariant("Default"));
        attrs << new Attribute(regW, BaseTypes::NUM_TYPE(), false, QVariant(600));
        attrs << new Attribute(pVal, BaseTypes::NUM_TYPE(), false, QVariant(0.001));
    }

    QMap<QString, PropertyDelegate*> delegates;
    {
          delegates[OUTPUT_DIR] = new URLDelegate("", "", false, true);
         {
            QVariantMap vm;
            vm["minimum"] = QVariant(0);
            vm["maximum"] = INT_MAX;
            vm["singleStep"] = QVariant(100);

            delegates[REG_WIDTH] = new SpinBoxDelegate(vm);
         }
         {
             QVariantMap contentMap;
             contentMap[SeqPosSettings::MOTIF_DB_CISTROME] = SeqPosSettings::MOTIF_DB_CISTROME;
             contentMap[SeqPosSettings::MOTIF_DB_PDM] = SeqPosSettings::MOTIF_DB_PDM;
             contentMap[SeqPosSettings::MOTIF_DB_Y1H] = SeqPosSettings::MOTIF_DB_Y1H;
             contentMap[SeqPosSettings::MOTIF_DB_TRANSFAC] = SeqPosSettings::MOTIF_DB_TRANSFAC;
             contentMap[SeqPosSettings::MOTIF_DB_HDPI] = SeqPosSettings::MOTIF_DB_HDPI;
             contentMap[SeqPosSettings::MOTIF_DB_JASPAR] = SeqPosSettings::MOTIF_DB_JASPAR;
             delegates[MOTIF_DB] = new ComboBoxDelegate(contentMap);
         }
         {
             QVariantMap vm;
             vm["minimum"] = QVariant(0);
             vm["maximum"] = QVariant(1);
             vm["singleStep"] = QVariant(0.001);

             delegates[P_VAL] = new DoubleSpinBoxDelegate(vm);
         }
    }

    Descriptor protoDesc(SeqPosWorkerFactory::ACTOR_ID,
    SeqPosWorker::tr("Collect Motifs with SeqPos"),
    SeqPosWorker::tr("Finds motifs enriched in a set of regions."));

    ActorPrototype *proto = new IntegralBusActorPrototype(protoDesc, portDescs, attrs);
    proto->setPrompter(new SeqPosPrompter());
    proto->setEditor(new DelegateEditor(delegates));
    WorkflowEnv::getProtoRegistry()->registerProto(BaseActorCategories::CATEGORY_CHIP_SEQ(), proto);
    WorkflowEnv::getDomainRegistry()->getById(LocalDomainFactory::ID)->registerEntry(new SeqPosWorkerFactory());
}

Worker *SeqPosWorkerFactory::createWorker(Actor *a) {
    return new SeqPosWorker(a);
}

QString SeqPosPrompter::composeRichDoc() {
    QString res = ""; 

     Actor* annProducer = qobject_cast<IntegralBusPort*>(target->getPort(IN_PORT_DESCR))->getProducer(ANNOT_SLOT_ID);
 
     QString unsetStr = "<font color='red'>"+tr("unset")+"</font>";
     QString annUrl = annProducer ? annProducer->getLabel() : unsetStr;
     
     QString dir = getHyperlink(OUTPUT_DIR, getURL(OUTPUT_DIR));
     bool deNovo = getParameter(FIND_DE_NOVO).toBool();
     QString motifDb = getParameter(MOTIF_DB).toString();
     QString genome = getParameter(GENOME_ASSEMBLY).toString();
 
     res.append(tr("Uses regions from <u>%1</u> to find motifs enriched in them.").arg(annUrl));
     res.append(tr(" Genome assembly: <u>%1</u>.").arg(genome.isEmpty()? unsetStr : genome));
     if (deNovo){
         res.append(tr(" Finds de novo motifs."));
     }else{
         res.append(tr(" Uses <u>%1</u> known motifs.").arg(motifDb.isEmpty()? unsetStr : motifDb));
     }
    
     res.append(tr(" Outputs all result files in <u>%1</u> directory").arg(dir.isEmpty() ? unsetStr : dir));
     res.append(".");

    return res;
}

} // LocalWorkflow
} // U2
/**
 * UGENE - Integrated Bioinformatics Tools.
 * Copyright (C) 2008-2012 UniPro <ugene@unipro.ru>
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
#include <U2Formats/GenbankLocationParser.h>

#include <U2Designer/DelegateEditors.h>

#include <U2Gui/DialogUtils.h>

#include <U2Lang/ActorPrototypeRegistry.h>
#include <U2Lang/BaseActorCategories.h>
#include <U2Lang/BaseTypes.h>
#include <U2Lang/WorkflowEnv.h>

#include "SeqPosWorker.h"

namespace U2 {
namespace LocalWorkflow {

const QString SeqPosWorkerFactory::ACTOR_ID("seqpos-id");

static const QString ANNOT_SLOT_ID("cp_treat-ann");

static const QString IN_TYPE_ID("seqpos-data");

static const QString IN_PORT_DESCR("in-data");

static const QString OUTPUT_DIR("output-dir");

static const QString GENOME_ASSEMBLY("assembly");
static const QString FIND_DE_NOVO("de_novo");
static const QString MOTIF_DB("motif_db");
static const QString OUT_NAME("out_name");
static const QString REG_WIDTH("reg_width");
static const QString P_VAL("p_val");

/************************************************************************/
/* Worker */
/************************************************************************/
SeqPosWorker::SeqPosWorker(Actor *p)
: BaseWorker(p)
, inChannel(NULL)
{

}

void SeqPosWorker::init() {
    inChannel = ports.value(IN_PORT_DESCR);
}

Task *SeqPosWorker::tick() {
    if (inChannel->hasMessage()) {
        U2OpStatus2Log os;

        Message m = getMessageAndSetupScriptValues(inChannel);
        QVariantMap data = m.getData().toMap();

        QVariant treatVar;
        if (!data.contains(ANNOT_SLOT_ID)) {
            os.setError("Annotations slot is empty");
            return new FailTask(os.getError());
        }

        treatVar = data[ANNOT_SLOT_ID];

        const QList<SharedAnnotationData>& treatData = QVariantUtils::var2ftl(treatVar.toList());

        SeqPosSettings settings = createSeqPosSettings(os);
        if (os.hasError()) {
            return new FailTask(os.getError());
        }

        Task* t = new SeqPosTask(settings, treatData);
        connect(t, SIGNAL(si_stateChanged()), SLOT(sl_taskFinished()));
        return t;
    }else if (inChannel->isEnded()) {
        setDone();
    }
    return NULL;
}

void SeqPosWorker::cleanup() {

}

void SeqPosWorker::sl_taskFinished() {
    SeqPosTask *t = dynamic_cast<SeqPosTask*>(sender());
    if (!t->isFinished() || t->hasError()) {
        return;
    }

    if (inChannel->isEnded() && !inChannel->hasMessage()) {
        setDone();
    }
}

U2::SeqPosSettings SeqPosWorker::createSeqPosSettings( U2OpStatus &os ){
    SeqPosSettings settings;

    settings.outDir = actor->getParameter(OUTPUT_DIR)->getAttributeValue<QString>(context);
    settings.genomeAssembly = actor->getParameter(GENOME_ASSEMBLY)->getAttributeValue<QString>(context);
    settings.findDeNovo = actor->getParameter(FIND_DE_NOVO)->getAttributeValue<bool>(context);
    settings.motifDB = actor->getParameter(MOTIF_DB)->getAttributeValue<QString>(context);
    settings.outName = actor->getParameter(OUT_NAME)->getAttributeValue<QString>(context);
    settings.regWidth = actor->getParameter(REG_WIDTH)->getAttributeValue<int>(context);
    settings.pVal = actor->getParameter(P_VAL)->getAttributeValue<float>(context);

    return settings;
}


QStringList SeqPosWorker::getOutputFiles() {
    return QStringList();
}

/************************************************************************/
/* Factory */
/************************************************************************/


void SeqPosWorkerFactory::init() {
    QList<PortDescriptor*> portDescs;
    
    //in port
    QMap<Descriptor, DataTypePtr> inTypeMap;
    Descriptor treatDesc(ANNOT_SLOT_ID,
        SeqPosWorker::tr("Input regions"),
        SeqPosWorker::tr("Peak summits."));
    inTypeMap[treatDesc] = BaseTypes::ANNOTATION_TABLE_LIST_TYPE();

    Descriptor inPortDesc(IN_PORT_DESCR,
        SeqPosWorker::tr("SeqPos data"),
        SeqPosWorker::tr("Finds motifs enriched in a set of regions."));

    DataTypePtr inTypeSet(new MapDataType(IN_TYPE_ID, inTypeMap));
    portDescs << new PortDescriptor(inPortDesc, inTypeSet, true);

    QList<Attribute*> attrs;
    {
         Descriptor outDir(OUTPUT_DIR,
             SeqPosWorker::tr("Output directory"),
             SeqPosWorker::tr("The directory to store seqpos results."));
         Descriptor genAssembly(GENOME_ASSEMBLY,
             SeqPosWorker::tr("Genome assembly version"),
             SeqPosWorker::tr("UCSC database version (GENOME)"));
         Descriptor findDeNovo(FIND_DE_NOVO,
             SeqPosWorker::tr("De novo motifs"),
             SeqPosWorker::tr("Run de novo motif search (-d)"));
         Descriptor motifDB(MOTIF_DB,
             SeqPosWorker::tr("Motif database"),
             SeqPosWorker::tr("Known motif collections. (-m)"));
         Descriptor outName(OUT_NAME,
             SeqPosWorker::tr("Output file name"),
             SeqPosWorker::tr("Name of the output file which stores new motifs found during a de novo search. (-n)"));
         Descriptor regW(REG_WIDTH,
             SeqPosWorker::tr("Region width"),
             SeqPosWorker::tr("Width of the region to be scanned for motifs; depends on a resolution of assay. (-w)"));
         Descriptor pVal(P_VAL,
             SeqPosWorker::tr("Pvalue cutoff"),
             SeqPosWorker::tr("Pvalue cutoff for the motif significance. (-p)"));


        attrs << new Attribute(outDir, BaseTypes::STRING_TYPE(), true, QVariant(""));
        attrs << new Attribute(genAssembly, BaseTypes::STRING_TYPE(), true, QVariant(""));
        attrs << new Attribute(findDeNovo, BaseTypes::BOOL_TYPE(), true, QVariant(false));
        attrs << new Attribute(motifDB, BaseTypes::STRING_TYPE(), true, QVariant("Cistrome curated motif database"));
        attrs << new Attribute(outName, BaseTypes::STRING_TYPE(), true, QVariant("Default"));
        attrs << new Attribute(regW, BaseTypes::NUM_TYPE(), false, QVariant(600));
        attrs << new Attribute(pVal, BaseTypes::NUM_TYPE(), false, QVariant(0.001));
    }

    QMap<QString, PropertyDelegate*> delegates;
    {
          delegates[OUTPUT_DIR] = new URLDelegate("", "", false, true);
         {
            QVariantMap vm;
            vm["minimum"] = QVariant(0);
            vm["maximum"] = INT_MAX;
            vm["singleStep"] = QVariant(100);

            delegates[REG_WIDTH] = new SpinBoxDelegate(vm);
         }
         {
             QVariantMap contentMap;
             contentMap[SeqPosSettings::MOTIF_DB_CISTROME] = SeqPosSettings::MOTIF_DB_CISTROME;
             contentMap[SeqPosSettings::MOTIF_DB_PDM] = SeqPosSettings::MOTIF_DB_PDM;
             contentMap[SeqPosSettings::MOTIF_DB_Y1H] = SeqPosSettings::MOTIF_DB_Y1H;
             contentMap[SeqPosSettings::MOTIF_DB_TRANSFAC] = SeqPosSettings::MOTIF_DB_TRANSFAC;
             contentMap[SeqPosSettings::MOTIF_DB_HDPI] = SeqPosSettings::MOTIF_DB_HDPI;
             contentMap[SeqPosSettings::MOTIF_DB_JASPAR] = SeqPosSettings::MOTIF_DB_JASPAR;
             delegates[MOTIF_DB] = new ComboBoxDelegate(contentMap);
         }
         {
             QVariantMap vm;
             vm["minimum"] = QVariant(0);
             vm["maximum"] = QVariant(1);
             vm["singleStep"] = QVariant(0.001);

             delegates[P_VAL] = new DoubleSpinBoxDelegate(vm);
         }
    }

    Descriptor protoDesc(SeqPosWorkerFactory::ACTOR_ID,
    SeqPosWorker::tr("Collect Motifs with SeqPos"),
    SeqPosWorker::tr("Finds motifs enriched in a set of regions."));

    ActorPrototype *proto = new IntegralBusActorPrototype(protoDesc, portDescs, attrs);
    proto->setPrompter(new SeqPosPrompter());
    proto->setEditor(new DelegateEditor(delegates));
    WorkflowEnv::getProtoRegistry()->registerProto(BaseActorCategories::CATEGORY_CHIP_SEQ(), proto);
    WorkflowEnv::getDomainRegistry()->getById(LocalDomainFactory::ID)->registerEntry(new SeqPosWorkerFactory());
}

Worker *SeqPosWorkerFactory::createWorker(Actor *a) {
    return new SeqPosWorker(a);
}

QString SeqPosPrompter::composeRichDoc() {
    QString res = ""; 

     Actor* annProducer = qobject_cast<IntegralBusPort*>(target->getPort(IN_PORT_DESCR))->getProducer(ANNOT_SLOT_ID);
 
     QString unsetStr = "<font color='red'>"+tr("unset")+"</font>";
     QString annUrl = annProducer ? annProducer->getLabel() : unsetStr;
     
     QString dir = getHyperlink(OUTPUT_DIR, getURL(OUTPUT_DIR));
     bool deNovo = getParameter(FIND_DE_NOVO).toBool();
     QString motifDb = getParameter(MOTIF_DB).toString();
     QString genome = getParameter(GENOME_ASSEMBLY).toString();
 
     res.append(tr("Uses regions from <u>%1</u> to find motifs enriched in them.").arg(annUrl));
     res.append(tr(" Genome assembly: <u>%1</u>.").arg(genome.isEmpty()? unsetStr : genome));
     if (deNovo){
         res.append(tr(" Finds de novo motifs."));
     }else{
         res.append(tr(" Uses <u>%1</u> known motifs.").arg(motifDb.isEmpty()? unsetStr : motifDb));
     }
    
     res.append(tr(" Outputs all result files in <u>%1</u> directory").arg(dir.isEmpty() ? unsetStr : dir));
     res.append(".");

    return res;
}

} // LocalWorkflow
} // U2
/**
 * UGENE - Integrated Bioinformatics Tools.
 * Copyright (C) 2008-2012 UniPro <ugene@unipro.ru>
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
#include <U2Formats/GenbankLocationParser.h>

#include <U2Designer/DelegateEditors.h>

#include <U2Gui/DialogUtils.h>

#include <U2Lang/ActorPrototypeRegistry.h>
#include <U2Lang/BaseActorCategories.h>
#include <U2Lang/BaseTypes.h>
#include <U2Lang/WorkflowEnv.h>

#include "SeqPosWorker.h"

namespace U2 {
namespace LocalWorkflow {

const QString SeqPosWorkerFactory::ACTOR_ID("seqpos-id");

static const QString ANNOT_SLOT_ID("cp_treat-ann");

static const QString IN_TYPE_ID("seqpos-data");

static const QString IN_PORT_DESCR("in-data");

static const QString OUTPUT_DIR("output-dir");

static const QString GENOME_ASSEMBLY("assembly");
static const QString FIND_DE_NOVO("de_novo");
static const QString MOTIF_DB("motif_db");
static const QString OUT_NAME("out_name");
static const QString REG_WIDTH("reg_width");
static const QString P_VAL("p_val");

/************************************************************************/
/* Worker */
/************************************************************************/
SeqPosWorker::SeqPosWorker(Actor *p)
: BaseWorker(p)
, inChannel(NULL)
{

}

void SeqPosWorker::init() {
    inChannel = ports.value(IN_PORT_DESCR);
}

Task *SeqPosWorker::tick() {
    if (inChannel->hasMessage()) {
        U2OpStatus2Log os;

        Message m = getMessageAndSetupScriptValues(inChannel);
        QVariantMap data = m.getData().toMap();

        QVariant treatVar;
        if (!data.contains(ANNOT_SLOT_ID)) {
            os.setError("Annotations slot is empty");
            return new FailTask(os.getError());
        }

        treatVar = data[ANNOT_SLOT_ID];

        const QList<SharedAnnotationData>& treatData = QVariantUtils::var2ftl(treatVar.toList());

        SeqPosSettings settings = createSeqPosSettings(os);
        if (os.hasError()) {
            return new FailTask(os.getError());
        }

        Task* t = new SeqPosTask(settings, treatData);
        connect(t, SIGNAL(si_stateChanged()), SLOT(sl_taskFinished()));
        return t;
    }else if (inChannel->isEnded()) {
        setDone();
    }
    return NULL;
}

void SeqPosWorker::cleanup() {

}

void SeqPosWorker::sl_taskFinished() {
    SeqPosTask *t = dynamic_cast<SeqPosTask*>(sender());
    if (!t->isFinished() || t->hasError()) {
        return;
    }

    if (inChannel->isEnded() && !inChannel->hasMessage()) {
        setDone();
    }
}

U2::SeqPosSettings SeqPosWorker::createSeqPosSettings( U2OpStatus &os ){
    SeqPosSettings settings;

    settings.outDir = actor->getParameter(OUTPUT_DIR)->getAttributeValue<QString>(context);
    settings.genomeAssembly = actor->getParameter(GENOME_ASSEMBLY)->getAttributeValue<QString>(context);
    settings.findDeNovo = actor->getParameter(FIND_DE_NOVO)->getAttributeValue<bool>(context);
    settings.motifDB = actor->getParameter(MOTIF_DB)->getAttributeValue<QString>(context);
    settings.outName = actor->getParameter(OUT_NAME)->getAttributeValue<QString>(context);
    settings.regWidth = actor->getParameter(REG_WIDTH)->getAttributeValue<int>(context);
    settings.pVal = actor->getParameter(P_VAL)->getAttributeValue<float>(context);

    return settings;
}


QStringList SeqPosWorker::getOutputFiles() {
    return QStringList();
}

/************************************************************************/
/* Factory */
/************************************************************************/


void SeqPosWorkerFactory::init() {
    QList<PortDescriptor*> portDescs;
    
    //in port
    QMap<Descriptor, DataTypePtr> inTypeMap;
    Descriptor treatDesc(ANNOT_SLOT_ID,
        SeqPosWorker::tr("Input regions"),
        SeqPosWorker::tr("Peak summits."));
    inTypeMap[treatDesc] = BaseTypes::ANNOTATION_TABLE_LIST_TYPE();

    Descriptor inPortDesc(IN_PORT_DESCR,
        SeqPosWorker::tr("SeqPos data"),
        SeqPosWorker::tr("Finds motifs enriched in a set of regions."));

    DataTypePtr inTypeSet(new MapDataType(IN_TYPE_ID, inTypeMap));
    portDescs << new PortDescriptor(inPortDesc, inTypeSet, true);

    QList<Attribute*> attrs;
    {
         Descriptor outDir(OUTPUT_DIR,
             SeqPosWorker::tr("Output directory"),
             SeqPosWorker::tr("The directory to store seqpos results."));
         Descriptor genAssembly(GENOME_ASSEMBLY,
             SeqPosWorker::tr("Genome assembly version"),
             SeqPosWorker::tr("UCSC database version (GENOME)"));
         Descriptor findDeNovo(FIND_DE_NOVO,
             SeqPosWorker::tr("De novo motifs"),
             SeqPosWorker::tr("Run de novo motif search (-d)"));
         Descriptor motifDB(MOTIF_DB,
             SeqPosWorker::tr("Motif database"),
             SeqPosWorker::tr("Known motif collections. (-m)"));
         Descriptor outName(OUT_NAME,
             SeqPosWorker::tr("Output file name"),
             SeqPosWorker::tr("Name of the output file which stores new motifs found during a de novo search. (-n)"));
         Descriptor regW(REG_WIDTH,
             SeqPosWorker::tr("Region width"),
             SeqPosWorker::tr("Width of the region to be scanned for motifs; depends on a resolution of assay. (-w)"));
         Descriptor pVal(P_VAL,
             SeqPosWorker::tr("Pvalue cutoff"),
             SeqPosWorker::tr("Pvalue cutoff for the motif significance. (-p)"));


        attrs << new Attribute(outDir, BaseTypes::STRING_TYPE(), true, QVariant(""));
        attrs << new Attribute(genAssembly, BaseTypes::STRING_TYPE(), true, QVariant(""));
        attrs << new Attribute(findDeNovo, BaseTypes::BOOL_TYPE(), true, QVariant(false));
        attrs << new Attribute(motifDB, BaseTypes::STRING_TYPE(), true, QVariant("Cistrome curated motif database"));
        attrs << new Attribute(outName, BaseTypes::STRING_TYPE(), true, QVariant("Default"));
        attrs << new Attribute(regW, BaseTypes::NUM_TYPE(), false, QVariant(600));
        attrs << new Attribute(pVal, BaseTypes::NUM_TYPE(), false, QVariant(0.001));
    }

    QMap<QString, PropertyDelegate*> delegates;
    {
          delegates[OUTPUT_DIR] = new URLDelegate("", "", false, true);
         {
            QVariantMap vm;
            vm["minimum"] = QVariant(0);
            vm["maximum"] = INT_MAX;
            vm["singleStep"] = QVariant(100);

            delegates[REG_WIDTH] = new SpinBoxDelegate(vm);
         }
         {
             QVariantMap contentMap;
             contentMap[SeqPosSettings::MOTIF_DB_CISTROME] = SeqPosSettings::MOTIF_DB_CISTROME;
             contentMap[SeqPosSettings::MOTIF_DB_PDM] = SeqPosSettings::MOTIF_DB_PDM;
             contentMap[SeqPosSettings::MOTIF_DB_Y1H] = SeqPosSettings::MOTIF_DB_Y1H;
             contentMap[SeqPosSettings::MOTIF_DB_TRANSFAC] = SeqPosSettings::MOTIF_DB_TRANSFAC;
             contentMap[SeqPosSettings::MOTIF_DB_HDPI] = SeqPosSettings::MOTIF_DB_HDPI;
             contentMap[SeqPosSettings::MOTIF_DB_JASPAR] = SeqPosSettings::MOTIF_DB_JASPAR;
             delegates[MOTIF_DB] = new ComboBoxDelegate(contentMap);
         }
         {
             QVariantMap vm;
             vm["minimum"] = QVariant(0);
             vm["maximum"] = QVariant(1);
             vm["singleStep"] = QVariant(0.001);

             delegates[P_VAL] = new DoubleSpinBoxDelegate(vm);
         }
    }

    Descriptor protoDesc(SeqPosWorkerFactory::ACTOR_ID,
    SeqPosWorker::tr("Collect Motifs with SeqPos"),
    SeqPosWorker::tr("Finds motifs enriched in a set of regions."));

    ActorPrototype *proto = new IntegralBusActorPrototype(protoDesc, portDescs, attrs);
    proto->setPrompter(new SeqPosPrompter());
    proto->setEditor(new DelegateEditor(delegates));
    WorkflowEnv::getProtoRegistry()->registerProto(BaseActorCategories::CATEGORY_CHIP_SEQ(), proto);
    WorkflowEnv::getDomainRegistry()->getById(LocalDomainFactory::ID)->registerEntry(new SeqPosWorkerFactory());
}

Worker *SeqPosWorkerFactory::createWorker(Actor *a) {
    return new SeqPosWorker(a);
}

QString SeqPosPrompter::composeRichDoc() {
    QString res = ""; 

     Actor* annProducer = qobject_cast<IntegralBusPort*>(target->getPort(IN_PORT_DESCR))->getProducer(ANNOT_SLOT_ID);
 
     QString unsetStr = "<font color='red'>"+tr("unset")+"</font>";
     QString annUrl = annProducer ? annProducer->getLabel() : unsetStr;
     
     QString dir = getHyperlink(OUTPUT_DIR, getURL(OUTPUT_DIR));
     bool deNovo = getParameter(FIND_DE_NOVO).toBool();
     QString motifDb = getParameter(MOTIF_DB).toString();
     QString genome = getParameter(GENOME_ASSEMBLY).toString();
 
     res.append(tr("Uses regions from <u>%1</u> to find motifs enriched in them.").arg(annUrl));
     res.append(tr(" Genome assembly: <u>%1</u>.").arg(genome.isEmpty()? unsetStr : genome));
     if (deNovo){
         res.append(tr(" Finds de novo motifs."));
     }else{
         res.append(tr(" Uses <u>%1</u> known motifs.").arg(motifDb.isEmpty()? unsetStr : motifDb));
     }
    
     res.append(tr(" Outputs all result files in <u>%1</u> directory").arg(dir.isEmpty() ? unsetStr : dir));
     res.append(".");

    return res;
}

} // LocalWorkflow
} // U2
/**
 * UGENE - Integrated Bioinformatics Tools.
 * Copyright (C) 2008-2012 UniPro <ugene@unipro.ru>
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
#include <U2Formats/GenbankLocationParser.h>

#include <U2Designer/DelegateEditors.h>

#include <U2Gui/DialogUtils.h>

#include <U2Lang/ActorPrototypeRegistry.h>
#include <U2Lang/BaseActorCategories.h>
#include <U2Lang/BaseTypes.h>
#include <U2Lang/WorkflowEnv.h>

#include "SeqPosWorker.h"

namespace U2 {
namespace LocalWorkflow {

const QString SeqPosWorkerFactory::ACTOR_ID("seqpos-id");

static const QString ANNOT_SLOT_ID("cp_treat-ann");

static const QString IN_TYPE_ID("seqpos-data");

static const QString IN_PORT_DESCR("in-data");

static const QString OUTPUT_DIR("output-dir");

static const QString GENOME_ASSEMBLY("assembly");
static const QString FIND_DE_NOVO("de_novo");
static const QString MOTIF_DB("motif_db");
static const QString OUT_NAME("out_name");
static const QString REG_WIDTH("reg_width");
static const QString P_VAL("p_val");

/************************************************************************/
/* Worker */
/************************************************************************/
SeqPosWorker::SeqPosWorker(Actor *p)
: BaseWorker(p)
, inChannel(NULL)
{

}

void SeqPosWorker::init() {
    inChannel = ports.value(IN_PORT_DESCR);
}

Task *SeqPosWorker::tick() {
    if (inChannel->hasMessage()) {
        U2OpStatus2Log os;

        Message m = getMessageAndSetupScriptValues(inChannel);
        QVariantMap data = m.getData().toMap();

        QVariant treatVar;
        if (!data.contains(ANNOT_SLOT_ID)) {
            os.setError("Annotations slot is empty");
            return new FailTask(os.getError());
        }

        treatVar = data[ANNOT_SLOT_ID];

        const QList<SharedAnnotationData>& treatData = QVariantUtils::var2ftl(treatVar.toList());

        SeqPosSettings settings = createSeqPosSettings(os);
        if (os.hasError()) {
            return new FailTask(os.getError());
        }

        Task* t = new SeqPosTask(settings, treatData);
        connect(t, SIGNAL(si_stateChanged()), SLOT(sl_taskFinished()));
        return t;
    }else if (inChannel->isEnded()) {
        setDone();
    }
    return NULL;
}

void SeqPosWorker::cleanup() {

}

void SeqPosWorker::sl_taskFinished() {
    SeqPosTask *t = dynamic_cast<SeqPosTask*>(sender());
    if (!t->isFinished() || t->hasError()) {
        return;
    }

    if (inChannel->isEnded() && !inChannel->hasMessage()) {
        setDone();
    }
}

U2::SeqPosSettings SeqPosWorker::createSeqPosSettings( U2OpStatus &os ){
    SeqPosSettings settings;

    settings.outDir = actor->getParameter(OUTPUT_DIR)->getAttributeValue<QString>(context);
    settings.genomeAssembly = actor->getParameter(GENOME_ASSEMBLY)->getAttributeValue<QString>(context);
    settings.findDeNovo = actor->getParameter(FIND_DE_NOVO)->getAttributeValue<bool>(context);
    settings.motifDB = actor->getParameter(MOTIF_DB)->getAttributeValue<QString>(context);
    settings.outName = actor->getParameter(OUT_NAME)->getAttributeValue<QString>(context);
    settings.regWidth = actor->getParameter(REG_WIDTH)->getAttributeValue<int>(context);
    settings.pVal = actor->getParameter(P_VAL)->getAttributeValue<float>(context);

    return settings;
}


QStringList SeqPosWorker::getOutputFiles() {
    return QStringList();
}

/************************************************************************/
/* Factory */
/************************************************************************/


void SeqPosWorkerFactory::init() {
    QList<PortDescriptor*> portDescs;
    
    //in port
    QMap<Descriptor, DataTypePtr> inTypeMap;
    Descriptor treatDesc(ANNOT_SLOT_ID,
        SeqPosWorker::tr("Input regions"),
        SeqPosWorker::tr("Peak summits."));
    inTypeMap[treatDesc] = BaseTypes::ANNOTATION_TABLE_LIST_TYPE();

    Descriptor inPortDesc(IN_PORT_DESCR,
        SeqPosWorker::tr("SeqPos data"),
        SeqPosWorker::tr("Finds motifs enriched in a set of regions."));

    DataTypePtr inTypeSet(new MapDataType(IN_TYPE_ID, inTypeMap));
    portDescs << new PortDescriptor(inPortDesc, inTypeSet, true);

    QList<Attribute*> attrs;
    {
         Descriptor outDir(OUTPUT_DIR,
             SeqPosWorker::tr("Output directory"),
             SeqPosWorker::tr("The directory to store seqpos results."));
         Descriptor genAssembly(GENOME_ASSEMBLY,
             SeqPosWorker::tr("Genome assembly version"),
             SeqPosWorker::tr("UCSC database version (GENOME)"));
         Descriptor findDeNovo(FIND_DE_NOVO,
             SeqPosWorker::tr("De novo motifs"),
             SeqPosWorker::tr("Run de novo motif search (-d)"));
         Descriptor motifDB(MOTIF_DB,
             SeqPosWorker::tr("Motif database"),
             SeqPosWorker::tr("Known motif collections. (-m)"));
         Descriptor outName(OUT_NAME,
             SeqPosWorker::tr("Output file name"),
             SeqPosWorker::tr("Name of the output file which stores new motifs found during a de novo search. (-n)"));
         Descriptor regW(REG_WIDTH,
             SeqPosWorker::tr("Region width"),
             SeqPosWorker::tr("Width of the region to be scanned for motifs; depends on a resolution of assay. (-w)"));
         Descriptor pVal(P_VAL,
             SeqPosWorker::tr("Pvalue cutoff"),
             SeqPosWorker::tr("Pvalue cutoff for the motif significance. (-p)"));


        attrs << new Attribute(outDir, BaseTypes::STRING_TYPE(), true, QVariant(""));
        attrs << new Attribute(genAssembly, BaseTypes::STRING_TYPE(), true, QVariant(""));
        attrs << new Attribute(findDeNovo, BaseTypes::BOOL_TYPE(), true, QVariant(false));
        attrs << new Attribute(motifDB, BaseTypes::STRING_TYPE(), true, QVariant("Cistrome curated motif database"));
        attrs << new Attribute(outName, BaseTypes::STRING_TYPE(), true, QVariant("Default"));
        attrs << new Attribute(regW, BaseTypes::NUM_TYPE(), false, QVariant(600));
        attrs << new Attribute(pVal, BaseTypes::NUM_TYPE(), false, QVariant(0.001));
    }

    QMap<QString, PropertyDelegate*> delegates;
    {
          delegates[OUTPUT_DIR] = new URLDelegate("", "", false, true);
         {
            QVariantMap vm;
            vm["minimum"] = QVariant(0);
            vm["maximum"] = INT_MAX;
            vm["singleStep"] = QVariant(100);

            delegates[REG_WIDTH] = new SpinBoxDelegate(vm);
         }
         {
             QVariantMap contentMap;
             contentMap[SeqPosSettings::MOTIF_DB_CISTROME] = SeqPosSettings::MOTIF_DB_CISTROME;
             contentMap[SeqPosSettings::MOTIF_DB_PDM] = SeqPosSettings::MOTIF_DB_PDM;
             contentMap[SeqPosSettings::MOTIF_DB_Y1H] = SeqPosSettings::MOTIF_DB_Y1H;
             contentMap[SeqPosSettings::MOTIF_DB_TRANSFAC] = SeqPosSettings::MOTIF_DB_TRANSFAC;
             contentMap[SeqPosSettings::MOTIF_DB_HDPI] = SeqPosSettings::MOTIF_DB_HDPI;
             contentMap[SeqPosSettings::MOTIF_DB_JASPAR] = SeqPosSettings::MOTIF_DB_JASPAR;
             delegates[MOTIF_DB] = new ComboBoxDelegate(contentMap);
         }
         {
             QVariantMap vm;
             vm["minimum"] = QVariant(0);
             vm["maximum"] = QVariant(1);
             vm["singleStep"] = QVariant(0.001);

             delegates[P_VAL] = new DoubleSpinBoxDelegate(vm);
         }
    }

    Descriptor protoDesc(SeqPosWorkerFactory::ACTOR_ID,
    SeqPosWorker::tr("Collect Motifs with SeqPos"),
    SeqPosWorker::tr("Finds motifs enriched in a set of regions."));

    ActorPrototype *proto = new IntegralBusActorPrototype(protoDesc, portDescs, attrs);
    proto->setPrompter(new SeqPosPrompter());
    proto->setEditor(new DelegateEditor(delegates));
    WorkflowEnv::getProtoRegistry()->registerProto(BaseActorCategories::CATEGORY_CHIP_SEQ(), proto);
    WorkflowEnv::getDomainRegistry()->getById(LocalDomainFactory::ID)->registerEntry(new SeqPosWorkerFactory());
}

Worker *SeqPosWorkerFactory::createWorker(Actor *a) {
    return new SeqPosWorker(a);
}

QString SeqPosPrompter::composeRichDoc() {
    QString res = ""; 

     Actor* annProducer = qobject_cast<IntegralBusPort*>(target->getPort(IN_PORT_DESCR))->getProducer(ANNOT_SLOT_ID);
 
     QString unsetStr = "<font color='red'>"+tr("unset")+"</font>";
     QString annUrl = annProducer ? annProducer->getLabel() : unsetStr;
     
     QString dir = getHyperlink(OUTPUT_DIR, getURL(OUTPUT_DIR));
     bool deNovo = getParameter(FIND_DE_NOVO).toBool();
     QString motifDb = getParameter(MOTIF_DB).toString();
     QString genome = getParameter(GENOME_ASSEMBLY).toString();
 
     res.append(tr("Uses regions from <u>%1</u> to find motifs enriched in them.").arg(annUrl));
     res.append(tr(" Genome assembly: <u>%1</u>.").arg(genome.isEmpty()? unsetStr : genome));
     if (deNovo){
         res.append(tr(" Finds de novo motifs."));
     }else{
         res.append(tr(" Uses <u>%1</u> known motifs.").arg(motifDb.isEmpty()? unsetStr : motifDb));
     }
    
     res.append(tr(" Outputs all result files in <u>%1</u> directory").arg(dir.isEmpty() ? unsetStr : dir));
     res.append(".");

    return res;
}

} // LocalWorkflow
} // U2
