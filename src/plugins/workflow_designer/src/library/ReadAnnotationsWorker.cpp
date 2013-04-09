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
#include <U2Core/AnnotationTableObject.h>
#include <U2Core/DocumentUtils.h>
#include <U2Core/IOAdapter.h>
#include <U2Core/IOAdapterUtils.h>
#include <U2Core/U2SafePoints.h>
#include <U2Core/ZlibAdapter.h>

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

#include "ReadAnnotationsWorker.h"

namespace U2 {
namespace LocalWorkflow {

const QString ReadAnnotationsWorkerFactory::ACTOR_ID("read-annotations");

/************************************************************************/
/* Worker */
/************************************************************************/
ReadAnnotationsWorker::ReadAnnotationsWorker(Actor *p)
: GenericDocReader(p), mode(ReadAnnotationsProto::SPLIT)
{

}

void ReadAnnotationsWorker::init() {
    GenericDocReader::init();
    mode = ReadAnnotationsProto::Mode(getValue<int>(ReadAnnotationsProto::MODE_ATTR));
    IntegralBus *outBus = dynamic_cast<IntegralBus*>(ch);
    assert(outBus);
    mtype = outBus->getBusType();
}

Task * ReadAnnotationsWorker::createReadTask(const QString &url, const QString &datasetName) {
    bool mergeAnnotations = (mode != ReadAnnotationsProto::SPLIT);
    return new ReadAnnotationsTask(url, datasetName, mergeAnnotations);
}

void ReadAnnotationsWorker::onTaskFinished(Task *task) {
    ReadAnnotationsTask *t = qobject_cast<ReadAnnotationsTask*>(task);
    if (ReadAnnotationsProto::MERGE_FILES == mode) {
        datasetData << t->takeResults();
        return;
    }

    sendData(t->takeResults());
}

void ReadAnnotationsWorker::sl_datasetEnded() {
    CHECK(datasetData.size() > 0, );
    QList<SharedAnnotationData> anns;
    foreach (const QVariantMap &m, datasetData) {
        anns << m[BaseSlots::ANNOTATION_TABLE_SLOT().getId()].value< QList<SharedAnnotationData> >();
    }
    QVariantMap m;
    m[BaseSlots::ANNOTATION_TABLE_SLOT().getId()] = qVariantFromValue< QList<SharedAnnotationData> >(anns);
    m[BaseSlots::DATASET_SLOT().getId()] = datasetData.first()[BaseSlots::DATASET_SLOT().getId()];

    sendData(QList<QVariantMap>() << m);
    datasetData.clear();
}

void ReadAnnotationsWorker::sendData(const QList<QVariantMap> &data) {
    foreach(const QVariantMap &m, data) {
        cache.append(Message(mtype, m));
    }
}

/************************************************************************/
/* Factory */
/************************************************************************/
const QString ReadAnnotationsProto::MODE_ATTR("mode");
ReadAnnotationsProto::ReadAnnotationsProto()
: GenericReadDocProto(ReadAnnotationsWorkerFactory::ACTOR_ID)
{
    setDisplayName(ReadAnnotationsWorker::tr("Read Annotations"));
    setDocumentation(ReadAnnotationsWorker::tr("Reads annotations from files"));

    { // ports description
        QMap<Descriptor, DataTypePtr> outTypeMap;
        outTypeMap[BaseSlots::ANNOTATION_TABLE_SLOT()] = BaseTypes::ANNOTATION_TABLE_TYPE();
        outTypeMap[BaseSlots::URL_SLOT()] = BaseTypes::STRING_TYPE();
        outTypeMap[BaseSlots::DATASET_SLOT()] = BaseTypes::STRING_TYPE();
        DataTypePtr outTypeSet(new MapDataType(BasePorts::OUT_ANNOTATIONS_PORT_ID(), outTypeMap));

        Descriptor outDesc(BasePorts::OUT_ANNOTATIONS_PORT_ID(),
            ReadAnnotationsWorker::tr("Annotations"),
            ReadAnnotationsWorker::tr("Annotations."));

        ports << new PortDescriptor(outDesc, outTypeSet, false, true);
    }

    Descriptor md(ReadAnnotationsProto::MODE_ATTR, ReadAnnotationsWorker::tr("Mode"),
        ReadAnnotationsWorker::tr("If the file contains more than one annotation table, <i>Split</i> mode sends them \"as is\" to the output, "
        "while <i>Merge</i> appends all the annotation tables and outputs the sole merged annotation table."
        "In <i>Merge files</i> is the same as <i>Merge</m> but it operates with all annotation tables from all files of one dataset."));


    attrs << new Attribute(md, BaseTypes::NUM_TYPE(), true, MERGE);

    QMap<QString, PropertyDelegate*> delegates;
    {
        QVariantMap modeMap;
        QString splitStr = ReadAnnotationsWorker::tr("Split");
        QString mergeStr = ReadAnnotationsWorker::tr("Merge");
        QString mergeFilesStr = ReadAnnotationsWorker::tr("Merge files");
        modeMap[splitStr] = SPLIT;
        modeMap[mergeStr] = MERGE;
        modeMap[mergeFilesStr] = MERGE_FILES;
        getEditor()->addDelegate(new ComboBoxDelegate(modeMap), MODE_ATTR);
    }

    setPrompter(new ReadDocPrompter(ReadAnnotationsWorker::tr("Reads annotations from <u>%1</u>.")));
    if (AppContext::isGUIMode()) {
        setIcon(GUIUtils::createRoundIcon(QColor(85,85,255), 22));
    }

}

void ReadAnnotationsWorkerFactory::init() {
    ActorPrototype *proto = new ReadAnnotationsProto();
    WorkflowEnv::getProtoRegistry()->registerProto(BaseActorCategories::CATEGORY_DATASRC(), proto);
    WorkflowEnv::getDomainRegistry()->getById(LocalDomainFactory::ID)->registerEntry(new ReadAnnotationsWorkerFactory());
}

Worker * ReadAnnotationsWorkerFactory::createWorker(Actor *a) {
    return new ReadAnnotationsWorker(a);
}

/************************************************************************/
/* Task */
/************************************************************************/
ReadAnnotationsTask::ReadAnnotationsTask(const QString &_url, const QString &_datasetName, bool _mergeAnnotations)
: Task(tr("Read annotations from %1").arg(_url), TaskFlag_None), url(_url), datasetName(_datasetName), mergeAnnotations(_mergeAnnotations)
{

}

const QString & ReadAnnotationsTask::getDatasetName() const {
    return datasetName;
}

void ReadAnnotationsTask::prepare() {
    int memUseMB = 0;
    QFileInfo file(url);
    memUseMB = file.size() / (1024*1024) + 1;
    IOAdapterFactory *iof = AppContext::getIOAdapterRegistry()->getIOAdapterFactoryById(IOAdapterUtils::url2io(url));
    if (BaseIOAdapters::GZIPPED_LOCAL_FILE == iof->getAdapterId()) {
        memUseMB = ZlibAdapter::getUncompressedFileSizeInBytes(url) / (1024*1024) + 1;
    } else if (BaseIOAdapters::GZIPPED_HTTP_FILE == iof->getAdapterId()) {
        memUseMB *= 2.5; //Need to calculate compress level
    }
    coreLog.trace(QString("Load annotations: Memory resource %1").arg(memUseMB));

    if (memUseMB > 0) {
        addTaskResource(TaskResourceUsage(RESOURCE_MEMORY, memUseMB, false));
    }
}

void ReadAnnotationsTask::run() {
    QFileInfo fi(url);
    CHECK_EXT(fi.exists(), stateInfo.setError(tr("File '%1' does not exist").arg(url)), );

    DocumentFormat *format = NULL;
    QList<DocumentFormat*> fs = DocumentUtils::toFormats(DocumentUtils::detectFormat(url));
    foreach(DocumentFormat *f, fs) {
        if (f->getSupportedObjectTypes().contains(GObjectTypes::ANNOTATION_TABLE)) {
            format = f;
            break;
        }
    }
    CHECK_EXT(NULL != format, stateInfo.setError(tr("Unsupported document format")), );

    ioLog.info(tr("Reading annotations from %1 [%2]").arg(url).arg(format->getFormatName()));
    IOAdapterFactory *iof = AppContext::getIOAdapterRegistry()->
        getIOAdapterFactoryById(IOAdapterUtils::url2io(url));
    std::auto_ptr<Document> doc(format->loadDocument(iof, url, QVariantMap(), stateInfo));
    CHECK_OP(stateInfo, );

    QList<GObject*> annsObjList = doc->findGObjectByType(GObjectTypes::ANNOTATION_TABLE);

    QVariantMap m;
    m[BaseSlots::URL_SLOT().getId()] = url;
    m[BaseSlots::DATASET_SLOT().getId()] = datasetName;

    QList<SharedAnnotationData> dataList;

    foreach(GObject *go, annsObjList) {
        AnnotationTableObject *annsObj = dynamic_cast<AnnotationTableObject*>(go);
        CHECK_EXT(NULL != annsObj, stateInfo.setError("NULL annotations object"), );

        if (!mergeAnnotations){
            dataList.clear();
        }
        foreach(Annotation *a, annsObj->getAnnotations()) {
            dataList << a->data();
        }
       
        if (!mergeAnnotations){
            m[BaseSlots::ANNOTATION_TABLE_SLOT().getId()] = qVariantFromValue<QList<SharedAnnotationData> >(dataList);
            results.append(m);
        }
    }

    if (mergeAnnotations && !annsObjList.isEmpty()){
        m[BaseSlots::ANNOTATION_TABLE_SLOT().getId()] = qVariantFromValue<QList<SharedAnnotationData> >(dataList);
        results.append(m);
    }
}

QList<QVariantMap> ReadAnnotationsTask::takeResults() {
    QList<QVariantMap> ret = results;
    results.clear();

    return ret;
}

void ReadAnnotationsTask::cleanup() {
    results.clear();
}

} // LocalWorkflow
} // U2
