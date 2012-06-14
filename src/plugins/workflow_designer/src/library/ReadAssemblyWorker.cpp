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
#include <U2Core/AssemblyObject.h>
#include <U2Core/DocumentImport.h>
#include <U2Core/DocumentProviderTask.h>
#include <U2Core/DocumentUtils.h>
#include <U2Core/IOAdapter.h>
#include <U2Core/IOAdapterUtils.h>
#include <U2Core/U2SafePoints.h>

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

#include "ReadAssemblyWorker.h"

namespace U2 {
namespace LocalWorkflow {

const QString ReadAssemblyWorkerFactory::ACTOR_ID("read-assembly");

/************************************************************************/
/* Worker */
/************************************************************************/
ReadAssemblyWorker::ReadAssemblyWorker(Actor *p)
: GenericDocReader(p)
{

}

void ReadAssemblyWorker::init() {
    GenericDocReader::init();
    IntegralBus *outBus = dynamic_cast<IntegralBus*>(ch);
    assert(outBus);
    mtype = outBus->getBusType();
}

Task *ReadAssemblyWorker::createReadTask(const QString &url) {
    return new ReadAssemblyTask(url, context->getDataStorage());
}

void ReadAssemblyWorker::sl_taskFinished() {
    ReadAssemblyTask *t = qobject_cast<ReadAssemblyTask*>(sender());
    if (!t->isFinished() || t->hasError()) {
        return;
    }
    foreach(const QVariantMap &m, t->results) {
        cache.append(Message(mtype, m));
        cache.append(Message(mtype, m));
    }
    t->results.clear();
}

/************************************************************************/
/* Task */
/************************************************************************/
ReadAssemblyTask::ReadAssemblyTask(const QString &url, DbiDataStorage *storage)
: Task(tr("Read assembly from %1").arg(url), TaskFlag_None),
url(url), storage(storage), format(NULL), doc(NULL)
{

}

void ReadAssemblyTask::prepare() {
    QFileInfo fi(url);
    if(!fi.exists()){
        stateInfo.setError(tr("File '%1' not exists").arg(url));
        return;
    }

    FormatDetectionConfig conf;
    conf.useImporters = true;
    QList<FormatDetectionResult> fs = DocumentUtils::detectFormat(url, conf);

    foreach (const FormatDetectionResult &f, fs) {
        if (NULL != f.format) {
            const QSet<GObjectType> &types = f.format->getSupportedObjectTypes();
            if (types.contains(GObjectTypes::ASSEMBLY)) {
                format = f.format;
                break;
            }
        } else if (NULL != f.importer) {
            QVariantMap hints;
            hints.insert(DocumentFormat::DBI_REF_HINT, qVariantFromValue(storage->getDbiRef()));
            DocumentProviderTask *t = f.importer->createImportTask(f, false, hints);
            addSubTask(t);
            return;
        }
    }

    if (format == NULL) {
        stateInfo.setError(tr("Unsupported document format"));
        return;
    }
}

QList<Task*> ReadAssemblyTask::onSubTaskFinished(Task *subTask) {
    QList<Task*> result;
    CHECK(!subTask->hasError(), result);

    DocumentProviderTask *t = dynamic_cast<DocumentProviderTask*>(subTask);
    CHECK(NULL != t, result);

    doc = t->takeDocument(false);
    return result;
}

void ReadAssemblyTask::run() {
    if (NULL == format && NULL == doc) {
        return;
    }

    std::auto_ptr<Document> docPtr(NULL);
    if (NULL == doc) {
        ioLog.info(tr("Reading assembly from %1 [%2]").arg(url).arg(format->getFormatName()));
        IOAdapterFactory* iof = AppContext::getIOAdapterRegistry()->getIOAdapterFactoryById(IOAdapterUtils::url2io(url));
        QVariantMap hints;
        hints.insert(DocumentFormat::DBI_REF_HINT, qVariantFromValue(storage->getDbiRef()));
        docPtr.reset(format->loadDocument(iof, url, hints, stateInfo));
        CHECK_OP(stateInfo, );
    } else {
        docPtr.reset(doc);
        doc = NULL;
    }
    CHECK(NULL != docPtr.get(), );
    docPtr->setDocumentOwnsDbiResources(false);

    foreach(GObject* go, docPtr->findGObjectByType(GObjectTypes::ASSEMBLY)) {
        AssemblyObject *assemblyObj = dynamic_cast<AssemblyObject*>(go);
        CHECK_EXT(NULL != assemblyObj, taskLog.error(tr("Incorrect assembly object in %1").arg(url)), );

        QVariantMap m;
        m.insert(BaseSlots::URL_SLOT().getId(), url);
        SharedDbiDataHandler handler = storage->getDataHandler(assemblyObj->getEntityRef().entityId);
        m.insert(BaseSlots::ASSEMBLY_SLOT().getId(), qVariantFromValue<SharedDbiDataHandler>(handler));
        results.append(m);
    }
}

/************************************************************************/
/* Factory */
/************************************************************************/
void ReadAssemblyWorkerFactory::init() {
    QList<PortDescriptor*> portDescs;
    {
        QMap<Descriptor, DataTypePtr> outTypeMap;
        outTypeMap[BaseSlots::ASSEMBLY_SLOT()] = BaseTypes::ASSEMBLY_TYPE();
        outTypeMap[BaseSlots::URL_SLOT()] = BaseTypes::STRING_TYPE();
        DataTypePtr outTypeSet(new MapDataType(BasePorts::OUT_ASSEMBLY_PORT_ID(), outTypeMap));

        Descriptor outDesc(BasePorts::OUT_ASSEMBLY_PORT_ID(),
            ReadAssemblyWorker::tr("Assembly"),
            ReadAssemblyWorker::tr("Assembly"));

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

    Descriptor protoDesc(ReadAssemblyWorkerFactory::ACTOR_ID,
        ReadAssemblyWorker::tr("Read Assembly"),
        ReadAssemblyWorker::tr("Reads assembly from files"));

    ActorPrototype *proto = new IntegralBusActorPrototype(protoDesc, portDescs, attrs);
    proto->setEditor(new DelegateEditor(delegates));
    proto->setPrompter(new ReadDocPrompter(ReadAssemblyWorker::tr("Reads assembly from <u>%1</u>.")));
    if (AppContext::isGUIMode()) {
        proto->setIcon(GUIUtils::createRoundIcon(QColor(85,85,255), 22));
    }

    WorkflowEnv::getProtoRegistry()->registerProto(BaseActorCategories::CATEGORY_DATASRC(), proto);
    WorkflowEnv::getDomainRegistry()->getById(LocalDomainFactory::ID)->registerEntry(new ReadAssemblyWorkerFactory());
}

Worker *ReadAssemblyWorkerFactory::createWorker(Actor *a) {
    return new ReadAssemblyWorker(a);
}

} // LocalWorkflow
} // U2
