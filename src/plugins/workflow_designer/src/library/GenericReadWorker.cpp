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

#include "GenericReadWorker.h"
#include "GenericReadActor.h"
#include "CoreLib.h"

#include <U2Core/AppContext.h>
#include <U2Core/AppResources.h>
#include <U2Core/DocumentModel.h>
#include <U2Core/IOAdapter.h>
#include <U2Core/IOAdapterUtils.h>
#include <U2Core/Log.h>
#include <U2Core/ProjectModel.h>
#include <U2Core/DocumentUtils.h>
#include <U2Core/GObjectTypes.h>
#include <U2Core/GObjectUtils.h>
#include <U2Core/GObjectRelationRoles.h>
#include <U2Core/DNASequenceObject.h>
#include <U2Core/MAlignmentImporter.h>
#include <U2Core/MAlignmentObject.h>
#include <U2Core/AnnotationTableObject.h>
#include <U2Core/MSAUtils.h>
#include <U2Core/U2SafePoints.h>
#include <U2Core/U2DbiRegistry.h>
#include <U2Core/U2OpStatusUtils.h>
#include <U2Core/U2SequenceUtils.h>
#include <U2Core/ZlibAdapter.h>

#include <U2Lang/BaseSlots.h>
#include <U2Lang/BaseAttributes.h>
#include <U2Lang/CoreLibConstants.h>
#include <U2Lang/Dataset.h>
#include <U2Lang/NoFailTaskWrapper.h>
#include <U2Lang/WorkflowEnv.h>
#include <U2Lang/WorkflowMonitor.h>

#include <U2Formats/DocumentFormatUtils.h>

#include <memory>

