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
    return new ReadAssemblyTask(url, context);
}

void ReadAssemblyWorker::sl_taskFinished() {
    ReadAssemblyTask *t = qobject_cast<ReadAssemblyTask*>(sender());
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
ConvertToIndexedBamTask::ConvertToIndexedBamTask(const DocumentFormatId &_formatId, const GUrl &_url, WorkflowContext *_ctx)
: Task("Convert assembly file to sorted BAM", TaskFlag_None), formatId(_formatId), url(_url), ctx(_ctx)
{

}

void ConvertToIndexedBamTask::run() {
    AppFileStorage *fileStorage = AppContext::getAppFileStorage();
    CHECK_EXT(NULL != fileStorage, stateInfo.setError("NULL file storage"), );

    QString cashedSortedBam = FileStorageUtils::getSortedBamUrl(url.getURLString(), ctx->getWorkflowProcess());
    if (!cashedSortedBam.isEmpty()) {
        result = cashedSortedBam;
        return;
    }

    GUrl bamUrl = url;
    if (BaseDocumentFormats::SAM == formatId) {
        QString bam = FileStorageUtils::getSamToBamConvertInfo(url.getURLString(), ctx->getWorkflowProcess());
        if (bam.isEmpty()) {
            QString dir = fileStorage->createDirectory();
            bamUrl = dir + "/" + url.fileName() + ".bam";
            BAMUtils::convertSamToBam(url, bamUrl, stateInfo);
            CHECK_OP(stateInfo, );

            FileStorageUtils::addSamToBamConvertInfo(url.getURLString(), bamUrl.getURLString(), ctx->getWorkflowProcess());
        } else {
            bamUrl = bam;
        }
    } else {
        CHECK_EXT(BaseDocumentFormats::BAM == formatId, setError("Only BAM/SAM files could be converted"), );
    }

    bool sorted = BAMUtils::isSortedBam(bamUrl, stateInfo);
    CHECK_OP(stateInfo, );

    GUrl sortedBamUrl = bamUrl;
    if (!sorted) {
        QString dir = fileStorage->createDirectory();
        QString baseName;
        if (dir.isEmpty()) {
            baseName = bamUrl.getURLString();
        } else {
            baseName = dir + "/" + bamUrl.fileName();
        }
        baseName +=  ".sorted";
        sortedBamUrl = BAMUtils::sortBam(bamUrl, baseName, stateInfo);
        CHECK_OP(stateInfo, );
    }

    bool indexed = sorted && BAMUtils::hasValidBamIndex(sortedBamUrl);
    if (!indexed) {
        BAMUtils::createBamIndex(sortedBamUrl, stateInfo);
        CHECK_OP(stateInfo, );
    }

    // if the file was sorted then it is needed to be saved in the file storage
    if (!sorted) {
        FileStorageUtils::addSortedBamUrl(bamUrl.getURLString(), sortedBamUrl.getURLString(), ctx->getWorkflowProcess());
    }
    result = sortedBamUrl;
}

GUrl ConvertToIndexedBamTask::getResultUrl() const {
    return result;
}

ReadAssemblyTask::ReadAssemblyTask(const QString &_url, WorkflowContext *_ctx)
: Task(tr("Read assembly from %1").arg(_url), TaskFlag_None),
url(_url), ctx(_ctx), format(NULL), doc(NULL), convertTask(NULL), importTask(NULL)
{

}

static bool isConvertingFormat(const DocumentFormatId &formatId) {
    return (BaseDocumentFormats::SAM == formatId || BaseDocumentFormats::BAM == formatId);
}

void ReadAssemblyTask::prepare() {
    QFileInfo fi(url);
    if(!fi.exists()){
        stateInfo.setError(tr("File '%1' not exists").arg(url));
        return;
    }

    FormatDetectionConfig conf;
    conf.useImporters = true;
    conf.excludeHiddenFormats = false;
    QList<FormatDetectionResult> fs = DocumentUtils::detectFormat(url, conf);

    foreach (const FormatDetectionResult &f, fs) {
        if (NULL != f.format) {
            if (isConvertingFormat(f.format->getFormatId())) {
                convertTask = new ConvertToIndexedBamTask(f.format->getFormatId(), url, ctx);
                addSubTask(convertTask);
                return;
            }

            const QSet<GObjectType> &types = f.format->getSupportedObjectTypes();
            if (types.contains(GObjectTypes::ASSEMBLY)) {
                format = f.format;
                break;
            }
        } else if (NULL != f.importer) {
            U2OpStatusImpl os;
            U2DbiRef dbiRef = ctx->getDataStorage()->createTmpDbi(os);
            SAFE_POINT_OP(os, );

            QVariantMap hints;
            hints.insert(DocumentFormat::DBI_REF_HINT, qVariantFromValue(dbiRef));
            importTask = f.importer->createImportTask(f, false, hints);
            addSubTask(importTask);
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
    CHECK(NULL != subTask, result);
    CHECK(!subTask->hasError(), result);

    if (importTask == subTask) {
        doc = importTask->takeDocument(false);
    } else if (convertTask == subTask) {
        url = convertTask->getResultUrl().getURLString();
        format = AppContext::getDocumentFormatRegistry()->getFormatById(BaseDocumentFormats::BAM);
    }
    return result;
}

void ReadAssemblyTask::run() {
    if (NULL == format && NULL == doc) {
        return;
    }

    std::auto_ptr<Document> docPtr(NULL);
    bool useGC = true;
    if (NULL == doc) {
        useGC = false;
        ioLog.info(tr("Reading assembly from %1 [%2]").arg(url).arg(format->getFormatName()));
        IOAdapterFactory* iof = AppContext::getIOAdapterRegistry()->getIOAdapterFactoryById(IOAdapterUtils::url2io(url));
        QVariantMap hints;
        {
            // TODO: fix this hardcoded DBI id recognition
            U2DbiFactoryId fId;
            if (BaseDocumentFormats::UGENEDB == format->getFormatId()) {
                fId = DEFAULT_DBI_ID;
            } else if (BaseDocumentFormats::BAM == format->getFormatId()) {
                SAFE_POINT(NULL != convertTask, "Internal error! Converting stage is missed", );
                fId = BAM_DBI_ID;
            }
            U2DbiFactory *dbiFactory = AppContext::getDbiRegistry()->getDbiFactoryById(fId);
            SAFE_POINT(NULL != dbiFactory, QString("Unknown dbi factory id: %").arg(fId), );

            U2OpStatusImpl os;
            U2DbiRef dbiRef(dbiFactory->getId(), U2DataId(url.toAscii()));
            ctx->getDataStorage()->openDbi(dbiRef, os);
            CHECK_OP(os, );

            hints.insert(DocumentFormat::DBI_REF_HINT, qVariantFromValue(dbiRef));
        }
        docPtr.reset(format->loadDocument(iof, url, hints, stateInfo));
        CHECK_OP(stateInfo, );
    } else {
        useGC = true;
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
        SharedDbiDataHandler handler = ctx->getDataStorage()->getDataHandler(assemblyObj->getEntityRef(), useGC);
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
