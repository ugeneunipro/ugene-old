#include "RemoteDBFetcherWorker.h"

#include <U2Core/DocumentModel.h>
#include <U2Core/AppContext.h>

#include <U2Core/GObjectUtils.h>
#include <U2Core/DNASequenceObject.h>
#include <U2Core/GObjectRelationRoles.h>
#include <U2Core/AnnotationTableObject.h>

#include <U2Core/AnnotationData.h>

#include <U2Lang/ActorModel.h>
#include <U2Lang/WorkflowEnv.h>
#include <U2Lang/ActorPrototypeRegistry.h>

#include <U2Lang/BaseSlots.h>
#include <U2Lang/BaseTypes.h>
#include <U2Lang/BasePorts.h>
#include <U2Lang/BaseActorCategories.h>

#include <U2Lang/CoreLibConstants.h>
#include <U2Designer/DelegateEditors.h>

#include <U2Core/LoadRemoteDocumentTask.h>
#include <U2Core/FailTask.h>

#include <U2Gui/GUIUtils.h>

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
    QString seq = seqids.size() > 1 ? RemoteDBFetcherWorker::tr("sequences") : RemoteDBFetcherWorker::tr("sequence");
    QString seqidsStr = seqids.isEmpty() ? unsetStr : QString("<u>%1</u>").arg(seqids.join(", "));
    
    QString dbid = getParameter(DBID_ID).value<QString>();
    dbid = RemoteDBFetcherFactory::cuteDbNames.key(dbid, dbid);
    
    QString saveDir = getParameter(PATH_ID).value<QString>();
    QString saveDirStr = RemoteDBFetcherWorker::tr("Save result to <u>%1</u> directory").arg(saveDir);
    
    return RemoteDBFetcherWorker::tr("Reads %1 identified with %2 from <u>%3</u> remote database. %4").
        arg(seq).
        arg(seqidsStr).
        arg(dbid).
        arg(saveDirStr);
}

/* class RemoteDBFetcherWorker : public BaseWorker */

RemoteDBFetcherWorker::RemoteDBFetcherWorker(Actor *a) : BaseWorker(a), output(0)
{
}

void RemoteDBFetcherWorker::init()
{
    output = ports.value(BasePorts::OUT_SEQ_PORT_ID());
    
    assert(dbid.isEmpty());
    dbid = actor->getParameter(DBID_ID)->getAttributeValue<QString>();
    if(RemoteDBFetcherFactory::cuteDbNames.values().contains(dbid.toLower())) {
        dbid = RemoteDBFetcherFactory::cuteDbNames.key(dbid.toLower());
        assert(!dbid.isEmpty());
    }
    seqids = actor->getParameter(SEQID_ID)->getAttributeValue<QString>().split(";", QString::SkipEmptyParts);
    
    fullPathDir = actor->getParameter(PATH_ID)->getAttributeValue<QString>();
    if (fullPathDir == DEFAULT_PATH) {
        fullPathDir = LoadRemoteDocumentTask::getDefaultDownloadDirectory();
    }
}

bool RemoteDBFetcherWorker::isReady()
{
    return !isDone();
}

Task* RemoteDBFetcherWorker::tick() {
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

    if (loadTask->hasErrors())
    {
        loadTask->setError(tr("failed to load item '%1' from '%2' DB : %3").arg(loadTask->getAccNumber(), loadTask->getDBName(), loadTask->getError()));
        return;
    }

    Document *doc = loadTask->getDocument();
    assert(doc);

    foreach(GObject *gobj, doc->findGObjectByType(GObjectTypes::SEQUENCE))
    {
        DNASequenceObject *dnao = qobject_cast<DNASequenceObject*>(gobj);
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
        messageData[ BaseSlots::DNA_SEQUENCE_SLOT().getId() ] = qVariantFromValue(dnao->getDNASequence());
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
    // FIXME: use constants. see LoadRemoteDocumentTask.cpp
    ret["NCBI GenBank (DNA sequence)"] ="ncbi dna";
    ret["NCBI protein sequence database"] ="ncbi protein";
    ret["PDB"] = "pdb";
    ret["SWISS-PROT"] = "swiss-prot";
    ret["UniProtKB/Swiss-Prot"] = "uniprot-swiss-prot";
    ret["UniProtKB/TrEMBL"] = "uniprot-trembl";
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
                    RemoteDBFetcherWorker::tr("Read from remote database"),
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
                         RemoteDBFetcherWorker::tr("Resource IDs"),
                         RemoteDBFetcherWorker::tr("Semicolon-separated list of resource IDs in the database."));
        attrs << new Attribute(seqidd, BaseTypes::STRING_TYPE(), true, "");

        Descriptor fullpathd(PATH_ID, 
                         RemoteDBFetcherWorker::tr("Save file to directory"), 
                         RemoteDBFetcherWorker::tr("Directory to store sequence files loaded from a database."));
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

}   // namespace U2::LocalWorkflow
}
