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
#include <U2Core/FileStorageUtils.h>
#include <U2Core/DocumentImport.h>
#include <U2Core/DocumentProviderTask.h>
#include <U2Core/DocumentUtils.h>
#include <U2Core/IOAdapter.h>
#include <U2Core/IOAdapterUtils.h>
#include <U2Core/U2DbiRegistry.h>
#include <U2Core/U2OpStatusUtils.h>
#include <U2Core/U2SafePoints.h>

#include <U2Designer/DelegateEditors.h>

#include <U2Formats/BAMUtils.h>

#include <U2Gui/DialogUtils.h>
#include <U2Gui/GUIUtils.h>

#include <U2Lang/ActorPrototypeRegistry.h>
#include <U2Lang/BaseActorCategories.h>
#include <U2Lang/BaseAttributes.h>
#include <U2Lang/BasePorts.h>
#include <U2Lang/BaseSlots.h>
#include <U2Lang/BaseTypes.h>
#include <U2Lang/ReadDocumentTaskFactory.h>
#include <U2Lang/WorkflowEnv.h>
#include <U2Lang/WorkflowTasksRegistry.h>

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
    WorkflowTasksRegistry *registry = WorkflowEnv::getWorkflowTasksRegistry();
    SAFE_POINT(NULL != registry, "NULL WorkflowTasksRegistry", NULL);
    ReadDocumentTaskFactory *factory = registry->getReadDocumentTaskFactory(ReadFactories::READ_ASSEMBLY);
    SAFE_POINT(NULL != factory, QString("NULL WorkflowTasksRegistry: %1").arg(ReadFactories::READ_ASSEMBLY), NULL);

    return factory->createTask(url, QVariantMap(), context);
}

void ReadAssemblyWorker::sl_taskFinished() {
    ReadDocumentTask *t = qobject_cast<ReadDocumentTask*>(sender());
    if (!t->isFinished() || t->hasError()) {
        return;
    }
    QList<SharedDbiDataHandler> result = t->takeResult();
    QString url = t->getUrl();
    foreach(const SharedDbiDataHandler &handler, result) {
        QVariantMap m;
        m.insert(BaseSlots::URL_SLOT().getId(), url);
        m.insert(BaseSlots::ASSEMBLY_SLOT().getId(), qVariantFromValue<SharedDbiDataHandler>(handler));

        cache.append(Message(mtype, m));
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
