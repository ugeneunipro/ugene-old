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

#include "ConservationPlotWorker.h"

namespace U2 {
namespace LocalWorkflow {

const QString ConservationPlotWorkerFactory::ACTOR_ID("conservation_plot-id");

static const QString ANNOT_SLOT_ID("cp_treat-ann");

static const QString IN_TYPE_ID("conservation_plot-data");

static const QString IN_PORT_DESCR("in-data");

static const QString OUTPUT_DIR("output-dir");
static const QString TITLE("title");
static const QString LABEL("label");
static const QString WINDOW_S("windos_s");
static const QString HEIGHT("height");
static const QString WIDTH("width");


/************************************************************************/
/* Worker */
/************************************************************************/
ConservationPlotWorker::ConservationPlotWorker(Actor *p)
: BaseWorker(p)
, inChannel(NULL)
{

}

void ConservationPlotWorker::init() {
    inChannel = ports.value(IN_PORT_DESCR);
}

Task *ConservationPlotWorker::tick() {
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

        ConservationPlotSettings settings = createConservationPlotSettings(os);
        if (os.hasError()) {
            return new FailTask(os.getError());
        }

        Task* t = new ConservationPlotTask(settings, treatData);
        connect(t, SIGNAL(si_stateChanged()), SLOT(sl_taskFinished()));
        return t;
    }else if (inChannel->isEnded()) {
        setDone();
    }
    return NULL;
}

void ConservationPlotWorker::cleanup() {

}

void ConservationPlotWorker::sl_taskFinished() {
    ConservationPlotTask *t = dynamic_cast<ConservationPlotTask*>(sender());
    if (!t->isFinished() || t->hasError()) {
        return;
    }

    if (inChannel->isEnded() && !inChannel->hasMessage()) {
        setDone();
    }
}

U2::ConservationPlotSettings ConservationPlotWorker::createConservationPlotSettings( U2OpStatus &os ){
    ConservationPlotSettings settings;

    settings.outDir = actor->getParameter(OUTPUT_DIR)->getAttributeValue<QString>(context);
    settings.title = actor->getParameter(TITLE)->getAttributeValue<QString>(context);
    settings.label = actor->getParameter(LABEL)->getAttributeValue<QString>(context);
    settings.windowSize = actor->getParameter(WINDOW_S)->getAttributeValue<int>(context);
    settings.height = actor->getParameter(HEIGHT)->getAttributeValue<int>(context);
    settings.width = actor->getParameter(WIDTH)->getAttributeValue<int>(context);

    return settings;
}


QStringList ConservationPlotWorker::getOutputFiles() {
    return QStringList();
}

/************************************************************************/
/* Factory */
/************************************************************************/


void ConservationPlotWorkerFactory::init() {
    QList<PortDescriptor*> portDescs;
    
    //in port
    QMap<Descriptor, DataTypePtr> inTypeMap;
    Descriptor treatDesc(ANNOT_SLOT_ID,
        ConservationPlotWorker::tr("Input regions"),
        ConservationPlotWorker::tr("Regions (centered at peak summits for better performance)."));
    inTypeMap[treatDesc] = BaseTypes::ANNOTATION_TABLE_LIST_TYPE();

    Descriptor inPortDesc(IN_PORT_DESCR,
        ConservationPlotWorker::tr("conservation_plot data"),
        ConservationPlotWorker::tr("Regions to plot the PhastCons scores profiles."));

    DataTypePtr inTypeSet(new MapDataType(IN_TYPE_ID, inTypeMap));
    portDescs << new PortDescriptor(inPortDesc, inTypeSet, true);

    QList<Attribute*> attrs;
    {
         Descriptor outDir(OUTPUT_DIR,
             ConservationPlotWorker::tr("Output directory"),
             ConservationPlotWorker::tr("The directory to store phastcons results."));
         Descriptor titleDescr(TITLE,
             ConservationPlotWorker::tr("Title"),
             ConservationPlotWorker::tr("Title of the figure (--title)"));
         Descriptor labelDescr(LABEL,
             ConservationPlotWorker::tr("Label"),
             ConservationPlotWorker::tr("Label of data in the figure (--bed-label)"));
         Descriptor windowSizeDescr(WINDOW_S,
             ConservationPlotWorker::tr("Window width"),
             ConservationPlotWorker::tr("Window width centered at middle of regions. (-w)"));
         Descriptor heightDescr(HEIGHT,
             ConservationPlotWorker::tr("Height"),
             ConservationPlotWorker::tr("Height of plot. (--height)"));
         Descriptor widthDescr(WIDTH,
             ConservationPlotWorker::tr("Width"),
             ConservationPlotWorker::tr("Width of plot. (--width)"));


        attrs << new Attribute(outDir, BaseTypes::STRING_TYPE(), true, QVariant(""));
        attrs << new Attribute(titleDescr, BaseTypes::STRING_TYPE(), true, QVariant("Average Phastcons around the Center of Sites"));
        attrs << new Attribute(labelDescr, BaseTypes::STRING_TYPE(), true, QVariant("Conservation at peak summits"));
        attrs << new Attribute(windowSizeDescr, BaseTypes::NUM_TYPE(), false, QVariant(1000));
        attrs << new Attribute(heightDescr, BaseTypes::NUM_TYPE(), false, QVariant(1000));
        attrs << new Attribute(widthDescr, BaseTypes::NUM_TYPE(), false, QVariant(1000));
    }

    QMap<QString, PropertyDelegate*> delegates;
    {
          delegates[OUTPUT_DIR] = new URLDelegate("", "", false, true);
         {
            QVariantMap vm;
            vm["minimum"] = QVariant(0);
            vm["maximum"] = INT_MAX;
            vm["singleStep"] = QVariant(1000);

            delegates[WINDOW_S] = new SpinBoxDelegate(vm);
            delegates[HEIGHT] = new SpinBoxDelegate(vm);
            delegates[WIDTH] = new SpinBoxDelegate(vm);
         }
    }

    Descriptor protoDesc(ConservationPlotWorkerFactory::ACTOR_ID,
    ConservationPlotWorker::tr("Build consevation plot"),
    ConservationPlotWorker::tr("Plots the PhastCons scores profiles."));

    ActorPrototype *proto = new IntegralBusActorPrototype(protoDesc, portDescs, attrs);
    proto->setPrompter(new ConservationPlotPrompter());
    proto->setEditor(new DelegateEditor(delegates));
    WorkflowEnv::getProtoRegistry()->registerProto(BaseActorCategories::CATEGORY_CHIP_SEQ(), proto);
    WorkflowEnv::getDomainRegistry()->getById(LocalDomainFactory::ID)->registerEntry(new ConservationPlotWorkerFactory());
}

Worker *ConservationPlotWorkerFactory::createWorker(Actor *a) {
    return new ConservationPlotWorker(a);
}

QString ConservationPlotPrompter::composeRichDoc() {
    QString res = ""; 

     Actor* annProducer = qobject_cast<IntegralBusPort*>(target->getPort(IN_PORT_DESCR))->getProducer(ANNOT_SLOT_ID);
 
     QString unsetStr = "<font color='red'>"+tr("unset")+"</font>";
     QString annUrl = annProducer ? annProducer->getLabel() : unsetStr;
     
     QString dir = getHyperlink(OUTPUT_DIR, getURL(OUTPUT_DIR));
 
     res.append(tr("Uses annotations from <u>%1</u> as peak regions for conservation plot.").arg(annUrl));
  
     res.append(tr(" Outputs all result files in <u>%1</u> directory").arg(dir));
     res.append(".");

    return res;
}

} // LocalWorkflow
} // U2
