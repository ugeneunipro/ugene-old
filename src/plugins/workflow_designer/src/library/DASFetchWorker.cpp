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
#include <U2Core/LoadDASDocumentTask.h>
#include <U2Core/DASSource.h>
#include <U2Core/U2SafePoints.h>

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

#include "DASFetchWorker.h"

namespace U2 {
namespace LocalWorkflow {

#define ICOLOR QColor(85,85,255) // FIXME

const QString DASFetcherFactory::ACTOR_ID("fetch-das");

static const QString TYPE("das.seq");
static const QString DBID_ID("database");
static const QString ANNID_ID("annotations");
static const QString SEQID_ID("resource-id");
static const QString PATH_ID("save-dir");
static const QString DEFAULT_PATH("default");


//////////////////////////////////////////////////////////////////////////
//DASFetcherPrompter
QString DASFetcherPrompter::composeRichDoc()
{
    QString unsetStr = "<font color='red'>"+tr("unset")+"</font>";
    QStringList seqids = getParameter(SEQID_ID).value<QString>().split(";", QString::SkipEmptyParts);
    QString seq = seqids.size() > 1 ?
        DASFetcherWorker::tr("sequences identified with") :
        DASFetcherWorker::tr("sequence identified with");
    QString seqidsStr = seqids.isEmpty() ? unsetStr : QString("<u>%1</u>").arg(seqids.join(", "));
    
    QString dbid = getParameter(DBID_ID).value<QString>();
    DASSourceRegistry * dasRegistry = AppContext::getDASSourceRegistry();
    if (dasRegistry){
        DASSource rs = dasRegistry->findById(dbid);
        if (rs.isValid()){
            dbid = rs.getName();
        }
    }

    QString saveDir = getParameter(PATH_ID).value<QString>();
    saveDir = getHyperlink(PATH_ID, saveDir);
    QString saveDirStr = DASFetcherWorker::tr("Save result to <u>%1</u> directory.").arg(saveDir);
    
    return DASFetcherWorker::tr("Reads %1 %2 from <u>%3</u> DAS source. %4").
        arg(seq).
        arg(getHyperlink(SEQID_ID, seqidsStr)).
        arg(getHyperlink(DBID_ID, dbid)).
        arg(saveDirStr);
}

//////////////////////////////////////////////////////////////////////////
//DASFetcherWorker
DASFetcherWorker::DASFetcherWorker(Actor *a) : BaseWorker(a), output(NULL)
{
}

void DASFetcherWorker::init()
{
    output = ports.value(BasePorts::OUT_SEQ_PORT_ID());
    
    assert(dbid.isEmpty());
    dbid = actor->getParameter(DBID_ID)->getAttributeValue<QString>(context);

    seqids = actor->getParameter(SEQID_ID)->getAttributeValue<QString>(context).split(";", QString::SkipEmptyParts);

    featureids = actor->getParameter(ANNID_ID)->getAttributeValue<QString>(context).split(",", QString::SkipEmptyParts);
    
    fullPathDir = actor->getParameter(PATH_ID)->getAttributeValue<QString>(context);
    if (fullPathDir == DEFAULT_PATH) {
        fullPathDir = LoadRemoteDocumentTask::getDefaultDownloadDirectory();
    } else {
        fullPathDir = context->absolutePath(fullPathDir);
    }
}

bool DASFetcherWorker::isReady()
{
    return !isDone();
}

Task* DASFetcherWorker::tick() {
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

    DASSource refSource;
    QList<DASSource> featureSources;
    DASSourceRegistry * dasRegistry = AppContext::getDASSourceRegistry();
    if (!dasRegistry){
        return ret;
    }else{
        refSource = dasRegistry->findById(dbid);
        if (!refSource.isValid()){
            return ret;
        }
        foreach(const QString& fId, featureids){
            if (!fId.isEmpty()){
                const DASSource& fs = dasRegistry->findByName(fId);
                if (fs.isValid()){
                    featureSources.append(fs);
                }
            }
        }
    }

    ret = new ConvertIdAndLoadDASDocumentTask(seqId, fullPathDir, refSource, featureSources);
    connect(ret, SIGNAL(si_stateChanged()), SLOT(sl_taskFinished()));

    return ret;
}

bool DASFetcherWorker::isDone() {
    return (!dbid.isEmpty() && seqids.isEmpty());
}

void DASFetcherWorker::cleanup() {
}

void DASFetcherWorker::sl_taskFinished() {
    ConvertIdAndLoadDASDocumentTask *loadTask = qobject_cast<ConvertIdAndLoadDASDocumentTask*>(sender());
    if(!loadTask){
        return;
    }

    if (loadTask->getState() != Task::State_Finished || loadTask->isCanceled()){
        return;
    }

    if (loadTask->hasError()) {
        loadTask->setError(tr("failed to load item from DAS source : %3").arg(loadTask->getError()));
        return;
    }

    Document *doc = loadTask->getDocument();
    if (!doc){
        return;
    }

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

        QList<AnnotationData> ads;
        if (!annotations.isEmpty()) {
            AnnotationTableObject *ato = qobject_cast<AnnotationTableObject *>(annotations.first());
            foreach ( const Annotation &a, ato->getAnnotations( ) ) {
                ads << a.getData( );
            }
        }

        QVariantMap messageData;
        SharedDbiDataHandler seqId = context->getDataStorage()->putSequence(dnao->getWholeSequence());
        messageData[ BaseSlots::DNA_SEQUENCE_SLOT().getId() ] = qVariantFromValue<SharedDbiDataHandler>(seqId);
        SharedDbiDataHandler tableId = context->getDataStorage()->putAnnotationTable(ads);
        messageData[ BaseSlots::ANNOTATION_TABLE_SLOT().getId() ] = qVariantFromValue<SharedDbiDataHandler>(tableId);

        DataTypePtr messageType = WorkflowEnv::getDataTypeRegistry()->getById(TYPE);

        output->put(Message(messageType, messageData));
    }

