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

#include "SNPToolboxWorker.h"
#include "BaseRequestForSnpWorker.h"

#include <U2Core/FailTask.h>
#include <U2Core/U2OpStatusUtils.h>
#include <U2Core/U2SafePoints.h>
#include <U2Core/QVariantUtils.h>
#include <U2Core/AppContext.h>
#include <U2Core/DataPathRegistry.h>

#include <U2Designer/DelegateEditors.h>

#include <U2Lang/ActorPrototypeRegistry.h>
#include <U2Lang/BaseActorCategories.h>
#include <U2Lang/BaseTypes.h>
#include <U2Lang/BaseSlots.h>
#include <U2Lang/BasePorts.h>
#include <U2Lang/WorkflowEnv.h>
#include <U2Lang/WorkflowMonitor.h>



namespace U2 {
namespace LocalWorkflow {

const QString SNPToolboxWorkerFactory::ACTOR_ID("SNPToolbox-id");

static const QString DB_PATH("db_path");

/************************************************************************/
/* Worker */
/************************************************************************/
SNPToolboxWorker::SNPToolboxWorker(Actor *p)
: BaseWorker(p)
, inChannel(NULL)
, output(NULL)
{

}

void SNPToolboxWorker::init() {
    inChannel = ports.value(BasePorts::IN_VARIATION_TRACK_PORT_ID());
    output = ports.value(BasePorts::OUT_VARIATION_TRACK_PORT_ID());
}

Task* SNPToolboxWorker::tick() {
    U2OpStatus2Log os;
    if (inChannel->hasMessage()) {
        Message m = getMessageAndSetupScriptValues(inChannel);
        QVariantMap data = m.getData().toMap();

        QVariant inVar;
        if (!data.contains(BaseSlots::VARIATION_TRACK_SLOT().getId())) {
            os.setError("Variations slot is empty");
            return new FailTask(os.getError());
        }

        QScopedPointer<VariantTrackObject> trackObj(NULL);
        {
            SharedDbiDataHandler objId = data.value(BaseSlots::VARIATION_TRACK_SLOT().getId())
                .value<SharedDbiDataHandler>();
            trackObj.reset(StorageUtils::getVariantTrackObject(context->getDataStorage(), objId));
            SAFE_POINT(NULL != trackObj.data(), tr("Can't get track object"), NULL);

        }

        SNPToolboxSettings settings = createSNPToolboxSettings();

        U2DbiRef dbiRef = trackObj->getEntityRef().dbiRef;
        U2VariantTrack track = trackObj->getVariantTrack(os);
        if(os.hasError()){
            return new FailTask(os.getError());
        }

        Task* t = new SNPToolboxTask(settings, track, dbiRef);
        connect(t, SIGNAL(si_stateChanged()), SLOT(sl_taskFinished()));
        return t;
    }


    if (inChannel->isEnded()) {
        setDone();
        output->setEnded();
    }
    return NULL;
}


void SNPToolboxWorker::sl_taskFinished() {
    SNPToolboxTask *t = dynamic_cast<SNPToolboxTask*>(sender());
    SAFE_POINT( NULL != t, "Invalid task is encountered", );
    if ( t->isCanceled( ) ) {
        return;
    }
    if (!t->isFinished() || t->hasError()) {
        return;
    }
    output->put(Message::getEmptyMapMessage());
    if (inChannel->isEnded() && !inChannel->hasMessage()) {
        setDone();
        output->setEnded();
    }
}

void SNPToolboxWorker::cleanup(){
}

SNPToolboxSettings SNPToolboxWorker::createSNPToolboxSettings(){
    SNPToolboxSettings settings;

    settings.dbPath = actor->getParameter(DB_PATH)->getAttributeValue<QString>(context);

    return settings;
}

/************************************************************************/
/* Factory */
/************************************************************************/


void SNPToolboxWorkerFactory::init() {
    //init data path
    U2DataPath* dataPath = NULL;
    U2DataPathRegistry* dpr =  AppContext::getDataPathRegistry();
    if (dpr){
        U2DataPath* dp = dpr->getDataPathByName(BaseRequestForSnpWorker::DB_SEQUENCE_PATH);
        if (dp && dp->isValid()){
            dataPath = dp;
        }
    }
    QList<PortDescriptor*> p; QList<Attribute*> a;
    {
        Descriptor sd(BasePorts::IN_VARIATION_TRACK_PORT_ID(), "Input variations", "Variations for annotations.");
        Descriptor od(BasePorts::OUT_VARIATION_TRACK_PORT_ID(), "Output variations", "Variations with annotations.");

        QMap<Descriptor, DataTypePtr> modelM;
        QMap<Descriptor, DataTypePtr> inM;
        inM[BaseSlots::VARIATION_TRACK_SLOT()] = BaseTypes::VARIATION_TRACK_TYPE();
        p << new PortDescriptor(sd, DataTypePtr(new MapDataType("in.variations", inM)), true /*input*/);
        QMap<Descriptor, DataTypePtr> outM;
        p << new PortDescriptor(od, DataTypePtr(new MapDataType("out.variations", outM)), false /*input*/, true /*multi*/);
    }
   
    QList<Attribute*> attrs;
    {
         Descriptor dbPath(DB_PATH,
             SNPToolboxWorker::tr("Database path"),
             SNPToolboxWorker::tr("Path to SNPToolbox database with sequences, features and damage effect data."));

         attrs << new Attribute(dbPath, BaseTypes::STRING_TYPE(), true, dataPath != NULL ? dataPath->getPath() : "");
    }

    QMap<QString, PropertyDelegate*> delegates;
    {
          delegates[DB_PATH] = new URLDelegate("", "", false, false, false);
    }

    Descriptor protoDesc(SNPToolboxWorkerFactory::ACTOR_ID,
    SNPToolboxWorker::tr("Annotate Variations with SNPToolbox"),
    SNPToolboxWorker::tr("Assess damage effect and find intersected genes with SNPToolbox algorithms."));

    ActorPrototype *proto = new IntegralBusActorPrototype(protoDesc, p, attrs);
    proto->setPrompter(new SNPToolboxPrompter());
    proto->setEditor(new DelegateEditor(delegates));
    WorkflowEnv::getProtoRegistry()->registerProto(BaseActorCategories::CATEGORY_SNP_ANNOTATION(), proto);
    WorkflowEnv::getDomainRegistry()->getById(LocalDomainFactory::ID)->registerEntry(new SNPToolboxWorkerFactory());
}

Worker *SNPToolboxWorkerFactory::createWorker(Actor *a) {
    return new SNPToolboxWorker(a);
}

QString SNPToolboxPrompter::composeRichDoc() {
    QString res = ""; 

    Actor* annProducer = qobject_cast<IntegralBusPort*>(target->getPort(BasePorts::IN_VARIATION_TRACK_PORT_ID()))->getProducer(BaseSlots::VARIATION_TRACK_SLOT().getId());

    QString unsetStr = "<font color='red'>"+tr("unset")+"</font>";
    QString annUrl = annProducer ? annProducer->getLabel() : unsetStr;

    QString path = getHyperlink(DB_PATH, getURL(DB_PATH));

    res.append(tr("Uses variations from <u>%1</u> as input.").arg(annUrl));
    res.append(tr(" Takes annotations from <u>%1</u> database.").arg(path.isEmpty() ? unsetStr : path));

    return res;
}

} // LocalWorkflow
} // U2
