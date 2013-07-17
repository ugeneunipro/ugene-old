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

#include <U2Core/DocumentModel.h>
#include <U2Core/AppContext.h>
#include <U2Core/BaseDocumentFormats.h>
#include <U2Core/SaveDocumentTask.h>
#include <U2Core/QVariantUtils.h>
#include <U2Core/FailTask.h>
#include <U2Core/IOAdapter.h>
#include <U2Core/IOAdapterUtils.h>
#include <U2Core/DocumentUtils.h>
#include <U2Core/GUrlUtils.h>
#include <U2Core/U2SafePoints.h>
#include <U2Core/U2OpStatusUtils.h>
#include <U2Core/MultiTask.h>

#include <U2Lang/BaseTypes.h>
#include <U2Lang/BaseSlots.h>
#include <U2Lang/BasePorts.h>
#include <U2Lang/BaseAttributes.h>
#include <U2Lang/BaseActorCategories.h>
#include <U2Lang/CoreLibConstants.h>
#include <U2Lang/WorkflowEnv.h>
#include <U2Lang/ActorPrototypeRegistry.h>

#include <U2Designer/DelegateEditors.h>

#include <U2Gui/DialogUtils.h>

#include "ExportAnnotations2CSVTask.h"
#include "WriteAnnotationsWorker.h"