    if (seqids.isEmpty())
    {
        output->setEnded();
    }
}

//////////////////////////////////////////////////////////////////////////
//DASFetcherFactory

class DASSourceTypeRelation : public AttributeRelation {
public:
    DASSourceTypeRelation(const QString &relatedAttrId)
        : AttributeRelation(relatedAttrId){}

    virtual QVariant getAffectResult(const QVariant &influencingValue, const QVariant &dependentValue,
        DelegateTags *, DelegateTags *) const{
        DASSourceRegistry * dasRegistry = AppContext::getDASSourceRegistry();
        if (dasRegistry){
            DASSource refsource = dasRegistry->findById(influencingValue.toString());
            const QList<DASSource>& featuresSources = dasRegistry->getFeatureSourcesByType(refsource.getReferenceType());
            QString res = "";
            foreach(const DASSource& ds, featuresSources){
                if (res != ""){
                    res.append(",");
                }
                res.append(ds.getName());
            }
            return res;
        }
        
        return dependentValue;
    };
    virtual RelationType getType() const {return CUSTOM_VALUE_CHANGER;}
};


void DASFetcherFactory::init()
{
    QMap<Descriptor, DataTypePtr> outMap;
    outMap[ BaseSlots::DNA_SEQUENCE_SLOT() ] = BaseTypes::DNA_SEQUENCE_TYPE();
    outMap[ BaseSlots::ANNOTATION_TABLE_SLOT() ] = BaseTypes::ANNOTATION_TABLE_TYPE();

    DataTypePtr outType( new MapDataType(Descriptor(TYPE), outMap) );
    DataTypeRegistry  *dr = WorkflowEnv::getDataTypeRegistry();
    assert(dr);
    dr->registerEntry(outType);

    Descriptor desc(ACTOR_ID,
                    DASFetcherWorker::tr("Read from DAS"),
                    DASFetcherWorker::tr("Reads sequences and annotations if any from the Distributed Annotation System."));

    QList<PortDescriptor*> pds;
    {
        Descriptor outd(BasePorts::OUT_SEQ_PORT_ID(), DASFetcherWorker::tr("Sequence"), DASFetcherWorker::tr("Sequence"));
        pds << new PortDescriptor(outd, outType, /*input*/ false, /*output*/ true);
    }

    DASSourceRegistry * dasRegistry = AppContext::getDASSourceRegistry();
    SAFE_POINT (dasRegistry, DASFetcherWorker::tr("No DAS registry"), );

    const QList<DASSource>& dasSources = dasRegistry->getReferenceSources();
    SAFE_POINT (dasSources.size() != 0, DASFetcherWorker::tr("No DAS sources"), );
    const QList<DASSource>& featureSources = dasRegistry->getFeatureSources();
    SAFE_POINT (featureSources.size() != 0, DASFetcherWorker::tr("No DAS feature sources"), );
    

    DASSource defaultDB = dasSources.first();
    QString defaultFeatures = "";
    foreach(const DASSource& fs, featureSources){
        if (fs.getReferenceType() == defaultDB.getReferenceType()){
            if (defaultFeatures!=""){
                defaultFeatures.append(",");
            }
            defaultFeatures.append(fs.getName());
        }
    }

    QList<Attribute*> attrs;
    {
        Descriptor annidd(ANNID_ID,
            DASFetcherWorker::tr("Feature Sources"),
            DASFetcherWorker::tr("The DAS sources to read features from."));
        attrs << new Attribute(annidd, BaseTypes::STRING_TYPE(), true, defaultFeatures);

        Descriptor dbidd(DBID_ID,
                         DASFetcherWorker::tr("Reference Source"),
                         DASFetcherWorker::tr("The DAS source to read reference from."));
        
        
        Attribute* seqAtr = new Attribute(dbidd, BaseTypes::STRING_TYPE(), true, defaultDB.getId());

        seqAtr->addRelation(new DASSourceTypeRelation(ANNID_ID));

        attrs << seqAtr;

        Descriptor seqidd(SEQID_ID,
            DASFetcherWorker::tr("Resource ID(s)"),
            DASFetcherWorker::tr("Semicolon-separated list of resource ID`s in the source."));


        attrs << new Attribute(seqidd, BaseTypes::STRING_TYPE(), true, "");

        Descriptor fullpathd(PATH_ID, 
                         DASFetcherWorker::tr("Save file to directory"), 
                         DASFetcherWorker::tr("The directory to store sequence files loaded from the source."));
        attrs << new Attribute(fullpathd, BaseTypes::STRING_TYPE(), true, DEFAULT_PATH);
    }
    
    ActorPrototype *proto = new IntegralBusActorPrototype(desc, pds, attrs);

    QMap<QString, PropertyDelegate*> delegates;
    {
        QVariantMap values;
        foreach(const DASSource &dbS, dasSources)
        {
            values[dbS.getName()] = dbS.getId();
        }
        delegates[DBID_ID] = new ComboBoxDelegate(values);

        delegates[SEQID_ID] = new StringListDelegate();
        delegates[PATH_ID] = new URLDelegate(QString(), QString(), false, true);
    }
    {
        QVariantMap values;
        
        foreach(const DASSource& fs, featureSources){
            bool active = false;
            if (fs.getReferenceType() == defaultDB.getReferenceType()){
                active = true;
            }
            values[fs.getName()] = active;
        }
        delegates[ANNID_ID] = new ComboBoxWithChecksDelegate(values);
    }

    proto->setEditor(new DelegateEditor(delegates));
    proto->setPrompter(new DASFetcherPrompter());

    if(AppContext::isGUIMode()) {
        proto->setIcon(GUIUtils::createRoundIcon(ICOLOR, 22));
    }
    
    WorkflowEnv::getProtoRegistry()->registerProto(BaseActorCategories::CATEGORY_DATASRC(), proto);

    DomainFactory *localDomain = WorkflowEnv::getDomainRegistry()->getById(LocalDomainFactory::ID);
    localDomain->registerEntry(new DASFetcherFactory());
}

}   // namespace U2::LocalWorkflow
}
