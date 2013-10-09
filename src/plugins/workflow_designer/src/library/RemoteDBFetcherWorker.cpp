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

#include <U2Core/AnnotationData.h>
#include <U2Core/AnnotationTableObject.h>
#include <U2Core/AppContext.h>
#include <U2Core/DNASequenceObject.h>
#include <U2Core/DocumentModel.h>
#include <U2Core/FailTask.h>
#include <U2Core/GObjectRelationRoles.h>
#include <U2Core/GObjectUtils.h>
#include <U2Core/LoadRemoteDocumentTask.h>

#include <U2Designer/DelegateEditors.h>

#include <U2Gui/GUIUtils.h>

#include <U2Lang/ActorModel.h>
#include <U2Lang/ActorPrototypeRegistry.h>
#include <U2Lang/CoreLibConstants.h>
#include <U2Lang/BaseActorCategories.h>
#include <U2Lang/BasePorts.h>
#include <U2Lang/BaseSlots.h>
#include <U2Lang/BaseTypes.h>
#include <U2Lang/WorkflowEnv.h>

#include "RemoteDBFetcherWorker.h"

namespace U2 {
namespace LocalWorkflow {

#define ICOLOR QColor(85,85,255) // FIXME

const QString RemoteDBFetcherFactory::ACTOR_ID("fetch-sequence");
static const QString TYPE("remote.seq");
static const QString DBID_ID("database");
static const QString SEQID_ID("resource-id");
static const QString PATH_ID("save-dir");
static const QString DEFAULT_PATH("default");

/* class RemoteDBFetcherPrompter : public PrompterBase<RemoteDBFetcherPrompter> */

QString RemoteDBFetcherPrompter::composeRichDoc()
{
    QString unsetStr = "<font color='red'>"+tr("unset")+"</font>";
    QStringList seqids = getParameter(SEQID_ID).value<QString>().split(";", QString::SkipEmptyParts);
    QString seq = seqids.size() > 1 ?
        RemoteDBFetcherWorker::tr("sequences identified with") :
        RemoteDBFetcherWorker::tr("sequence identified with");
    QString seqidsStr = seqids.isEmpty() ? unsetStr : QString("<u>%1</u>").arg(seqids.join(", "));
    
    QString dbid = getParameter(DBID_ID).value<QString>();
    dbid = RemoteDBFetcherFactory::cuteDbNames.key(dbid, dbid);
    
    QString saveDir = getParameter(PATH_ID).value<QString>();
    saveDir = getHyperlink(PATH_ID, saveDir);
    QString saveDirStr = RemoteDBFetcherWorker::tr("Save result to <u>%1</u> directory.").arg(saveDir);
    
    return RemoteDBFetcherWorker::tr("Reads %1 %2 from <u>%3</u> remote database. %4").
        arg(seq).
        arg(getHyperlink(SEQID_ID, seqidsStr)).
        arg(getHyperlink(DBID_ID, dbid)).
        arg(saveDirStr);
}

/* class RemoteDBFetcherWorker : public BaseWorker */

RemoteDBFetcherWorker::RemoteDBFetcherWorker(Actor *a) : BaseWorker(a), output(NULL)
{
}

void RemoteDBFetcherWorker::init()
{
    output = ports.value(BasePorts::OUT_SEQ_PORT_ID());
    
    assert(dbid.isEmpty());
    dbid = actor->getParameter(DBID_ID)->getAttributeValue<QString>(context);
    if(RemoteDBFetcherFactory::cuteDbNames.values().contains(dbid.toLower())) {
        dbid = RemoteDBFetcherFactory::cuteDbNames.key(dbid.toLower());
        assert(!dbid.isEmpty());
    }
    seqids = actor->getParameter(SEQID_ID)->getAttributeValue<QString>(context).split(";", QString::SkipEmptyParts);
    
    fullPathDir = actor->getParameter(PATH_ID)->getAttributeValue<QString>(context);
    if (fullPathDir == DEFAULT_PATH) {
        fullPathDir = LoadRemoteDocumentTask::getDefaultDownloadDirectory();
    } else {
        fullPathDir = context->absolutePath(fullPathDir);
    }
}

bool RemoteDBFetcherWorker::isReady()
{
    return !isDone();
}

Task* RemoteDBFetcherWorker::tick() {
    if(!QDir(fullPathDir).exists()) {
        if(!QDir().mkpath(fullPathDir)) {
            return new FailTask(tr("Cannot create directory '%1'").arg(fullPathDir));
        }
    }
    
    Task *ret = 0;
    // fetch and load next file
    QString seqId = "";
    while (seqId.isEmpty()) {
        if (seqids.isEmpty()) {
            return ret;
        }

        seqId = seqids.takeFirst().trimmed();
    }

    ret = new LoadRemoteDocumentTask(seqId, dbid, fullPathDir);
    connect(ret, SIGNAL(si_stateChanged()), SLOT(sl_taskFinished()));

    return ret;
}

bool RemoteDBFetcherWorker::isDone() {
    return (!dbid.isEmpty() && seqids.isEmpty());
}

void RemoteDBFetcherWorker::cleanup() {
}

void RemoteDBFetcherWorker::sl_taskFinished() {
    LoadRemoteDocumentTask *loadTask = qobject_cast<LoadRemoteDocumentTask*>(sender());
    assert(loadTask);

    if (loadTask->getState() != Task::State_Finished)
    {
        return;
    }

    if (loadTask->hasError())
    {
        loadTask->setError(tr("failed to load item '%1' from '%2' DB : %3").arg(loadTask->getAccNumber(), loadTask->getDBName(), loadTask->getError()));
        return;
    }

    Document *doc = loadTask->getDocument();
    assert(doc);

    foreach(GObject *gobj, doc->findGObjectByType(GObjectTypes::SEQUENCE))
    {
        U2SequenceObject *dnao = qobject_cast<U2SequenceObject*>(gobj);
        assert(dnao);

        QList<GObject*> allLoadedAnnotations = doc->findGObjectByType(GObjectTypes::ANNOTATION_TABLE);
        QList<GObject*> annotations = GObjectUtils::findObjectsRelatedToObjectByRole(gobj,
                                                                                     GObjectTypes::ANNOTATION_TABLE,
                                                                                     GObjectRelationRole::SEQUENCE,
                                                                                     allLoadedAnnotations,
                                                                                     UOF_LoadedOnly);

        QList<SharedAnnotationData> sads;
        if (!annotations.isEmpty())
        {
            AnnotationTableObject *ato = qobject_cast<AnnotationTableObject*>(annotations.first());
            foreach(Annotation *a, ato->getAnnotations())
            {
                sads << a->data();
            }
        }

        QVariantMap messageData;
        SharedDbiDataHandler seqId = context->getDataStorage()->putSequence(dnao->getWholeSequence());
        messageData[ BaseSlots::DNA_SEQUENCE_SLOT().getId() ] = qVariantFromValue<SharedDbiDataHandler>(seqId);
        messageData[ BaseSlots::ANNOTATION_TABLE_SLOT().getId() ] = qVariantFromValue(sads);

        DataTypePtr messageType = WorkflowEnv::getDataTypeRegistry()->getById(TYPE);

        output->put(Message(messageType, messageData));
    }

    if (seqids.isEmpty())
    {
        output->setEnded();
    }
}

/* class RemoteDBFetcherFactory : public DomainFactory */

const QMap<QString, QString> RemoteDBFetcherFactory::cuteDbNames = RemoteDBFetcherFactory::initCuteDbNames();

QMap<QString, QString> RemoteDBFetcherFactory::initCuteDbNames() {
    QMap<QString, QString> ret;
    ret[RemoteDBRegistry::ENSEMBL] = "ensembl";
    ret[RemoteDBRegistry::GENBANK_DNA] ="ncbi-dna";
    ret[RemoteDBRegistry::GENBANK_PROTEIN] ="ncbi-protein";
    ret[RemoteDBRegistry::PDB] = "pdb";
    ret[RemoteDBRegistry::SWISS_PROT] = "swiss-prot";
    ret[RemoteDBRegistry::UNIPROTKB_SWISS_PROT] = "uniprot-swiss-prot";
    ret[RemoteDBRegistry::UNIPROTKB_TREMBL] = "uniprot-trembl";
    return ret;
}

void RemoteDBFetcherFactory::init()
{
    QMap<Descriptor, DataTypePtr> outMap;
    outMap[ BaseSlots::DNA_SEQUENCE_SLOT() ] = BaseTypes::DNA_SEQUENCE_TYPE();
    outMap[ BaseSlots::ANNOTATION_TABLE_SLOT() ] = BaseTypes::ANNOTATION_TABLE_TYPE();

    DataTypePtr outType( new MapDataType(Descriptor(TYPE), outMap) );
    DataTypeRegistry  *dr = WorkflowEnv::getDataTypeRegistry();
    assert(dr);
    dr->registerEntry(outType);

    Descriptor desc(ACTOR_ID,
                    RemoteDBFetcherWorker::tr("Read from Remote Database"),
                    RemoteDBFetcherWorker::tr("Reads sequences and annotations if any from a remote database."));

    QList<PortDescriptor*> pds;
    {
        Descriptor outd(BasePorts::OUT_SEQ_PORT_ID(), RemoteDBFetcherWorker::tr("Sequence"), RemoteDBFetcherWorker::tr("Sequence"));
        pds << new PortDescriptor(outd, outType, /*input*/ false, /*output*/ true);
    }
    
    RemoteDBRegistry &registry = RemoteDBRegistry::getRemoteDBRegistry();
    QStringList dataBases = registry.getDBs();
    QString defaultDB = dataBases.first();

    QList<Attribute*> attrs;
    {
        Descriptor dbidd(DBID_ID,
                         RemoteDBFetcherWorker::tr("Database"),
                         RemoteDBFetcherWorker::tr("The database to read from."));
        attrs << new Attribute(dbidd, BaseTypes::STRING_TYPE(), true, cuteDbNames.value(defaultDB, defaultDB));

        Descriptor seqidd(SEQID_ID,
                         RemoteDBFetcherWorker::tr("Resource ID(s)"),
                         RemoteDBFetcherWorker::tr("Semicolon-separated list of resource ID`s in the database."));
        attrs << new Attribute(seqidd, BaseTypes::STRING_TYPE(), true, "");

        Descriptor fullpathd(PATH_ID, 
                         RemoteDBFetcherWorker::tr("Save file to directory"), 
                         RemoteDBFetcherWorker::tr("The directory to store sequence files loaded from a database."));
        attrs << new Attribute(fullpathd, BaseTypes::STRING_TYPE(), true, DEFAULT_PATH);
    }
    
    ActorPrototype *proto = new IntegralBusActorPrototype(desc, pds, attrs);

    QMap<QString, PropertyDelegate*> delegates;
    {
        QVariantMap values;
        foreach(const QString &dbName, dataBases)
        {
            values[dbName] = cuteDbNames.value(dbName, dbName);
        }
        delegates[DBID_ID] = new ComboBoxDelegate(values);

        delegates[SEQID_ID] = new StringListDelegate();
        delegates[PATH_ID] = new URLDelegate(QString(), QString(), false, true);
    }

    proto->setEditor(new DelegateEditor(delegates));
    proto->setPrompter(new RemoteDBFetcherPrompter());

    if(AppContext::isGUIMode()) {
        proto->setIcon(GUIUtils::createRoundIcon(ICOLOR, 22));
    }
    
    WorkflowEnv::getProtoRegistry()->registerProto(BaseActorCategories::CATEGORY_DATASRC(), proto);

    DomainFactory *localDomain = WorkflowEnv::getDomainRegistry()->getById(LocalDomainFactory::ID);
    localDomain->registerEntry(new RemoteDBFetcherFactory());
}

///////////////////////////////////////////////////////////////////////////////////////////////////////


/* class FetchSequenceByIdFromAnnotationWorker : public BaseWorker */

FetchSequenceByIdFromAnnotationWorker::FetchSequenceByIdFromAnnotationWorker(Actor *a) : BaseWorker(a), input(NULL), output(NULL)
{
}

void FetchSequenceByIdFromAnnotationWorker::init()
{
    input = ports.value(BasePorts::IN_ANNOTATIONS_PORT_ID());
    output = ports.value(BasePorts::OUT_SEQ_PORT_ID());

    fullPathDir = actor->getParameter(PATH_ID)->getAttributeValue<QString>(context);
    if (fullPathDir == DEFAULT_PATH) {
         fullPathDir = LoadRemoteDocumentTask::getDefaultDownloadDirectory();
    }

    dbId = actor->getParameter(DBID_ID)->getAttributeValue<QString>(context);
}

QString FetchSequenceByIdFromAnnotationPrompter::composeRichDoc()
{
    IntegralBusPort* input = qobject_cast<IntegralBusPort*>(target->getPort(BasePorts::IN_ANNOTATIONS_PORT_ID()));
    Actor* producer = input->getProducer(BaseSlots::ANNOTATION_TABLE_SLOT().getId());
    QString unsetStr = "<font color='red'>"+tr("unset")+"</font>";
    QString producerName = tr(" from <u>%1</u>").arg(producer ? producer->getLabel() : unsetStr);

    return tr("In each annotation from %1 search for accession ID and download the corresponding sequences.").arg(producerName);
}

Task* FetchSequenceByIdFromAnnotationWorker::tick() {

    if(!QDir(fullPathDir).exists()) {
        if(!QDir().mkpath(fullPathDir)) {
            return new FailTask(tr("Cannot create directory '%1'").arg(fullPathDir));
        }
    }

    if (input->hasMessage()) {
        Message inputMessage = getMessageAndSetupScriptValues(input);
        if (inputMessage.isEmpty()) {
            output->transit();
            return NULL;
        }
        QVariantMap qm = inputMessage.getData().toMap();
        QList<SharedAnnotationData> inputAnns = qVariantValue<QList<SharedAnnotationData> >( qm.value(BaseSlots::ANNOTATION_TABLE_SLOT().getId()) );

        QStringList accIds;

        foreach (const SharedAnnotationData& ann, inputAnns) {
            QString accId  = ann->findFirstQualifierValue("accession");
            if(!accId.isEmpty()) {
                accIds << accId;
            }
        }

        Task* task = new LoadRemoteDocumentTask(accIds.join(","), dbId);
        connect(task, SIGNAL(si_stateChanged()), SLOT(sl_taskFinished()));

        return task;


    } else if (input->isEnded()) {
        setDone();
        output->setEnded();
    }

    return NULL;

}


void FetchSequenceByIdFromAnnotationWorker::cleanup() {
}

void FetchSequenceByIdFromAnnotationWorker::sl_taskFinished() {

    LoadRemoteDocumentTask *loadTask = qobject_cast<LoadRemoteDocumentTask*>(sender());
    assert(loadTask);

    if (loadTask->getState() != Task::State_Finished)
    {
        return;
    }

    if (loadTask->hasError())
    {
        loadTask->setError(tr("failed to load item '%1' from '%2' DB : %3").arg(loadTask->getAccNumber(), loadTask->getDBName(), loadTask->getError()));
        return;
    }

    Document *doc = loadTask->getDocument();
    assert(doc);

    foreach(GObject *gobj, doc->findGObjectByType(GObjectTypes::SEQUENCE))
    {
        U2SequenceObject *dnao = qobject_cast<U2SequenceObject*>(gobj);
        assert(dnao);

        QList<GObject*> allLoadedAnnotations = doc->findGObjectByType(GObjectTypes::ANNOTATION_TABLE);
        QList<GObject*> annotations = GObjectUtils::findObjectsRelatedToObjectByRole(gobj,
                                                                                     GObjectTypes::ANNOTATION_TABLE,
                                                                                     GObjectRelationRole::SEQUENCE,
                                                                                     allLoadedAnnotations,
                                                                                     UOF_LoadedOnly);

        QList<SharedAnnotationData> sads;
        if (!annotations.isEmpty())
        {
            AnnotationTableObject *ato = qobject_cast<AnnotationTableObject*>(annotations.first());
            foreach(Annotation *a, ato->getAnnotations())
            {
                sads << a->data();
            }
        }

        QVariantMap messageData;
        SharedDbiDataHandler seqId = context->getDataStorage()->putSequence(dnao->getWholeSequence());
        messageData[ BaseSlots::DNA_SEQUENCE_SLOT().getId() ] = qVariantFromValue<SharedDbiDataHandler>(seqId);
        messageData[ BaseSlots::ANNOTATION_TABLE_SLOT().getId() ] = qVariantFromValue(sads);

        DataTypePtr messageType = WorkflowEnv::getDataTypeRegistry()->getById(TYPE);

        output->put(Message(messageType, messageData));
    }

    if (input->isEnded()) {
        output->setEnded();
    }

}

/* Class FetchSequencByIdFromAnnotationFactory */

const QString FetchSequenceByIdFromAnnotationFactory::ACTOR_ID("fetch-sequence-by-id-from-annotation");

void FetchSequenceByIdFromAnnotationFactory::init()
{

    //accept annotations as input
    QMap<Descriptor, DataTypePtr> inputMap;
    inputMap[ BaseSlots::ANNOTATION_TABLE_SLOT() ] = BaseTypes::ANNOTATION_TABLE_TYPE();

    // output sequence and annotations
    QMap<Descriptor, DataTypePtr> outputMap;
    outputMap[ BaseSlots::DNA_SEQUENCE_SLOT() ] = BaseTypes::DNA_SEQUENCE_TYPE();
    outputMap[ BaseSlots::ANNOTATION_TABLE_SLOT() ] = BaseTypes::ANNOTATION_TABLE_TYPE();


    //DataTypePtr inputType( new MapDataType(Descriptor(TYPE), inputMap) );
    //DataTypeRegistry  *dr = WorkflowEnv::getDataTypeRegistry();
    //assert(dr);
    //dr->registerEntry(inputType);

    Descriptor desc(ACTOR_ID,
                    FetchSequenceByIdFromAnnotationWorker::tr("Fetch Sequences by ID from Annotation"),
                    FetchSequenceByIdFromAnnotationWorker::tr("Parses annotations to find any IDs and fetches corresponding sequences."));

    QList<PortDescriptor*> pds;
    { //Create input port descriptors
        Descriptor inDesc( BasePorts::IN_ANNOTATIONS_PORT_ID(),FetchSequenceByIdFromAnnotationWorker::tr("Input annotations"),
            FetchSequenceByIdFromAnnotationWorker::tr("The annotations are scanned for accesion ids.") );

           pds << new PortDescriptor( inDesc, DataTypePtr(new MapDataType("input.anns", inputMap)), /*input*/ true );
    }

    {
        Descriptor outDesc(BasePorts::OUT_SEQ_PORT_ID(), FetchSequenceByIdFromAnnotationWorker::tr("Sequence"),
                           FetchSequenceByIdFromAnnotationWorker::tr("The downloaded sequences"));
        pds << new PortDescriptor(outDesc, DataTypePtr(new MapDataType("output.seqs", outputMap)),  false,  true);
    }

    //RemoteDBRegistry &registry = RemoteDBRegistry::getRemoteDBRegistry();
    //QStringList dataBases = registry.getDBs();
    //QString defaultDB = dataBases.first();

    QList<Attribute*> attrs;
    {

        Descriptor fullpathd(PATH_ID,
                         RemoteDBFetcherWorker::tr("Save file to directory"),
                         RemoteDBFetcherWorker::tr("The directory to store sequence files loaded from a database."));
        attrs << new Attribute(fullpathd, BaseTypes::STRING_TYPE(), true, DEFAULT_PATH);

        Descriptor dbidd(DBID_ID,
                         RemoteDBFetcherWorker::tr("NCBI database"),
                         RemoteDBFetcherWorker::tr("The database to read from."));
        attrs << new Attribute(dbidd, BaseTypes::STRING_TYPE(), true, GENBANK_NUCLEOTIDE_ID);
    }

    ActorPrototype *proto = new IntegralBusActorPrototype(desc, pds, attrs);

    QMap<QString, PropertyDelegate*> delegates;
    {
        delegates[PATH_ID] = new URLDelegate(QString(), QString(), false, true);

        QStringList dbIds;
        dbIds.append(GENBANK_NUCLEOTIDE_ID);
        dbIds.append(GENBANK_PROTEIN_ID);

        QVariantMap dbValues;
        foreach( const QString& id, dbIds) {
            dbValues[id] = id;
        }

        delegates[DBID_ID] = new ComboBoxDelegate(dbValues);

    }

    proto->setEditor(new DelegateEditor(delegates));
    proto->setPrompter(new FetchSequenceByIdFromAnnotationPrompter());

    if(AppContext::isGUIMode()) {
        proto->setIcon(GUIUtils::createRoundIcon(ICOLOR, 22));
    }

    WorkflowEnv::getProtoRegistry()->registerProto(BaseActorCategories::CATEGORY_BASIC(), proto);

    DomainFactory *localDomain = WorkflowEnv::getDomainRegistry()->getById(LocalDomainFactory::ID);
    localDomain->registerEntry(new FetchSequenceByIdFromAnnotationFactory());


}




}   // namespace U2::LocalWorkflow
}
