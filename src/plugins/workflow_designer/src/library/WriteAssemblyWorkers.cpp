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
#include <U2Core/DocumentModel.h>
#include <U2Core/DocumentUtils.h>
#include <U2Core/GUrlUtils.h>
#include <U2Core/MultiTask.h>
#include <U2Core/U2AssemblyDbi.h>
#include <U2Core/U2SafePoints.h>

#include <U2Designer/DelegateEditors.h>

#include <U2Formats/BAMUtils.h>

#include <U2Lang/ActorPrototypeRegistry.h>
#include <U2Lang/BaseActorCategories.h>
#include <U2Lang/BaseAttributes.h>
#include <U2Lang/BasePorts.h>
#include <U2Lang/BaseSlots.h>
#include <U2Lang/BaseTypes.h>
#include <U2Lang/WorkflowEnv.h>

#include "DocActors.h"

#include "WriteAssemblyWorkers.h"

namespace U2 {
namespace LocalWorkflow {

const QString WriteAssemblyWorkerFactory::ACTOR_ID("write-assembly");
const QString INDEX_ATTRIBUTE_ID = "build-index";

/************************************************************************/
/* BaseWriteAssemblyWorker */
/************************************************************************/
BaseWriteAssemblyWorker::BaseWriteAssemblyWorker(Actor *a)
: BaseDocWriter(a)
{

}

void BaseWriteAssemblyWorker::data2doc(Document *doc, const QVariantMap &data) {
    CHECK(data.contains(BaseSlots::ASSEMBLY_SLOT().getId()), );
    SharedDbiDataHandler assemblyId = data[BaseSlots::ASSEMBLY_SLOT().getId()].value<SharedDbiDataHandler>();
    std::auto_ptr<AssemblyObject> assemblyObj(StorageUtils::getAssemblyObject(context->getDataStorage(), assemblyId));
    SAFE_POINT(NULL != assemblyObj.get(), tr("Assembly writer: NULL assembly object"), );

    QString objName = assemblyObj->getGObjectName();
    if (doc->findGObjectByName(objName)) {
        objName = BaseDocWriter::getUniqueObjectName(doc, objName);
        assemblyObj->setGObjectName(objName);
    }
    algoLog.trace(QString("Adding assembly [%1] to %3 doc %2").arg(objName).arg(doc->getURLString()).arg(doc->getDocumentFormat()->getFormatName()));

    DocumentFormat *docFormat = doc->getDocumentFormat();
    DocumentFormatId formatId = docFormat->getFormatId();
    if (docFormat->isObjectOpSupported(doc, DocumentFormat::DocObjectOp_Add, GObjectTypes::ASSEMBLY)) {
        doc->addObject(assemblyObj.get());
        assemblyObj.release();
    } else {
        algoLog.trace("Failed to add assembly object to document: op is not supported!");
    }
}

/************************************************************************/
/* WriteBAMWorker */
/************************************************************************/
WriteBAMWorker::WriteBAMWorker(Actor *a)
: BaseWriteAssemblyWorker(a), buildIndex(false)
{

}

void WriteBAMWorker::takeParameters(U2OpStatus &os) {
    BaseWriteAssemblyWorker::takeParameters(os);

    Attribute *indexAttr = actor->getParameter(INDEX_ATTRIBUTE_ID);
    CHECK(NULL != indexAttr, );
    buildIndex = indexAttr->getAttributePureValue().toBool();
}

bool WriteBAMWorker::isStreamingSupport() const {
    return false;
}

Task * WriteBAMWorker::getWriteDocTask(Document *doc, const SaveDocFlags &flags) {
    return new WriteBAMTask(doc, buildIndex, flags);
}

/************************************************************************/
/* WriteBAMTask */
/************************************************************************/
WriteBAMTask::WriteBAMTask(Document *_doc, bool _buildIndex, const SaveDocFlags &_flags)
: Task("Write BAM/SAM file", TaskFlag_None), doc(_doc), buildIndex(_buildIndex), flags(_flags)
{

}

void WriteBAMTask::run() {
    CHECK_EXT(NULL != doc, stateInfo.setError("NULL document"), );

    if (flags.testFlag(SaveDoc_Roll)) {
        QSet<QString> excludeFileNames = DocumentUtils::getNewDocFileNameExcludesHint();
        bool rollResult = GUrlUtils::renameFileWithNameRoll(doc->getURLString(), stateInfo, excludeFileNames, &coreLog);
        if (!rollResult) {
            return;
        }
    }

    BAMUtils::writeDocument(doc, stateInfo);
    CHECK_OP(stateInfo, );

    // BAM only
    if (buildIndex && BaseDocumentFormats::BAM == doc->getDocumentFormatId()) {
        BAMUtils::createBamIndex(doc->getURL(), stateInfo);
    }
}

/************************************************************************/
/* WriteAssemblyWorkerFactory */
/************************************************************************/
void WriteAssemblyWorkerFactory::init() {
    DocumentFormatConstraints constr;
    constr.supportedObjectTypes.insert(GObjectTypes::ASSEMBLY);
    constr.addFlagToSupport(DocumentFormatFlag_SupportWriting);
    QList<DocumentFormatId> supportedFormats = AppContext::getDocumentFormatRegistry()->selectFormats(constr);
    CHECK(!supportedFormats.isEmpty(), );

    Descriptor inDesc(BasePorts::IN_ASSEMBLY_PORT_ID(),
        WriteBAMWorker::tr("Assembly"),
        WriteBAMWorker::tr("Assembly"));
    Descriptor protoDesc(WriteAssemblyWorkerFactory::ACTOR_ID,
        WriteBAMWorker::tr("Write Assembly"),
        WriteBAMWorker::tr("Writes all supplied assemblies to file(s) in selected format"));

    QList<PortDescriptor*> portDescs;
    {
        QMap<Descriptor, DataTypePtr> inTypeMap;
        Descriptor writeUrlD(BaseSlots::URL_SLOT().getId(),
            WriteBAMWorker::tr("Location"),
            WriteBAMWorker::tr("Location for writing data"));
        inTypeMap[writeUrlD] = BaseTypes::STRING_TYPE();
        inTypeMap[BaseSlots::ASSEMBLY_SLOT()] = BaseTypes::ASSEMBLY_TYPE();
        DataTypePtr writeAssemblyType(new MapDataType(BasePorts::IN_ASSEMBLY_PORT_ID(), inTypeMap));

        portDescs << new PortDescriptor(inDesc, writeAssemblyType, true);
    }

    QList<Attribute*> attrs;
    Attribute *docFormatAttr = NULL;
    {
        SAFE_POINT(!supportedFormats.isEmpty(), "No assembly formats", );
        docFormatAttr = new Attribute(BaseAttributes::DOCUMENT_FORMAT_ATTRIBUTE(), BaseTypes::STRING_TYPE(), false, 
            supportedFormats.contains(BaseDocumentFormats::BAM) ? BaseDocumentFormats::BAM : supportedFormats.first());
        attrs << docFormatAttr;

        Descriptor indexDescr(INDEX_ATTRIBUTE_ID,
            BaseWriteAssemblyWorker::tr("Build index (BAM only)"),
            BaseWriteAssemblyWorker::tr("Build BAM index for the target BAM file. The file .bai will be created in the same directory"));

        Attribute *indexAttr = new Attribute(indexDescr, BaseTypes::BOOL_TYPE(), false, true);
        indexAttr->addRelation(new VisibilityRelation(BaseAttributes::DOCUMENT_FORMAT_ATTRIBUTE().getId(), BaseDocumentFormats::BAM));
        attrs << indexAttr;
    }

    WriteDocActorProto *childProto = new WriteDocActorProto(protoDesc, GObjectTypes::ASSEMBLY, portDescs, inDesc.getId(), attrs);
    IntegralBusActorPrototype *proto = childProto;
    docFormatAttr->addRelation(new FileExtensionRelation(childProto->getUrlAttr()->getId(), docFormatAttr->getAttributePureValue().toString()));

    // set up delegates
    {
        QVariantMap formatsMap;
        foreach (const DocumentFormatId &fid, supportedFormats) {
            formatsMap[fid] = fid;
        }

        ComboBoxDelegate *formatComboDelegate = new ComboBoxDelegate(formatsMap);
        QObject::connect(formatComboDelegate, SIGNAL(si_valueChanged(const QString &)), childProto->getUrlDelegate(), SLOT(sl_formatChanged(const QString &)));
        proto->getEditor()->addDelegate(formatComboDelegate, BaseAttributes::DOCUMENT_FORMAT_ATTRIBUTE().getId());
    }
    proto->setPrompter(new WriteDocPrompter(WriteBAMWorker::tr("Save all assemblies from <u>%1</u> to <u>%2</u>."),
        BaseSlots::ASSEMBLY_SLOT().getId()));

    WorkflowEnv::getProtoRegistry()->registerProto(BaseActorCategories::CATEGORY_DATASINK(), proto);
    WorkflowEnv::getDomainRegistry()->getById(LocalDomainFactory::ID)->registerEntry(new WriteAssemblyWorkerFactory());
}

Worker *WriteAssemblyWorkerFactory::createWorker(Actor *a) {
    Attribute *formatAttr = a->getParameter(BaseAttributes::DOCUMENT_FORMAT_ATTRIBUTE().getId());
    QString formatId = formatAttr->getAttributePureValue().toString();
    if (BaseDocumentFormats::SAM == formatId
     || BaseDocumentFormats::BAM == formatId) {
         return new WriteBAMWorker(a);
    } else {
        return new BaseWriteAssemblyWorker(a);
    }
}

} // LocalWorkflow
} // U2
