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

#include "ConductGOWorker.h"

namespace U2 {
namespace LocalWorkflow {

const QString ConductGOWorkerFactory::ACTOR_ID("conduct-go-id");

static const QString ANNOT_SLOT_ID("in-ann");

static const QString IN_TYPE_ID("conduct-go-data");

static const QString IN_PORT_DESCR("in-data");

static const QString OUTPUT_DIR("output-dir");
static const QString TITLE("title");
static const QString GENE_UNIVERSE("gene-universe");


/************************************************************************/
/* Worker */
/************************************************************************/
ConductGOWorker::ConductGOWorker(Actor *p)
: BaseWorker(p)
, inChannel(NULL)
{

}

void ConductGOWorker::init() {
    inChannel = ports.value(IN_PORT_DESCR);
}

Task *ConductGOWorker::tick() {
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

        ConductGOSettings settings = createConductGOSettings(os);
        if (os.hasError()) {
            return new FailTask(os.getError());
        }

        Task* t = new ConductGOTask(settings, treatData);
        connect(t, SIGNAL(si_stateChanged()), SLOT(sl_taskFinished()));
        return t;
    }else if (inChannel->isEnded()) {
        setDone();
    }
    return NULL;
}

void ConductGOWorker::cleanup() {

}

void ConductGOWorker::sl_taskFinished() {
    ConductGOTask *t = dynamic_cast<ConductGOTask*>(sender());
    if (!t->isFinished() || t->hasError()) {
        return;
    }

    if (inChannel->isEnded() && !inChannel->hasMessage()) {
        setDone();
    }
}

U2::ConductGOSettings ConductGOWorker::createConductGOSettings( U2OpStatus &os ){
    ConductGOSettings settings;

    settings.outDir = actor->getParameter(OUTPUT_DIR)->getAttributeValue<QString>(context);
    settings.title = actor->getParameter(TITLE)->getAttributeValue<QString>(context);
    settings.geneUniverse = actor->getParameter(GENE_UNIVERSE)->getAttributeValue<QString>(context);

    return settings;
}


QStringList ConductGOWorker::getOutputFiles() {
    return QStringList();
}

/************************************************************************/
/* Factory */
/************************************************************************/


void ConductGOWorkerFactory::init() {
    QList<PortDescriptor*> portDescs;
    
    //in port
    QMap<Descriptor, DataTypePtr> inTypeMap;
    Descriptor treatDesc(ANNOT_SLOT_ID,
        ConductGOWorker::tr("Target genes"),
        ConductGOWorker::tr("Gene list to identify over represented GO terms."));
    inTypeMap[treatDesc] = BaseTypes::ANNOTATION_TABLE_LIST_TYPE();

    Descriptor inPortDesc(IN_PORT_DESCR,
        ConductGOWorker::tr("Conduct GO data"),
        ConductGOWorker::tr("Gene list to identify over represented GO terms."));

    DataTypePtr inTypeSet(new MapDataType(IN_TYPE_ID, inTypeMap));
    portDescs << new PortDescriptor(inPortDesc, inTypeSet, true);

    QList<Attribute*> attrs;
    {
         Descriptor outDir(OUTPUT_DIR,
             ConductGOWorker::tr("Output dir"),
             ConductGOWorker::tr("The directory to store Conduct GO results."));
         Descriptor titleDescr(TITLE,
             ConductGOWorker::tr("Title"),
             ConductGOWorker::tr("Title is used to name the output files - so make it meaningful."));
         Descriptor guDescr(GENE_UNIVERSE,
             ConductGOWorker::tr("Gene Universe"),
             ConductGOWorker::tr("Select a gene universe."));


        attrs << new Attribute(outDir, BaseTypes::STRING_TYPE(), true, QVariant(""));
        attrs << new Attribute(titleDescr, BaseTypes::STRING_TYPE(), true, QVariant("Default"));
        attrs << new Attribute(guDescr, BaseTypes::STRING_TYPE(), true, QVariant("hgu133a"));
    }

    QMap<QString, PropertyDelegate*> delegates;
    {
          delegates[OUTPUT_DIR] = new URLDelegate("", "", false, true);
    }

    Descriptor protoDesc(ConductGOWorkerFactory::ACTOR_ID,
    ConductGOWorker::tr("Conduct GO"),
    ConductGOWorker::tr("Given a list of genes, using Bioconductor (GO, GOstats) and  DAVID at NIH."));

    ActorPrototype *proto = new IntegralBusActorPrototype(protoDesc, portDescs, attrs);
    proto->setPrompter(new ConductGOPrompter());
    proto->setEditor(new DelegateEditor(delegates));
    WorkflowEnv::getProtoRegistry()->registerProto(BaseActorCategories::CATEGORY_CHIP_SEQ(), proto);
    WorkflowEnv::getDomainRegistry()->getById(LocalDomainFactory::ID)->registerEntry(new ConductGOWorkerFactory());
}

Worker *ConductGOWorkerFactory::createWorker(Actor *a) {
    return new ConductGOWorker(a);
}

QString ConductGOPrompter::composeRichDoc() {
    QString res = ""; 

    Actor* annProducer = qobject_cast<IntegralBusPort*>(target->getPort(IN_PORT_DESCR))->getProducer(ANNOT_SLOT_ID);

    QString unsetStr = "<font color='red'>"+tr("unset")+"</font>";
    QString annUrl = annProducer ? annProducer->getLabel() : unsetStr;
     
    QString dir = getHyperlink(OUTPUT_DIR, getURL(OUTPUT_DIR));
 
    res.append(tr("Uses annotations from <u>%1</u> as gene list to conduct GO.").arg(annUrl));
   
    res.append(tr(" Outputs all result files in <u>%1</u> directory").arg(dir));
    res.append(".");

    return res;
}

} // LocalWorkflow
} // U2
