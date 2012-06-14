/**
 * UGENE - Integrated Bioinformatics Tools.
 * Copyright (C) 2008-2011 UniPro <ugene@unipro.ru>
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

#include <U2Core/AppContext.h>
#include <U2Core/AppResources.h>
#include <U2Core/DocumentUtils.h>
#include <U2Core/IOAdapter.h>
#include <U2Core/IOAdapterUtils.h>
#include <U2Core/U2SafePoints.h>
#include <U2Core/VariantTrackObject.h>

#include <U2Designer/DelegateEditors.h>

#include <U2Gui/DialogUtils.h>
#include <U2Gui/GUIUtils.h>

#include <U2Lang/ActorPrototypeRegistry.h>
#include <U2Lang/BaseActorCategories.h>
#include <U2Lang/BaseAttributes.h>
#include <U2Lang/BasePorts.h>
#include <U2Lang/BaseSlots.h>
#include <U2Lang/BaseTypes.h>
#include <U2Lang/WorkflowEnv.h>

#include "DocActors.h"

#include "ReadVariationWorker.h"

namespace U2 {
namespace LocalWorkflow {

const QString ReadVariationWorkerFactory::ACTOR_ID("read-variations");

/************************************************************************/
/* Worker */
/************************************************************************/
ReadVariationWorker::ReadVariationWorker(Actor *p)
: GenericDocReader(p)
{

}

void ReadVariationWorker::init() {
    GenericDocReader::init();
    IntegralBus *outBus = dynamic_cast<IntegralBus*>(ch);
    assert(outBus);
    mtype = outBus->getBusType();
}

Task *ReadVariationWorker::createReadTask(const QString &url) {
    return new ReadVariationTask(url, context->getDataStorage());
}

void ReadVariationWorker::sl_taskFinished() {
    ReadVariationTask *t = qobject_cast<ReadVariationTask*>(sender());
    if (!t->isFinished() || t->hasError()) {
        return;
    }
    foreach(const QVariantMap &m, t->results) {
        cache.append(Message(mtype, m));
    }
    t->results.clear();
}

/************************************************************************/
/* Task */
/************************************************************************/
ReadVariationTask::ReadVariationTask(const QString &url, DbiDataStorage *storage)
: Task(tr("Read variations from %1").arg(url), TaskFlag_None), url(url), storage(storage)
{

}

void ReadVariationTask::prepare() {
    int memUseMB = 0;
    QFileInfo file(url);
    memUseMB = file.size() / (1024*1024);
    IOAdapterFactory *iof = AppContext::getIOAdapterRegistry()->getIOAdapterFactoryById(IOAdapterUtils::url2io(url));
    if (iof->getAdapterId() == BaseIOAdapters::GZIPPED_LOCAL_FILE || iof->getAdapterId() == BaseIOAdapters::GZIPPED_HTTP_FILE) {
        memUseMB *= 2.5; //Need to calculate compress level
    }
    coreLog.trace(QString("load document:Memory resource %1").arg(memUseMB));

    if (memUseMB > 0) {
        addTaskResource(TaskResourceUsage(RESOURCE_MEMORY, memUseMB, false));
    }
}

void ReadVariationTask::run() {
    QFileInfo fi(url);
    if(!fi.exists()){
        stateInfo.setError(tr("File '%1' not exists").arg(url));
        return;
    }
    QList<DocumentFormat*> fs = DocumentUtils::toFormats(DocumentUtils::detectFormat(url));
    DocumentFormat *format = NULL;

    foreach (DocumentFormat *f, fs) {
        const QSet<GObjectType> &types = f->getSupportedObjectTypes();
        if (types.contains(GObjectTypes::VARIANT_TRACK)) {
            format = f;
            break;
        }
    }

    if (format == NULL) {
        stateInfo.setError(tr("Unsupported document format"));
        return;
    }
    ioLog.info(tr("Reading variations from %1 [%2]").arg(url).arg(format->getFormatName()));
    IOAdapterFactory* iof = AppContext::getIOAdapterRegistry()->getIOAdapterFactoryById(IOAdapterUtils::url2io(url));
    QVariantMap hints;
    hints.insert(DocumentFormat::DBI_REF_HINT, qVariantFromValue(storage->getDbiRef()));
    std::auto_ptr<Document> doc(format->loadDocument(iof, url, hints, stateInfo));
    CHECK_OP(stateInfo, );
    doc->setDocumentOwnsDbiResources(false);

    foreach(GObject* go, doc->findGObjectByType(GObjectTypes::VARIANT_TRACK)) {
        VariantTrackObject *trackObj = dynamic_cast<VariantTrackObject*>(go);
        CHECK_EXT(NULL != trackObj, taskLog.error(tr("Incorrect track object in %1").arg(url)), )

        QVariantMap m;
        m.insert(BaseSlots::URL_SLOT().getId(), url);
        SharedDbiDataHandler handler = storage->getDataHandler(trackObj->getEntityRef().entityId);
        m.insert(BaseSlots::VARIATION_TRACK_SLOT().getId(), qVariantFromValue<SharedDbiDataHandler>(handler));
        results.append(m);
    }
}

/************************************************************************/
/* Factory */
/************************************************************************/
void ReadVariationWorkerFactory::init() {
    QList<PortDescriptor*> portDescs;
    {
        QMap<Descriptor, DataTypePtr> outTypeMap;
        outTypeMap[BaseSlots::VARIATION_TRACK_SLOT()] = BaseTypes::VARIATION_TRACK_TYPE();
        outTypeMap[BaseSlots::URL_SLOT()] = BaseTypes::STRING_TYPE();
        DataTypePtr outTypeSet(new MapDataType(BasePorts::OUT_VARIATION_TRACK_PORT_ID(), outTypeMap));

        Descriptor outDesc(BasePorts::OUT_VARIATION_TRACK_PORT_ID(),
            ReadVariationWorker::tr("Variation track"),
            ReadVariationWorker::tr("Variation track"));

        portDescs << new PortDescriptor(outDesc, outTypeSet, false, true);
    }

    QList<Attribute*> attrs;
    {
        attrs << new Attribute(BaseAttributes::URL_IN_ATTRIBUTE(), BaseTypes::STRING_TYPE(), true);
    }

    QMap<QString, PropertyDelegate*> delegates;
    {
        delegates[BaseAttributes::URL_IN_ATTRIBUTE().getId()] = new URLDelegate(DialogUtils::prepareDocumentsFileFilter(true), QString(), true);
    }

    Descriptor protoDesc(ReadVariationWorkerFactory::ACTOR_ID,
        ReadVariationWorker::tr("Read Variations"),
        ReadVariationWorker::tr("Reads variations from files and produces variations tracks"));

    ActorPrototype *proto = new IntegralBusActorPrototype(protoDesc, portDescs, attrs);
    proto->setEditor(new DelegateEditor(delegates));
    proto->setPrompter(new ReadDocPrompter(ReadVariationWorker::tr("Reads variations from <u>%1</u>.")));
    if (AppContext::isGUIMode()) {
        proto->setIcon( GUIUtils::createRoundIcon(QColor(85,85,255), 22));
    }

    WorkflowEnv::getProtoRegistry()->registerProto(BaseActorCategories::CATEGORY_DATASRC(), proto);
    WorkflowEnv::getDomainRegistry()->getById(LocalDomainFactory::ID)->registerEntry(new ReadVariationWorkerFactory());
}

Worker *ReadVariationWorkerFactory::createWorker(Actor *a) {
    return new ReadVariationWorker(a);
}

} // LocalWorkflow
} // U2
