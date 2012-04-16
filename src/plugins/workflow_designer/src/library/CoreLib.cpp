/**
 * UGENE - Integrated Bioinformatics Tools.
 * Copyright (C) 2008-2012 UniPro <ugene@unipro.ru>
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

#include <QtXml/qdom.h>

#include "CoreLib.h"
#include "BaseDocWorker.h"
#include "DocWorkers.h"

#include "DocActors.h"
#include "GenericReadActor.h"
#include "FindWorker.h"
#include "SequenceSplitWorker.h"
#include "ScriptWorker.h"
#include "Text2SequenceWorker.h"
#include "ImportAnnotationsWorker.h"
#include "SequencesToMSAWorker.h"
#include "FilterAnnotationsWorker.h"
#include "CDSearchWorker.h"
#include "StatisticWorkers.h"
#include "ReverseComplementWorker.h"
#include "MSA2SequenceWorker.h"
#include "ExternalProcessWorker.h"
#include "library/GetFileListWorker.h"
#include "library/GroupWorker.h"
#include "library/PassFilterWorker.h"
#include "library/MarkSequenceWorker.h"
#include "library/MultiplexerWorker.h"
#include "library/AminoTranslationWorker.h"

#include "RemoteDBFetcherWorker.h"

#include <U2Core/BaseDocumentFormats.h>
#include <U2Core/DocumentModel.h>
#include <U2Core/AppContext.h>
#include <U2Core/Settings.h>
#include <U2Core/Log.h>
#include <U2Core/GObjectTypes.h>
#include <U2Lang/IncludedProtoFactory.h>

#include <U2Lang/IntegralBusModel.h>
#include <U2Lang/WorkflowManager.h>
#include <U2Lang/ActorPrototypeRegistry.h>
#include <U2Lang/WorkflowEnv.h>

#include <U2Lang/WorkflowSettings.h>
#include <U2Lang/BaseTypes.h>
#include <U2Lang/LocalDomain.h>
#include <U2Lang/BaseSlots.h>
#include <U2Lang/BasePorts.h>
#include <U2Lang/BaseAttributes.h>
#include <U2Lang/BaseActorCategories.h>
#include <U2Designer/DelegateEditors.h>
#include <U2Lang/CoreLibConstants.h>
#include <U2Lang/ScriptWorkerSerializer.h>

#include <U2Core/global.h>
#include <U2Gui/DialogUtils.h>
#include <U2Gui/GUIUtils.h>
#include <U2Lang/HRSchemaSerializer.h>


/* TRANSLATOR U2::Workflow::CoreLib */

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
        IntegralBusActorPrototype* proto = new WriteDocActorProto(BaseDocumentFormats::FASTA, acd, p, pd.getId(), a);
        proto->setPrompter(new WriteFastaPrompter("FASTA"));
        r->registerProto(BaseActorCategories::CATEGORY_DATASINK(), proto);
    }
   
    // READ PLAIN TEXT actor proto
    {
        QMap<Descriptor, DataTypePtr> m;
        m[BaseSlots::URL_SLOT()] = BaseTypes::STRING_TYPE();
        m[BaseSlots::TEXT_SLOT()] = BaseTypes::STRING_TYPE();
        DataTypePtr dtl(new MapDataType(Descriptor(CoreLibConstants::TEXT_TYPESET_ID), m));
        dr->registerEntry(dtl);
        
        QList<PortDescriptor*> p; QList<Attribute*> a;
        a << new Attribute(BaseAttributes::READ_BY_LINES_ATTRIBUTE(), BaseTypes::BOOL_TYPE(), false, false);
        
        Descriptor acd(CoreLibConstants::READ_TEXT_PROTO_ID, tr("Read Plain Text"), tr("Reads text from local or remote files."));
        p << new PortDescriptor(Descriptor(BasePorts::OUT_TEXT_PORT_ID(), tr("Plain text"), ""), dtl, false, true);
        IntegralBusActorPrototype* proto = new ReadDocActorProto(BaseDocumentFormats::PLAIN_TEXT, acd, p, a);
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
        m[BaseSlots::TEXT_SLOT()] = new ListDataType("string-list", BaseTypes::STRING_TYPE());
        DataTypePtr dtl(new MapDataType(Descriptor("in.text"), m));

        QList<PortDescriptor*> p; QList<Attribute*> a;
        Descriptor acd(CoreLibConstants::WRITE_TEXT_PROTO_ID, tr("Write plain text"), tr("Write strings to a file"));
        Descriptor pd(BasePorts::IN_TEXT_PORT_ID(), tr("Plain text"), tr("Plain text"));
        p << new PortDescriptor(pd, dtl, true);
        a << new Attribute(BaseAttributes::ACCUMULATE_OBJS_ATTRIBUTE(), BaseTypes::BOOL_TYPE(), false, true);
        IntegralBusActorPrototype* proto = new WriteDocActorProto(BaseDocumentFormats::PLAIN_TEXT, acd, p, pd.getId(), a);
        proto->setPrompter(new WriteDocPrompter(tr("Save text from <u>%1</u> to <u>%2</u>."), BaseSlots::TEXT_SLOT().getId()));
        r->registerProto(BaseActorCategories::CATEGORY_DATASINK(), proto);
    }
    // GENERIC WRITE MSA actor proto
    {
        DocumentFormatConstraints constr;
        constr.supportedObjectTypes.insert( GObjectTypes::MULTIPLE_ALIGNMENT );
        constr.addFlagToSupport(DocumentFormatFlag_SupportWriting);
        QList<DocumentFormatId> supportedFormats = AppContext::getDocumentFormatRegistry()->selectFormats( constr );

        if( !supportedFormats.isEmpty() ) {
            QList<PortDescriptor*> p; QList<Attribute*> a;
            Descriptor acd(CoreLibConstants::WRITE_MSA_PROTO_ID, tr("Write alignment"), tr("Writes all supplied alignments to file(s) in selected format"));
            Descriptor pd(BasePorts::IN_MSA_PORT_ID(), tr("Multiple sequence alignment"), tr("Multiple sequence alignment"));
            p << new PortDescriptor(pd, writeMAType, true);
            Attribute *docFormatAttr = new Attribute(BaseAttributes::DOCUMENT_FORMAT_ATTRIBUTE(), BaseTypes::STRING_TYPE(), false, 
                supportedFormats.contains( BaseDocumentFormats::CLUSTAL_ALN ) ? BaseDocumentFormats::CLUSTAL_ALN : supportedFormats.first() );
            a << docFormatAttr;
            WriteDocActorProto *childProto = new WriteDocActorProto( acd, GObjectTypes::MULTIPLE_ALIGNMENT, p, pd.getId(), a );
            IntegralBusActorPrototype * proto = childProto;
            docFormatAttr->addRelation(new FileExtensionRelation(childProto->getUrlAttr()->getId(), docFormatAttr->getAttributePureValue().toString()));
            
            QVariantMap m;
            foreach( const DocumentFormatId & fid, supportedFormats ) {
                m[fid] = fid;
            }
            ComboBoxDelegate *comboDelegate = new ComboBoxDelegate(m);
            connect(comboDelegate, SIGNAL(si_valueChanged(const QString &)), childProto->getUrlDelegate(), SLOT(sl_formatChanged(const QString &)));
            proto->getEditor()->addDelegate(comboDelegate, BaseAttributes::DOCUMENT_FORMAT_ATTRIBUTE().getId());
            proto->setPrompter(new WriteDocPrompter(tr("Save all MSAs from <u>%1</u> to <u>%2</u>."), BaseSlots::MULTIPLE_ALIGNMENT_SLOT().getId()));
            r->registerProto(BaseActorCategories::CATEGORY_DATASINK(), proto);
        }
    }
    
    // GENERIC WRITE SEQ actor proto
    {
        DocumentFormatConstraints constr;
        constr.supportedObjectTypes.insert( GObjectTypes::SEQUENCE );
        constr.addFlagToSupport(DocumentFormatFlag_SupportWriting);
        QList<DocumentFormatId> supportedFormats = AppContext::getDocumentFormatRegistry()->selectFormats( constr );
        
        if( !supportedFormats.isEmpty() ) {
            QMap<Descriptor, DataTypePtr> typeMap;
            typeMap[writeUrlD] = BaseTypes::STRING_TYPE();
            typeMap[BaseSlots::DNA_SEQUENCE_SLOT()] = BaseTypes::DNA_SEQUENCE_TYPE();
            typeMap[BaseSlots::ANNOTATION_TABLE_SLOT()] = BaseTypes::ANNOTATION_TABLE_LIST_TYPE();
            DataTypePtr typeSet( new MapDataType(Descriptor(SEQ_TYPESET_ID), typeMap));
            
            QList<PortDescriptor*> p; QList<Attribute*> a;
            Descriptor acd(CoreLibConstants::WRITE_SEQ_PROTO_ID, tr("Write sequence"), tr("Writes all supplied sequences to file(s) in selected format."));
            Descriptor pd(BasePorts::IN_SEQ_PORT_ID(), tr("Sequence"), tr("Sequence"));
            p << new PortDescriptor(pd, typeSet, true);
            a << new Attribute(BaseAttributes::ACCUMULATE_OBJS_ATTRIBUTE(), BaseTypes::BOOL_TYPE(), false, true);
            Attribute *docFormatAttr = new Attribute(BaseAttributes::DOCUMENT_FORMAT_ATTRIBUTE(), BaseTypes::STRING_TYPE(), false, 
                supportedFormats.contains( BaseDocumentFormats::FASTA ) ? BaseDocumentFormats::FASTA : supportedFormats.first() );
            a << docFormatAttr;
            WriteDocActorProto *childProto = new WriteDocActorProto( acd, GObjectTypes::SEQUENCE, p, pd.getId(), a );
            IntegralBusActorPrototype * proto = childProto;
            docFormatAttr->addRelation(new FileExtensionRelation(childProto->getUrlAttr()->getId(), docFormatAttr->getAttributePureValue().toString()));
            
            QVariantMap m;
            foreach( const DocumentFormatId & fid, supportedFormats ) {
                m[fid] = fid;
            }
            ComboBoxDelegate *comboDelegate = new ComboBoxDelegate(m);
            connect(comboDelegate, SIGNAL(si_valueChanged(const QString &)), childProto->getUrlDelegate(), SLOT(sl_formatChanged(const QString &)));
            proto->getEditor()->addDelegate(comboDelegate, BaseAttributes::DOCUMENT_FORMAT_ATTRIBUTE().getId());
            proto->setPrompter(new WriteDocPrompter(tr("Save all sequences from <u>%1</u> to <u>%2</u>."), BaseSlots::DNA_SEQUENCE_SLOT().getId()));
            r->registerProto(BaseActorCategories::CATEGORY_DATASINK(), proto);
        }
    }
    DataWorkerFactory::init();
    FindWorkerFactory::init();
    RemoteDBFetcherFactory::init();
    SequenceSplitWorkerFactory::init();
    Text2SequenceWorkerFactory::init();
    Alignment2SequenceWorkerFactory::init();
    ImportAnnotationsWorkerFactory::init();
    FilterAnnotationsWorkerFactory::init();
    SequencesToMSAWorkerFactory::init();
    CDSearchWorkerFactory::init();
    DNAStatWorkerFactory::init();
    RCWorkerFactory::init();
    PassFilterWorkerFactory::init();
    MarkSequenceWorkerFactory::init();
    MultiplexerWorkerFactory::init();
    GetFileListWorkerFactory::init();
    FetchSequenceByIdFromAnnotationFactory::init();
    GroupWorkerFactory::init();
    AminoTranslationWorkerFactory::init();

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