namespace U2 {
namespace LocalWorkflow {

const QString WriteAnnotationsWorkerFactory::ACTOR_ID("write-annotations");

static const QString WRITE_ANNOTATIONS_IN_TYPE_ID("write-annotations-in-type");
static const QString CSV_FORMAT_ID("csv");
static const QString ANNOTATIONS_NAME("annotations-name");
static const QString ANNOTATIONS_NAME_DEF_VAL("unknown features");
static const QString SEPARATOR("separator");
static const QString SEPARATOR_DEFAULT_VALUE (",");
static const QString WRITE_NAMES("write_names");

/*******************************
 * WriteAnnotationsWorker
 *******************************/
WriteAnnotationsWorker::~WriteAnnotationsWorker() {
    qDeleteAll(createdAnnotationObjects);
}

void WriteAnnotationsWorker::init() {
    annotationsPort = ports.value(BasePorts::IN_ANNOTATIONS_PORT_ID());
}

Task * WriteAnnotationsWorker::tick() {
    QString formatId = getValue<QString>(BaseAttributes::DOCUMENT_FORMAT_ATTRIBUTE().getId());
    DocumentFormat * format = AppContext::getDocumentFormatRegistry()->getFormatById(formatId);
    SaveDocFlags fl(getValue<uint>(BaseAttributes::FILE_MODE_ATTRIBUTE().getId()));
    if( formatId != CSV_FORMAT_ID && format == NULL ) {
        return new FailTask(tr("Unrecognized formatId: '%1'").arg(formatId));
    }

    QString seqName;

    while(annotationsPort->hasMessage()) {
        Message inputMessage = getMessageAndSetupScriptValues(annotationsPort);
        if (inputMessage.isEmpty()) {
            continue;
        }

        QString filepath = getValue<QString>(BaseAttributes::URL_OUT_ATTRIBUTE().getId());
        filepath = filepath.isEmpty() ? inputMessage.getData().toMap().value(BaseSlots::URL_SLOT().getId()).value<QString>() : filepath;
        if (filepath.isEmpty()) {
            return new FailTask(tr("Unspecified URL to write %1").arg(formatId));
        }
        filepath = context->absolutePath(filepath);
        SharedDbiDataHandler seqId = inputMessage.getData().toMap().value(BaseSlots::DNA_SEQUENCE_SLOT().getId()).value<SharedDbiDataHandler>();
        std::auto_ptr<U2SequenceObject> seqObj(StorageUtils::getSequenceObject(context->getDataStorage(), seqId));
        
        if (NULL != seqObj.get()) {
            seqName = seqObj->getSequenceName();
        }
        QStringList exts = formatId == CSV_FORMAT_ID ? QStringList("csv") : format->getSupportedDocumentFileExtensions();

        QString objName = getValue<QString>(ANNOTATIONS_NAME);
        if(objName.isEmpty()) {
            objName = ANNOTATIONS_NAME_DEF_VAL;
            coreLog.details(tr("Annotations name not specified. Default value used: '%1'").arg(objName));
        }
        AnnotationTableObject * att = NULL;
        if (annotationsByUrl.contains(filepath)) {
            att = annotationsByUrl.value(filepath);
        } else {
            att = new AnnotationTableObject(objName);
            annotationsByUrl.insert(filepath, att);
        }

        QList<SharedAnnotationData> atl = QVariantUtils::var2ftl(inputMessage.getData().toMap().
            value(BaseSlots::ANNOTATION_TABLE_SLOT().getId()).toList());
        bool isWriteNames = getValue<bool>(WRITE_NAMES);
        foreach(SharedAnnotationData ad, atl) {
            if(isWriteNames){
                U2Qualifier seqNameQual;
                seqNameQual.name = "Sequence Name";
                seqNameQual.value = seqName;
                ad->qualifiers.append(seqNameQual);
            }
            att->addAnnotation(new Annotation(ad));
        }
    } // while

    bool done = annotationsPort->isEnded();
    if (!done) {
        return NULL;
    }

    setDone();
    QList<Task*> taskList;
    QSet<QString> excludeFileNames = DocumentUtils::getNewDocFileNameExcludesHint();
    foreach (QString filepath, annotationsByUrl.keys()) {
        AnnotationTableObject *att = annotationsByUrl.value(filepath);

        if(formatId == CSV_FORMAT_ID) {
            createdAnnotationObjects << att; // will delete in destructor
            TaskStateInfo ti;
            if(fl.testFlag(SaveDoc_Roll) && !GUrlUtils::renameFileWithNameRoll(filepath, ti, excludeFileNames, &coreLog)) {
                return new FailTask(ti.getError());
            }
            taskList << new ExportAnnotations2CSVTask(att->getAnnotations(), QByteArray(), QString(), NULL, false, 
                  false, filepath, fl.testFlag(SaveDoc_Append)
                , getValue<QString>(SEPARATOR));
        } else {
            fl |= SaveDoc_DestroyAfter;
            IOAdapterFactory* iof = AppContext::getIOAdapterRegistry()->getIOAdapterFactoryById(IOAdapterUtils::url2io(filepath));
            DocumentFormat* df = AppContext::getDocumentFormatRegistry()->getFormatById(formatId);
            U2OpStatusImpl os;
            QVariantMap hints;
            U2DbiRef dbiRef = context->getDataStorage()->getDbiRef();
            hints[DocumentFormat::DBI_REF_HINT] = qVariantFromValue(dbiRef);
            Document * doc = df->createNewLoadedDocument(iof, filepath, os, hints);
            CHECK_OP(os, new FailTask(os.getError()));
            att->setModified(false);
            doc->addObject(att); // savedoc task will delete doc -> doc will delete att
            taskList << new SaveDocumentTask(doc, fl, excludeFileNames);
        }
    }
    if (taskList.isEmpty()) {
        return NULL;
    } else if (1 == taskList.size()) {
        return taskList.first();
    }
    return new MultiTask(tr("Save annotations"), taskList);
}

void WriteAnnotationsWorker::cleanup() {
}

/*******************************
 * WriteAnnotationsWorkerFactory
 *******************************/
void WriteAnnotationsWorkerFactory::init() {
    // ports description
    QList<PortDescriptor*> portDescs;
    {
        QMap<Descriptor, DataTypePtr> inM;
        inM[BaseSlots::ANNOTATION_TABLE_SLOT()] = BaseTypes::ANNOTATION_TABLE_LIST_TYPE();
        inM[BaseSlots::URL_SLOT()] = BaseTypes::STRING_TYPE();
        inM[BaseSlots::DNA_SEQUENCE_SLOT()] = BaseTypes::DNA_SEQUENCE_TYPE();
        DataTypePtr inSet(new MapDataType(WRITE_ANNOTATIONS_IN_TYPE_ID, inM));
        Descriptor inPortDesc(BasePorts::IN_ANNOTATIONS_PORT_ID(), WriteAnnotationsWorker::tr("Input annotations"), 
            WriteAnnotationsWorker::tr("Input annotations which will be written to output file"));
        portDescs << new PortDescriptor(inPortDesc, inSet, true);
    }
    DocumentFormatConstraints constr;
    constr.supportedObjectTypes.insert( GObjectTypes::ANNOTATION_TABLE );
    constr.addFlagToSupport(DocumentFormatFlag_SupportWriting);
    QList<DocumentFormatId> supportedFormats = AppContext::getDocumentFormatRegistry()->selectFormats( constr );
    supportedFormats.append(CSV_FORMAT_ID);
    DocumentFormatId format = supportedFormats.contains(BaseDocumentFormats::PLAIN_GENBANK) ? BaseDocumentFormats::PLAIN_GENBANK : supportedFormats.first();
    // attributes description
    QList<Attribute*> attrs;
    {
        Attribute *docFormatAttr = new Attribute(BaseAttributes::DOCUMENT_FORMAT_ATTRIBUTE(), BaseTypes::STRING_TYPE(), false, format);
        Attribute *urlAttr = new Attribute(BaseAttributes::URL_OUT_ATTRIBUTE(), BaseTypes::STRING_TYPE(), false );
        attrs << docFormatAttr;
        attrs << urlAttr;
        attrs << new Attribute(BaseAttributes::FILE_MODE_ATTRIBUTE(), BaseTypes::NUM_TYPE(), false, SaveDoc_Roll);
        Descriptor annotationsNameDesc(ANNOTATIONS_NAME, WriteAnnotationsWorker::tr("Annotations name"),
            WriteAnnotationsWorker::tr("Annotations name: Name of the saved"
            " annotations. This option is only available for document formats"
            " that support saving of annotations names."));
        Attribute *nameAttr = new Attribute(annotationsNameDesc, BaseTypes::STRING_TYPE(), false, QVariant(ANNOTATIONS_NAME_DEF_VAL));
        attrs << nameAttr;
        Descriptor separatorDesc(SEPARATOR, WriteAnnotationsWorker::tr("CSV separator"), 
            WriteAnnotationsWorker::tr("String which separates values in CSV files"));
        attrs << new Attribute(separatorDesc, BaseTypes::STRING_TYPE(), false, QVariant(SEPARATOR_DEFAULT_VALUE));

        Descriptor writeNamesDesc(WRITE_NAMES, WriteAnnotationsWorker::tr("Write sequence names"), 
            WriteAnnotationsWorker::tr("Add names of sequences into CSV file"));
        attrs << new Attribute(writeNamesDesc, BaseTypes::BOOL_TYPE(), false, false);

        docFormatAttr->addRelation(new FileExtensionRelation(urlAttr->getId()));
        nameAttr->addRelation(new VisibilityRelation(BaseAttributes::DOCUMENT_FORMAT_ATTRIBUTE().getId(),
            QVariantList()
            << CSV_FORMAT_ID
            << BaseDocumentFormats::PLAIN_EMBL
            << BaseDocumentFormats::PLAIN_GENBANK
            << BaseDocumentFormats::GFF
            << BaseDocumentFormats::PLAIN_SWISS_PROT));
    }

    Descriptor protoDesc(WriteAnnotationsWorkerFactory::ACTOR_ID, 
        WriteAnnotationsWorker::tr("Write Annotations"), 
        WriteAnnotationsWorker::tr("Writes all supplied annotations to file(s) in selected formatId."));
    ActorPrototype * proto = new IntegralBusActorPrototype(protoDesc, portDescs, attrs);

    // proto delegates
    QMap<QString, PropertyDelegate*> delegates;
    {
        QVariantMap m;
        foreach( const DocumentFormatId & fid, supportedFormats ) {
            m[fid] = fid;
        }
        delegates[BaseAttributes::DOCUMENT_FORMAT_ATTRIBUTE().getId()] = new ComboBoxDelegate(m);
        delegates[BaseAttributes::URL_OUT_ATTRIBUTE().getId()] =
            new URLDelegate(DialogUtils::prepareDocumentsFileFilter(format, true), QString(), false, false, true, NULL, format);
        delegates[BaseAttributes::FILE_MODE_ATTRIBUTE().getId()] = new FileModeDelegate(attrs.size() > 2);
    }
    proto->setEditor(new DelegateEditor(delegates));
    proto->setPrompter(new WriteAnnotationsPrompter());
    proto->setValidator(new ScreenedParamValidator(BaseAttributes::URL_OUT_ATTRIBUTE().getId(), 
        BasePorts::IN_ANNOTATIONS_PORT_ID(), BaseSlots::URL_SLOT().getId()));
    proto->setPortValidator(BasePorts::IN_ANNOTATIONS_PORT_ID(), new ScreenedSlotValidator(BaseSlots::URL_SLOT().getId()));
    WorkflowEnv::getProtoRegistry()->registerProto(BaseActorCategories::CATEGORY_DATASINK(), proto);
    WorkflowEnv::getDomainRegistry()->getById(LocalDomainFactory::ID)->registerEntry(new WriteAnnotationsWorkerFactory());
}

Worker * WriteAnnotationsWorkerFactory::createWorker(Actor* a) {
    return new WriteAnnotationsWorker(a);
}

/***************************
 * WriteAnnotationsPrompter
 ***************************/
QString WriteAnnotationsPrompter::composeRichDoc() {
    QString unsetStr = "<font color='red'>"+tr("unset")+"</font>";
    IntegralBusPort * input = qobject_cast<IntegralBusPort*>(target->getPort(BasePorts::IN_ANNOTATIONS_PORT_ID()));
    QString annName = getProducers(BasePorts::IN_ANNOTATIONS_PORT_ID(), BaseSlots::ANNOTATION_TABLE_SLOT().getId());
    annName = annName.isEmpty() ? unsetStr : annName;
    
    QString url = getScreenedURL(input, BaseAttributes::URL_OUT_ATTRIBUTE().getId(), BaseSlots::URL_SLOT().getId());
    QString format = getParameter(BaseAttributes::DOCUMENT_FORMAT_ATTRIBUTE().getId()).value<QString>();
    
    return tr("Save all annotations from <u>%1</u> to %2 in %3 format")
        .arg(annName)
        .arg(getHyperlink(BaseAttributes::URL_OUT_ATTRIBUTE().getId(), url))
        .arg(getHyperlink(BaseAttributes::DOCUMENT_FORMAT_ATTRIBUTE().getId(), format));
}

} // LocalWorkflow
} // U2
