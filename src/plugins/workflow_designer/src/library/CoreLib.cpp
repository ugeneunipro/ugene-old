/**
 * UGENE - Integrated Bioinformatics Tools.
 * Copyright (C) 2008-2015 UniPro <ugene@unipro.ru>
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

#include <qdom.h>

#include <U2Core/AppContext.h>
#include <U2Core/BaseDocumentFormats.h>
#include <U2Core/DocumentModel.h>
#include <U2Core/GObjectTypes.h>
#include <U2Core/Log.h>
#include <U2Core/Settings.h>
#include <U2Core/global.h>

#include <U2Designer/DelegateEditors.h>

#include <U2Gui/DialogUtils.h>
#include <U2Gui/GUIUtils.h>

#include <U2Lang/ActorPrototypeRegistry.h>
#include <U2Lang/BaseActorCategories.h>
#include <U2Lang/BaseAttributes.h>
#include <U2Lang/BasePorts.h>
#include <U2Lang/BaseSlots.h>
#include <U2Lang/BaseTypes.h>
#include <U2Lang/CoreLibConstants.h>
#include <U2Lang/HRSchemaSerializer.h>
#include <U2Lang/IncludedProtoFactory.h>
#include <U2Lang/IntegralBusModel.h>
#include <U2Lang/LocalDomain.h>
#include <U2Lang/ScriptWorkerSerializer.h>
#include <U2Lang/WorkflowEnv.h>
#include <U2Lang/WorkflowManager.h>
#include <U2Lang/WorkflowSettings.h>

#include "BaseDocWriter.h"
#include "CDSearchWorker.h"
#include "CoreLib.h"
#include "DocActors.h"
#include "DocWorkers.h"
#include "ExternalProcessWorker.h"
#include "FilterAnnotationsByQualifierWorker.h"
#include "FilterAnnotationsWorker.h"
#include "FindWorker.h"
#include "GenericReadActor.h"
#include "ImportAnnotationsWorker.h"
#include "MSA2SequenceWorker.h"
#include "ReverseComplementWorker.h"
#include "ScriptWorker.h"
#include "SequenceSplitWorker.h"
#include "SequencesToMSAWorker.h"
#include "StatisticWorkers.h"
#include "Text2SequenceWorker.h"
#include "library/AlignToReferenceWorker.h"
#include "library/AminoTranslationWorker.h"
#include "library/AssemblyToSequenceWorker.h"
#include "library/ConvertFilesFormatWorker.h"
#include "library/ExtractAssemblyCoverageWorker.h"
#include "library/ExtractConsensusWorker.h"
#include "library/ExtractMSAConsensusWorker.h"
#include "library/FASTQWorkersLibrary.h"
#include "library/FilterBamWorker.h"
#include "library/GetFileListWorker.h"
#include "library/GroupWorker.h"
#include "library/MarkSequenceWorker.h"
#include "library/MergeBamWorker.h"
#include "library/MultiplexerWorker.h"
#include "library/PassFilterWorker.h"
#include "library/ReadAnnotationsWorker.h"
#include "library/ReadAssemblyWorker.h"
#include "library/ReadVariationWorker.h"
#include "library/RemoteDBFetcherWorker.h"
#include "library/RenameChromosomeInVariationWorker.h"
#include "library/RmdupBamWorker.h"
#include "library/SortBamWorker.h"
#include "library/WriteAnnotationsWorker.h"
#include "library/WriteAssemblyWorkers.h"
#include "library/WriteVariationWorker.h"
#include "util/WriteSequenceValidator.h"

namespace U2 {
using namespace LocalWorkflow;
namespace Workflow {

static const QString FASTQ_TYPESET_ID("fastq.content");
static const QString FASTA_TYPESET_ID("fasta.content");
static const QString SEQ_TYPESET_ID("seq.content");
static const QString GENBANK_TYPESET_ID("genbank.content");
static const QString MA_TYPESET_ID("ma.content");

#define WORKFLOW_DOC "GB2WORKFLOW"
#define ACTOR_ELEMENT "Actor"
#define INPUT_PORT_ELEMENT "Input-port"
#define OUTPUT_PORT_ELEMENT "Output-port"
#define ATTRIBUTE_ELEMENT "Attributes"
#define IN_SLOT_ELEMENT "In-Slots"
#define OUT_SLOT_ELEMENT "Out-Slots"
#define SLOT_ID "Slot"
#define ATTR_ELEMENT "Attribute"
#define NAME_ID "Name"
#define TYPE_ID "Type"
#define NAME_ELEMENT "Element-name"
#define DESCR_ELEMENT "Element-description"
#define DESCR_ID "Description"

void CoreLib::init() {

    Descriptor writeUrlD(BaseSlots::URL_SLOT().getId(), tr("Location"), tr("Location for writing data"));
    DataTypeRegistry* dr = WorkflowEnv::getDataTypeRegistry();
    assert(dr);

    DataTypePtr writeMAType;
    {
        QMap<Descriptor, DataTypePtr> m;
        m[writeUrlD] = BaseTypes::STRING_TYPE();
        m[BaseSlots::MULTIPLE_ALIGNMENT_SLOT()] = BaseTypes::MULTIPLE_ALIGNMENT_TYPE();
        writeMAType = new MapDataType(Descriptor(MA_TYPESET_ID), m);
        //dr->registerEntry(writeMAType);
    }

    ActorPrototypeRegistry* r = WorkflowEnv::getProtoRegistry();
    assert(r);
    r->registerProto(BaseActorCategories::CATEGORY_DATASRC(), new GenericMAActorProto());
    r->registerProto(BaseActorCategories::CATEGORY_DATASRC(), new GenericSeqActorProto());

    // WRITE FASTA actor proto
    {
        QMap<Descriptor, DataTypePtr> m;
        m[writeUrlD] = BaseTypes::STRING_TYPE();
        m[BaseSlots::DNA_SEQUENCE_SLOT()] = BaseTypes::DNA_SEQUENCE_TYPE();
        m[BaseSlots::FASTA_HEADER_SLOT()] = BaseTypes::STRING_TYPE();
        DataTypePtr fastaTypeSet(new MapDataType(Descriptor(FASTA_TYPESET_ID), m));

        QList<PortDescriptor*> p; QList<Attribute*> a;
        Descriptor acd(CoreLibConstants::WRITE_FASTA_PROTO_ID, tr("Write FASTA"), tr("Writes all supplied sequences to file(s) in FASTA format."));
        Descriptor pd(BasePorts::IN_SEQ_PORT_ID(), tr("Sequence"), tr("A sequence along with FASTA header line."));
        p << new PortDescriptor(pd, fastaTypeSet, true);
        a << new Attribute(BaseAttributes::ACCUMULATE_OBJS_ATTRIBUTE(), BaseTypes::BOOL_TYPE(), false, true);
        IntegralBusActorPrototype* proto = new WriteDocActorProto(BaseDocumentFormats::FASTA, acd, p, pd.getId(), a, false, false);
        proto->setPrompter(new WriteFastaPrompter("FASTA"));
        r->registerProto(BaseActorCategories::CATEGORY_DATASINK(), proto);
    }

    // READ PLAIN TEXT actor proto
    {
        QMap<Descriptor, DataTypePtr> m;
        m[BaseSlots::URL_SLOT()] = BaseTypes::STRING_TYPE();
        m[BaseSlots::TEXT_SLOT()] = BaseTypes::STRING_TYPE();
        m[BaseSlots::DATASET_SLOT()] = BaseTypes::STRING_TYPE();
        DataTypePtr dtl(new MapDataType(Descriptor(CoreLibConstants::TEXT_TYPESET_ID), m));
        dr->registerEntry(dtl);

        QList<PortDescriptor*> p; QList<Attribute*> a;
        a << new Attribute(BaseAttributes::READ_BY_LINES_ATTRIBUTE(), BaseTypes::BOOL_TYPE(), false, false);

        Descriptor acd(CoreLibConstants::READ_TEXT_PROTO_ID, tr("Read Plain Text"), tr("Reads text from local or remote files."));
        p << new PortDescriptor(Descriptor(BasePorts::OUT_TEXT_PORT_ID(), tr("Plain text"), ""), dtl, false, true);
        ReadDocActorProto* proto = new ReadDocActorProto(BaseDocumentFormats::PLAIN_TEXT, acd, p, a);
        proto->setCompatibleDbObjectTypes(QSet<GObjectType>() << GObjectTypes::TEXT);
        proto->setPrompter(new ReadDocPrompter(tr("Reads text from <u>%1</u>.")));

        if(AppContext::isGUIMode()) {
            proto->setIcon( GUIUtils::createRoundIcon(QColor(85,85,255), 22));
        }

        r->registerProto(BaseActorCategories::CATEGORY_DATASRC(), proto);
    }
    // WRITE PLAIN TEXT actor proto
    {
        QMap<Descriptor, DataTypePtr> m;
        m[writeUrlD] = BaseTypes::STRING_TYPE();
        m[BaseSlots::TEXT_SLOT()] = BaseTypes::STRING_LIST_TYPE();
        DataTypePtr dtl(new MapDataType(Descriptor("in.text"), m));

        QList<PortDescriptor*> p; QList<Attribute*> a;
        Descriptor acd(CoreLibConstants::WRITE_TEXT_PROTO_ID, tr("Write Plain Text"), tr("Write strings to a file."));
        Descriptor pd(BasePorts::IN_TEXT_PORT_ID(), tr("Plain text"), tr("Plain text"));
        p << new PortDescriptor(pd, dtl, true);
        Attribute *accumulateObjsAttr = new Attribute(BaseAttributes::ACCUMULATE_OBJS_ATTRIBUTE(), BaseTypes::BOOL_TYPE(), false, true);
        accumulateObjsAttr->addRelation(new VisibilityRelation(BaseAttributes::DATA_STORAGE_ATTRIBUTE().getId(), BaseAttributes::LOCAL_FS_DATA_STORAGE()));
        a << accumulateObjsAttr;
        IntegralBusActorPrototype* proto = new WriteDocActorProto(BaseDocumentFormats::PLAIN_TEXT, acd, p, pd.getId(), a, true, false);
        proto->setPrompter(new WriteDocPrompter(tr("Save text from <u>%1</u> to <u>%2</u>."), BaseSlots::TEXT_SLOT().getId()));
        r->registerProto(BaseActorCategories::CATEGORY_DATASINK(), proto);
    }
    // GENERIC WRITE MSA actor proto
    {
        DocumentFormatConstraints constr;
        constr.supportedObjectTypes.insert( GObjectTypes::MULTIPLE_ALIGNMENT );
        constr.addFlagToSupport(DocumentFormatFlag_SupportWriting);
        constr.addFlagToExclude(DocumentFormatFlag_CannotBeCreated);
        QList<DocumentFormatId> supportedFormats = AppContext::getDocumentFormatRegistry()->selectFormats( constr );

        if( !supportedFormats.isEmpty() ) {
            DocumentFormatId format = supportedFormats.contains( BaseDocumentFormats::CLUSTAL_ALN ) ? BaseDocumentFormats::CLUSTAL_ALN : supportedFormats.first();
            QList<PortDescriptor*> p; QList<Attribute*> a;
            Descriptor acd(CoreLibConstants::WRITE_MSA_PROTO_ID, tr("Write Alignment"), tr("Writes all supplied alignments to file(s) in selected format."));
            Descriptor pd(BasePorts::IN_MSA_PORT_ID(), tr("Multiple sequence alignment"), tr("Multiple sequence alignment"));
            p << new PortDescriptor(pd, writeMAType, true);
            Attribute *docFormatAttr = new Attribute(BaseAttributes::DOCUMENT_FORMAT_ATTRIBUTE(), BaseTypes::STRING_TYPE(), false, format);
            a << docFormatAttr;
            WriteDocActorProto *proto = new WriteDocActorProto(format, acd, p, pd.getId(), a, true, false);
            docFormatAttr->addRelation(new FileExtensionRelation(proto->getUrlAttr()->getId()));
            docFormatAttr->addRelation(new VisibilityRelation(BaseAttributes::DATA_STORAGE_ATTRIBUTE().getId(), BaseAttributes::LOCAL_FS_DATA_STORAGE()));

            QVariantMap m;
            foreach( const DocumentFormatId & fid, supportedFormats ) {
                m[fid] = fid;
            }
            proto->getEditor()->addDelegate(new ComboBoxDelegate(m), BaseAttributes::DOCUMENT_FORMAT_ATTRIBUTE().getId());
            proto->setPrompter(new WriteDocPrompter(tr("Save all MSAs from <u>%1</u> to <u>%2</u>."), BaseSlots::MULTIPLE_ALIGNMENT_SLOT().getId()));
            r->registerProto(BaseActorCategories::CATEGORY_DATASINK(), proto);
        }
    }

    // GENERIC WRITE SEQ actor proto
    {
        DocumentFormatConstraints constr;
        constr.supportedObjectTypes.insert( GObjectTypes::SEQUENCE );
        constr.addFlagToSupport(DocumentFormatFlag_SupportWriting);
        constr.addFlagToExclude(DocumentFormatFlag_CannotBeCreated);
        QList<DocumentFormatId> supportedFormats = AppContext::getDocumentFormatRegistry()->selectFormats( constr );

        if( !supportedFormats.isEmpty() ) {
            DocumentFormatId format = supportedFormats.contains( BaseDocumentFormats::FASTA ) ? BaseDocumentFormats::FASTA : supportedFormats.first();
            QMap<Descriptor, DataTypePtr> typeMap;
            typeMap[writeUrlD] = BaseTypes::STRING_TYPE();
            typeMap[BaseSlots::DNA_SEQUENCE_SLOT()] = BaseTypes::DNA_SEQUENCE_TYPE();
            typeMap[BaseSlots::ANNOTATION_TABLE_SLOT()] = BaseTypes::ANNOTATION_TABLE_LIST_TYPE();
            DataTypePtr typeSet( new MapDataType(Descriptor(SEQ_TYPESET_ID), typeMap));

            QList<PortDescriptor*> p; QList<Attribute*> a;
            Descriptor acd(CoreLibConstants::WRITE_SEQ_PROTO_ID, tr("Write Sequence"), tr("Writes all supplied sequences to file(s) in selected format."));
            Descriptor pd(BasePorts::IN_SEQ_PORT_ID(), tr("Sequence"), tr("Sequence"));
            p << new PortDescriptor(pd, typeSet, true);
            Attribute *accumulateAttr = new Attribute(BaseAttributes::ACCUMULATE_OBJS_ATTRIBUTE(), BaseTypes::BOOL_TYPE(), false, true);
            a << accumulateAttr;
            accumulateAttr->addRelation(new VisibilityRelation(BaseAttributes::DATA_STORAGE_ATTRIBUTE().getId(), BaseAttributes::LOCAL_FS_DATA_STORAGE()));
            Attribute *docFormatAttr = new Attribute(BaseAttributes::DOCUMENT_FORMAT_ATTRIBUTE(), BaseTypes::STRING_TYPE(), true, format);
            a << docFormatAttr;
            Attribute* splitAttr = new Attribute(BaseAttributes::SPLIT_SEQ_ATTRIBUTE(), BaseTypes::NUM_TYPE(), false, 1);
            splitAttr->addRelation(new VisibilityRelation(BaseAttributes::DOCUMENT_FORMAT_ATTRIBUTE().getId(), BaseDocumentFormats::FASTA));
            splitAttr->addRelation(new VisibilityRelation(BaseAttributes::DATA_STORAGE_ATTRIBUTE().getId(), BaseAttributes::LOCAL_FS_DATA_STORAGE()));
            a << splitAttr;
            WriteDocActorProto *proto = new WriteDocActorProto(format, acd, p, pd.getId(), a, true, false, false);
            proto->setPortValidator(pd.getId(), new WriteSequencePortValidator());
            proto->setValidator(new WriteSequenceValidator(BaseAttributes::URL_OUT_ATTRIBUTE().getId(), BasePorts::IN_SEQ_PORT_ID(), BaseSlots::URL_SLOT().getId()));
            docFormatAttr->addRelation(new FileExtensionRelation(proto->getUrlAttr()->getId()));
            docFormatAttr->addRelation(new VisibilityRelation(BaseAttributes::DATA_STORAGE_ATTRIBUTE().getId(), BaseAttributes::LOCAL_FS_DATA_STORAGE()));

            QVariantMap m;
            foreach( const DocumentFormatId & fid, supportedFormats ) {
                m[fid] = fid;
            }

            ComboBoxDelegate *comboDelegate = new ComboBoxDelegate(m);

            QVariantMap lenMap; lenMap["minimum"] = QVariant(1);
            lenMap["maximum"] = QVariant(100);
            SpinBoxDelegate* spinDelegate  = new SpinBoxDelegate(lenMap);

            proto->getEditor()->addDelegate(comboDelegate, BaseAttributes::DOCUMENT_FORMAT_ATTRIBUTE().getId());
            proto->getEditor()->addDelegate(spinDelegate, BaseAttributes::SPLIT_SEQ_ATTRIBUTE().getId());
            proto->setPrompter(new WriteDocPrompter(tr("Save all sequences from <u>%1</u> to <u>%2</u>."), BaseSlots::DNA_SEQUENCE_SLOT().getId()));
            r->registerProto(BaseActorCategories::CATEGORY_DATASINK(), proto);
        }
    }
    AlignToReferenceWorkerFactory::init();
    Alignment2SequenceWorkerFactory::init();
    AminoTranslationWorkerFactory::init();
    AssemblyToSequencesWorkerFactory::init();
    CASAVAFilterWorkerFactory::init();
    CDSearchWorkerFactory::init();
    ConvertFilesFormatWorkerFactory::init();
    DNAStatWorkerFactory::init();
    DataWorkerFactory::init();
    ExtractAssemblyCoverageWorkerFactory::init();
    ExtractConsensusWorkerFactory::init();
    ExtractMSAConsensusSequenceWorkerFactory::init();
    ExtractMSAConsensusStringWorkerFactory::init();
    FetchSequenceByIdFromAnnotationFactory::init();
    FilterAnnotationsByQualifierWorkerFactory::init();
    FilterAnnotationsWorkerFactory::init();
    FilterBamWorkerFactory::init();
    FindWorkerFactory::init();
    GetFileListWorkerFactory::init();
    GroupWorkerFactory::init();
    ImportAnnotationsWorkerFactory::init();
    MarkSequenceWorkerFactory::init();
    MergeBamWorkerFactory::init();
    MergeFastqWorkerFactory::init();
    MultiplexerWorkerFactory::init();
    PassFilterWorkerFactory::init();
    QualityTrimWorkerFactory::init();
    RCWorkerFactory::init();
    ReadAnnotationsWorkerFactory::init();
    ReadAssemblyWorkerFactory::init();
    ReadVariationWorkerFactory::init();
    RenameChomosomeInVariationWorkerFactory::init();
    RemoteDBFetcherFactory::init();
    RmdupBamWorkerFactory::init();
    SequenceSplitWorkerFactory::init();
    SequencesToMSAWorkerFactory::init();
    SortBamWorkerFactory::init();
    Text2SequenceWorkerFactory::init();
    WriteAnnotationsWorkerFactory::init();
    WriteAssemblyWorkerFactory::init();
    WriteVariationWorkerFactory::init();

    initUsersWorkers();
    initExternalToolsWorkers();
}

void CoreLib::initUsersWorkers() {
    QString path = WorkflowSettings::getUserDirectory();

    QDir dir(path);
    if(!dir.exists()) {
        //log.info(tr("There isn't directory with users workflow elements"));
        return;
    }
    dir.setNameFilters(QStringList() << "*.usa"); //think about file extension // Answer: Ok :)
    QFileInfoList fileList = dir.entryInfoList();

    foreach(const QFileInfo& fileInfo, fileList) {
        QString url = fileInfo.filePath();
        QFile file(url);
        file.open(QIODevice::ReadOnly);
        QByteArray content = file.readAll();
        file.close();

        QString error;
        ActorPrototype *proto = ScriptWorkerSerializer::string2actor(content, QString(), error, url);
        if (NULL == proto) {
            coreLog.error(error);
            return;
        }

        WorkflowEnv::getProtoRegistry()->registerProto(BaseActorCategories::CATEGORY_SCRIPT(), proto);

        DomainFactory* localDomain = WorkflowEnv::getDomainRegistry()->getById( LocalDomainFactory::ID );
        localDomain->registerEntry( new ScriptWorkerFactory(proto->getId()) );
    }
}

void CoreLib::initExternalToolsWorkers() {
    QString path = WorkflowSettings::getExternalToolDirectory();
    QDir dir(path);
    if(!dir.exists()) {
        return;
    }
    dir.setNameFilters(QStringList() << "*.etc");
    QFileInfoList fileList = dir.entryInfoList();

    foreach(const QFileInfo& fileInfo, fileList) {
        QString url = fileInfo.filePath();
        QFile file(url);
        file.open(QIODevice::ReadOnly);
        QString data = file.readAll().data();

        ExternalProcessConfig *cfg = NULL;
        cfg = HRSchemaSerializer::string2Actor(data);

        if(cfg) {
            cfg->filePath = url;
            ExternalProcessWorkerFactory::init(cfg);
        }
        file.close();
    }
}

void CoreLib::initIncludedWorkers() {
    QString path = WorkflowSettings::getIncludedElementsDirectory();
    QDir dir(path);
    if(!dir.exists()) {
        return;
    }
    dir.setNameFilters(QStringList() << "*.uwl");
    QFileInfoList fileList = dir.entryInfoList();

    foreach(const QFileInfo& fileInfo, fileList) {
        // read file content
        QString url = fileInfo.filePath();
        QFile file(url);
        file.open(QIODevice::ReadOnly);
        QString data = file.readAll().data();
        file.close();

        // parse schema from data
        QList<QString> urlList;
        urlList << url;
        Schema *schema = new Schema();
        QMap<ActorId, ActorId> procMap;
        QString error = HRSchemaSerializer::string2Schema(data, schema, NULL, &procMap, urlList);

        // generate proto from schema
        ActorPrototype *proto = NULL;
        QString actorName;
        if (error.isEmpty()) {
            actorName = schema->getTypeName();
            proto = IncludedProtoFactory::getSchemaActorProto(schema, actorName, url);
        }

        if (NULL != proto) {
            // register the new proto
            if (IncludedProtoFactory::isRegistered(actorName)) {
                bool isEqualProtos = IncludedProtoFactory::isRegisteredTheSameProto(actorName, proto);
                if (!isEqualProtos) {
                    coreLog.error(tr("Another worker with this name is already registered: %1").arg(actorName));
                } else {
                    coreLog.trace(tr("The actor '%1' has been already registered").arg(actorName));
                }
            } else {
                WorkflowEnv::getProtoRegistry()->registerProto(BaseActorCategories::CATEGORY_INCLUDES(), proto);
                WorkflowEnv::getSchemaActorsRegistry()->registerSchema(schema->getTypeName(), schema);
            }
        }
    }
}


} // Workflow namespace
} // U2 namespace