namespace U2 {
using namespace Workflow;
namespace LocalWorkflow {

/**************************
 * GenericDocReader
 **************************/
void GenericDocReader::init() {
    assert(ports.size() == 1);
    ch = ports.values().first();
    Attribute *urlAttr = actor->getParameter(BaseAttributes::URL_IN_ATTRIBUTE().getId());
    QList<Dataset> sets = urlAttr->getAttributeValue< QList<Dataset> >(context);
    files = new DatasetFilesIterator(sets);
    connect(files, SIGNAL(si_datasetEnded()), SLOT(sl_datasetEnded()));
}

GenericDocReader::~GenericDocReader() {
    delete files;
}

Task * GenericDocReader::tick() {
    files->tryEmitDatasetEnded();

    bool sendMessages = !cache.isEmpty();
    if (sendMessages) {
        while (!cache.isEmpty()) {
            ch->put(cache.takeFirst());
        }
    }

    if (!sendMessages && files->hasNext()) {
        QString newUrl = files->getNextFile();
        Task *t = createReadTask(newUrl, files->getLastDatasetName());
        NoFailTaskWrapper *wrapper = new NoFailTaskWrapper(t);
        connect(wrapper, SIGNAL(si_stateChanged()), SLOT(sl_taskFinished()));
        return wrapper;
    } else if (!files->hasNext()) {
        // the cache is empty and the no more URLs -> finish the worker
        setDone();
        ch->setEnded();
    }
    return NULL;
}

bool GenericDocReader::isDone() {
    return BaseWorker::isDone() && cache.isEmpty();
}

void GenericDocReader::sl_taskFinished() {
    NoFailTaskWrapper *wrapper = qobject_cast<NoFailTaskWrapper*>(sender());
    SAFE_POINT(NULL != wrapper, "NULL wrapper task", );
    Task *t = wrapper->originalTask();
    CHECK(t->isFinished(), );
    if (t->hasError()) {
        monitor()->addTaskError(wrapper, t->getError());
        return;
    }
    onTaskFinished(t);
}

void GenericDocReader::sl_datasetEnded() {

}

/**************************
 * GenericMSAReader
 **************************/
void GenericMSAReader::init() {
    GenericDocReader::init();
    mtype = WorkflowEnv::getDataTypeRegistry()->getById(GenericMAActorProto::TYPE);
}

void GenericMSAReader::onTaskFinished(Task *task) {
    LoadMSATask *t = qobject_cast<LoadMSATask*>(task);
    foreach(const QVariant& msaHandler, t->results) {
        QVariantMap m;
        m[BaseSlots::URL_SLOT().getId()] = t->url;
        m[BaseSlots::DATASET_SLOT().getId()] = t->datasetName;
        m[BaseSlots::MULTIPLE_ALIGNMENT_SLOT().getId()] = msaHandler;
        cache.append(Message(mtype, m));
    }
}

/**************************
 * LoadMSATask
 **************************/
LoadMSATask::LoadMSATask(const QString &_url, const QString &_datasetName, DbiDataStorage* _storage)
: Task(tr("Read MSA from %1").arg(_url), TaskFlag_None),
  url(_url),
  datasetName(_datasetName),
  storage(_storage)
{

}

void LoadMSATask::prepare() {
    int memUseMB = 0;
    QFileInfo file(url);
    memUseMB = file.size() / (1024*1024);
    IOAdapterFactory* iof = AppContext::getIOAdapterRegistry()->getIOAdapterFactoryById(IOAdapterUtils::url2io(url));
    if (iof->getAdapterId() == BaseIOAdapters::GZIPPED_LOCAL_FILE || iof->getAdapterId() == BaseIOAdapters::GZIPPED_HTTP_FILE) {
        memUseMB *= 2.5; //Need to calculate compress level
    }
    coreLog.trace(QString("load document:Memory resource %1").arg(memUseMB));

    if (memUseMB > 0) {
        addTaskResource(TaskResourceUsage(RESOURCE_MEMORY, memUseMB, false));
    }
}

void LoadMSATask::run() {
    QFileInfo fi(url);
    if(!fi.exists()){
        stateInfo.setError(  tr("File '%1' not exists").arg(url) );
        return;
    }
    DocumentFormat* format = NULL;
    QList<DocumentFormat*> fs = DocumentUtils::toFormats(DocumentUtils::detectFormat(url));
    foreach(DocumentFormat* f, fs) {
        if (f->getSupportedObjectTypes().contains(GObjectTypes::MULTIPLE_ALIGNMENT)) {
            format = f;
            break;
        }
    }
    if (format == NULL) {
        foreach(DocumentFormat* f, fs) {
            if (f->getSupportedObjectTypes().contains(GObjectTypes::SEQUENCE)) {
                format = f;
                break;
            }
        }
    }
    if (format == NULL) {
        stateInfo.setError( tr("Unsupported document format: %1").arg(url) );
        return;
    }
    ioLog.info(tr("Reading MSA from %1 [%2]").arg(url).arg(format->getFormatName()));
    IOAdapterFactory* iof = AppContext::getIOAdapterRegistry()->getIOAdapterFactoryById(IOAdapterUtils::url2io(url));

    cfg[DocumentFormat::DBI_REF_HINT] = qVariantFromValue(storage->getDbiRef());
    cfg[DocumentReadingMode_DontMakeUniqueNames] = true;
    QScopedPointer<Document> doc(format->loadDocument(iof, url, cfg, stateInfo));
    CHECK_OP(stateInfo, );
    doc->setDocumentOwnsDbiResources(false);

    if (!doc->findGObjectByType(GObjectTypes::MULTIPLE_ALIGNMENT).isEmpty()) {
        foreach(GObject* go, doc->findGObjectByType(GObjectTypes::MULTIPLE_ALIGNMENT)) {
            SharedDbiDataHandler handler = storage->getDataHandler(go->getEntityRef());
            QVariant res = qVariantFromValue<SharedDbiDataHandler>(handler);
            results.append(res);
        }
    } else {
        MAlignment ma = MSAUtils::seq2ma(doc->findGObjectByType(GObjectTypes::SEQUENCE), stateInfo);

        U2EntityRef msaRef = MAlignmentImporter::createAlignment(storage->getDbiRef(), ma, stateInfo);
        CHECK_OP(stateInfo, );

        SharedDbiDataHandler handler = storage->getDataHandler(msaRef);
        QVariant res = qVariantFromValue<SharedDbiDataHandler>(handler);
        results.append(res);
    }
}

/**************************
 * GenericSeqReader
 **************************/
void GenericSeqReader::init() {
    GenericDocReader::init();
    mtype = WorkflowEnv::getDataTypeRegistry()->getById(GenericSeqActorProto::TYPE);
    GenericSeqActorProto::Mode mode = GenericSeqActorProto::Mode(actor->getParameter(
                                                                    GenericSeqActorProto::MODE_ATTR)->getAttributeValue<int>(context));
    if (GenericSeqActorProto::MERGE == mode) {
        QString mergeToken = DocumentReadingMode_SequenceMergeGapSize;
        cfg[mergeToken] = actor->getParameter(GenericSeqActorProto::GAP_ATTR)->getAttributeValue<int>(context);
        cfg[GenericSeqActorProto::LIMIT_ATTR] = 0; // no limit in merge mode
    } else {        
        cfg[GenericSeqActorProto::LIMIT_ATTR] = actor->getParameter(GenericSeqActorProto::LIMIT_ATTR)->getAttributeValue<int>(context);
    }
    selector.accExpr = actor->getParameter(GenericSeqActorProto::ACC_ATTR)->getAttributeValue<QString>(context);
}

Task * GenericSeqReader::createReadTask(const QString &url, const QString &datasetName) {
    QVariantMap hints = cfg;
    hints[BaseSlots::DATASET_SLOT().getId()] = datasetName;
    return new LoadSeqTask(url, hints, &selector, context->getDataStorage());
}

void GenericSeqReader::onTaskFinished(Task *task) {
    LoadSeqTask* t = qobject_cast<LoadSeqTask*>(task);
    int limit = cfg[GenericSeqActorProto::LIMIT_ATTR].toInt();
    int currentCount = 0;
    foreach(const QVariantMap& m, t->results) {
        if (0 != limit && currentCount >= limit) {
            break;
        }
        cache.append(Message(mtype, m));
        currentCount++;
    }
    t->results.clear();
}

/**************************
 * LoadSeqTask
 **************************/
void LoadSeqTask::prepare() {
    QFileInfo fi(url);
    if(!fi.exists()){
        stateInfo.setError(tr("File '%1' not exists").arg(url));
        return;
    }

    QList<DocumentFormat*> fs = DocumentUtils::toFormats(DocumentUtils::detectFormat(url));
    foreach (DocumentFormat *f, fs) {
        const QSet<GObjectType>& types = f->getSupportedObjectTypes();
        if (types.contains(GObjectTypes::SEQUENCE) || types.contains(GObjectTypes::MULTIPLE_ALIGNMENT)) {
            format = f;
            break;
        }
    }
    if (format == NULL) {
        stateInfo.setError(tr("Unsupported document format: %1").arg(url));
        return;
    }

    const QSet<GObjectType> &types = format->getSupportedObjectTypes();
    if (!types.contains(GObjectTypes::ANNOTATION_TABLE)) {
        // no memory resources should be reserved for this file, because
        // sequences and alignments are stored into the database
        return;
    }

    int memUseMB = 0;
    QFileInfo file(url);
    memUseMB = file.size() / (1024*1024);
    IOAdapterFactory* iof = AppContext::getIOAdapterRegistry()->getIOAdapterFactoryById(IOAdapterUtils::url2io(url));
    if (iof->getAdapterId() == BaseIOAdapters::GZIPPED_LOCAL_FILE) {
        qint64 bytes = ZlibAdapter::getUncompressedFileSizeInBytes(url);
        if (bytes > 0) {
            memUseMB = bytes / (1024*1024);
        }
    } else if (iof->getAdapterId() == BaseIOAdapters::GZIPPED_HTTP_FILE) {
        memUseMB *= 2.5; //Need to calculate compress level
    }
    coreLog.trace(QString("load document:Memory resource %1").arg(memUseMB));

    if (memUseMB > 0) {
        addTaskResource(TaskResourceUsage(RESOURCE_MEMORY, memUseMB, false));
    }
}

void LoadSeqTask::run() {
    CHECK(NULL != format, );
    ioLog.info(tr("Reading sequences from %1 [%2]").arg(url).arg(format->getFormatName()));
    IOAdapterFactory* iof = AppContext::getIOAdapterRegistry()->getIOAdapterFactoryById(IOAdapterUtils::url2io(url));
    cfg[DocumentFormat::DBI_REF_HINT] = qVariantFromValue(storage->getDbiRef());
    cfg[DocumentReadingMode_DontMakeUniqueNames] = true;
    QScopedPointer<Document> doc(format->loadDocument(iof, url, cfg, stateInfo));
    CHECK_OP(stateInfo, );
    doc->setDocumentOwnsDbiResources(false);

    DbiOperationsBlock opBlock(storage->getDbiRef(), stateInfo);
    const QSet<GObjectType>& types = format->getSupportedObjectTypes();
    if (types.contains(GObjectTypes::SEQUENCE)) {
        QList<GObject*> seqObjs = doc->findGObjectByType(GObjectTypes::SEQUENCE);
        QList<GObject*> annObjs = doc->findGObjectByType(GObjectTypes::ANNOTATION_TABLE);
        QList<GObject*> allLoadedAnnotations = doc->findGObjectByType(GObjectTypes::ANNOTATION_TABLE);
        foreach ( GObject *go, seqObjs ) {
            SAFE_POINT( NULL != go, "Invalid object encountered!", );
            if ( !selector->objectMatches( static_cast<U2SequenceObject *>( go ) ) ) {
                continue;
            }
            QVariantMap m;
            m[BaseSlots::URL_SLOT( ).getId( )] = url;
            m[BaseSlots::DATASET_SLOT( ).getId( )] = cfg.value( BaseSlots::DATASET_SLOT( ).getId( ), "" );
            SharedDbiDataHandler handler = storage->getDataHandler( go->getEntityRef( ) );
            m[BaseSlots::DNA_SEQUENCE_SLOT( ).getId( )] = qVariantFromValue<SharedDbiDataHandler>( handler );
            QList<GObject *> annotations = GObjectUtils::findObjectsRelatedToObjectByRole( go,
                GObjectTypes::ANNOTATION_TABLE, GObjectRelationRole::SEQUENCE,
                allLoadedAnnotations, UOF_LoadedOnly );
            if ( !annotations.isEmpty( ) ) {
                QList<AnnotationData> l;
                foreach (GObject *annGObj, annotations ) {
                    AnnotationTableObject *att = qobject_cast<AnnotationTableObject *>( annGObj );
                    foreach ( const Annotation &a, att->getAnnotations( ) ) {
                        l << a.getData( );
                    }
                    annObjs.removeAll( annGObj );
                }
                const SharedDbiDataHandler tableId = storage->putAnnotationTable( l );
                m.insert( BaseSlots::ANNOTATION_TABLE_SLOT( ).getId( ),
                    qVariantFromValue<SharedDbiDataHandler>( tableId ) );
            }
            results.append( m );
        }

        // if there are annotations that are not connected to a sequence -> put them  independently
        foreach ( GObject *annObj, annObjs ) {
            AnnotationTableObject *att = qobject_cast<AnnotationTableObject *>( annObj );
            if ( att->findRelatedObjectsByRole( GObjectRelationRole::SEQUENCE ).isEmpty( ) ) {
                SAFE_POINT( NULL != att, "Invalid annotation table object encountered!", );
                QVariantMap m;
                m.insert( BaseSlots::URL_SLOT( ).getId( ), url );

                QList<AnnotationData> l;
                foreach ( const Annotation &a, att->getAnnotations( ) ) {
                    l << a.getData( );
                }
                const SharedDbiDataHandler tableId = storage->putAnnotationTable( l );
                m.insert( BaseSlots::ANNOTATION_TABLE_SLOT( ).getId( ),
                    qVariantFromValue<SharedDbiDataHandler>( tableId ) );
                results.append( m );
            }
        }
    } else {
        //TODO merge seqs from alignment
        //             QString mergeToken = MERGE_MULTI_DOC_GAP_SIZE_SETTINGS;
        //             bool merge = cfg.contains(mergeToken);
        //             int gaps = cfg.value(mergeToken).toInt();
        U2OpStatus2Log os;
        foreach(GObject* go, doc->findGObjectByType(GObjectTypes::MULTIPLE_ALIGNMENT)) {
            foreach(const DNASequence& s, MSAUtils::ma2seq(((MAlignmentObject*)go)->getMAlignment(), false)) {
                if (!selector->matches(s)) {
                    continue;
                }
                QVariantMap m;
                U2EntityRef seqRef = U2SequenceUtils::import(storage->getDbiRef(), s, os);
                CHECK_OP(os, );
                m[BaseSlots::URL_SLOT().getId()] = url;
                m[BaseSlots::DATASET_SLOT().getId()] = cfg.value(BaseSlots::DATASET_SLOT().getId(), "");
                SharedDbiDataHandler handler = storage->getDataHandler(seqRef);
                m[BaseSlots::DNA_SEQUENCE_SLOT().getId()] = qVariantFromValue<SharedDbiDataHandler>(handler);
                results.append(m);
            }
        }
    }
}

/**************************
 * DNASelector
 **************************/
bool DNASelector::matches( const DNASequence& dna) {
    if (accExpr.isEmpty()) {
        return true;
    }
    if (dna.info.contains(DNAInfo::ACCESSION)) {
        return dna.info.value(DNAInfo::ACCESSION).toString().contains(QRegExp(accExpr));
    }
    return dna.getName().contains(QRegExp(accExpr));
}

bool DNASelector::objectMatches( const U2SequenceObject *dna) {
    if (accExpr.isEmpty()) {
        return true;
    }
    QVariantMap info = dna->getSequenceInfo();
    if (info.contains(DNAInfo::ACCESSION)) {
        return info.value(DNAInfo::ACCESSION).toString().contains(QRegExp(accExpr));
    }
    return dna->getSequenceName().contains(QRegExp(accExpr));
}

} // Workflow namespace
} // U2 namespace
