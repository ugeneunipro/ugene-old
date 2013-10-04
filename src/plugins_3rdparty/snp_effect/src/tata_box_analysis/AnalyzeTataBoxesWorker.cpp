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

#include <U2Core/FailTask.h>
#include <U2Core/U2OpStatusUtils.h>
#include <U2Core/U2SafePoints.h>
#include <U2Core/AppContext.h>
#include <U2Core/U2Region.h>
#include <U2Core/Gene.h>
#include <U2Core/SNPTablesUtils.h>
#include <U2Core/U2SequenceDbi.h>

#include <U2Lang/ActorPrototypeRegistry.h>
#include <U2Lang/BaseActorCategories.h>
#include <U2Lang/BasePorts.h>
#include <U2Lang/BaseSlots.h>
#include <U2Lang/BaseTypes.h>
#include <U2Lang/WorkflowEnv.h>

#include <U2Designer/DelegateEditors.h>

#include <U2Formats/SNPDatabaseUtils.h>

#include "AnalyzeTataBoxesTask.h"
#include "AnalyzeTataBoxesWorker.h"

namespace U2 {

namespace LocalWorkflow {

const QString AnalyzeTataBoxesWorkerFactory::ACTOR_ID( "tata-box-snp" );

/************************************************************************/
/* Worker */
/************************************************************************/

AnalyzeTataBoxesWorker::AnalyzeTataBoxesWorker( Actor *p )
    : BaseRequestForSnpWorker( p )
{

}

#define TATA_OFFSET_SMALL 20
#define TATA_OFFSET_BIG 70
static U2Region getTataRegion(const Gene& gene){
    U2Region res;

    qint64 start = -1;
    if (gene.isComplemented()){
        res = U2Region(gene.getRegion().endPos() + TATA_OFFSET_SMALL, TATA_OFFSET_BIG - TATA_OFFSET_SMALL); //upped bound of the sequence is not checked
    }else{
        if (gene.getRegion().startPos - TATA_OFFSET_BIG < 0){ //bad TATA
            return res;
        }
        res = U2Region(gene.getRegion().startPos - TATA_OFFSET_BIG, TATA_OFFSET_BIG - TATA_OFFSET_SMALL); //upped bound of the sequence is not checked
    }

    return res;
}

QList<Task *> AnalyzeTataBoxesWorker::createVariationProcessingTasks( const U2Variant &var,
    const U2VariantTrack &track, U2Dbi *dbi )
{
    QList<Task *> tasks;

    U2ObjectDbi* objDbi = dbi->getObjectDbi();
    SAFE_POINT(objDbi!=NULL, "No object DBI", tasks);
    U2FeatureDbi* featureDbi = dbi->getFeatureDbi();
    SAFE_POINT(featureDbi!=NULL, "No feature DBI", tasks);
    U2SequenceDbi* seqDbi = dbi->getSequenceDbi();
    SAFE_POINT(seqDbi!=NULL, "No sequence DBI", tasks);

    U2DataId seqId = track.sequence.isEmpty() ? SNPDatabaseUtils::getSequenceId(track.sequenceName, objDbi) : track.sequence;
    U2OpStatusImpl os;
    QList<Gene> genes = SNPTablesUtils::findRegulatedGenes(seqId, VARIATION_REGION(var), featureDbi, os);
    CHECK_OP(os, tasks);
    foreach(const Gene& gene, genes){
        QVariantMap inputData;

        U2Region tataReg = getTataRegion(gene);
        if (tataReg == U2Region()){
            continue;
        }

        if (!tataReg.contains(var.startPos)){
            continue;
        }
        
        //get sequence
        QByteArray seq1 = seqDbi->getSequenceData(seqId, tataReg, os);
        CHECK_OP(os, tasks);
        qint64 mutationPos = var.startPos - tataReg.startPos;
        QByteArray seq2 = seq1;
        seq2.replace( mutationPos, 1, var.obsData );

        inputData[SnpRequestKeys::SNP_FEATURE_ID_KEY] = gene.getFeatureId();
        inputData[SnpRequestKeys::TATA_TOOLS_SEQ_1] = seq1;
        inputData[SnpRequestKeys::TATA_TOOLS_SEQ_2] = seq2;

        Task* t = new AnalyzeTataBoxesTask(inputData, var);
        connect( t, SIGNAL( si_stateChanged( ) ), SLOT( sl_taskFinished( ) ) );
        tasks.append(t);

    }

    return tasks;
}

QList<SnpResponseKey> AnalyzeTataBoxesWorker::getResultKeys() const{
    QList<SnpResponseKey> res;
    res << SnpResponseKeys::TATA_TOOLS;
    return res;
}

/************************************************************************/
/* Factory */
/************************************************************************/

AnalyzeTataBoxesWorkerFactory::AnalyzeTataBoxesWorkerFactory( )
    : DomainFactory( ACTOR_ID )
{

}

void AnalyzeTataBoxesWorkerFactory::init( )
{
    //init data path
    U2DataPath* dataPath = NULL;
    U2DataPathRegistry* dpr =  AppContext::getDataPathRegistry();
    if (dpr){
        U2DataPath* dp = dpr->getDataPathByName(BaseRequestForSnpWorker::DB_SEQUENCE_PATH);
        if (dp && dp->isValid()){
            dataPath = dp;
        }
    }
    QList<PortDescriptor*> p;
    QList<Attribute*> a;
    {
        Descriptor sd( BasePorts::IN_VARIATION_TRACK_PORT_ID( ), "Input variations",
            "Variations for annotations." );
        Descriptor od( BasePorts::OUT_VARIATION_TRACK_PORT_ID( ), "Output variations",
            "Variations with annotations." );

        QMap<Descriptor, DataTypePtr> modelM;
        QMap<Descriptor, DataTypePtr> inM;
        inM[BaseSlots::VARIATION_TRACK_SLOT( )] = BaseTypes::VARIATION_TRACK_TYPE( );
        p << new PortDescriptor( sd, DataTypePtr( new MapDataType( "in.variations", inM ) ),
            true /*input*/ );
        QMap<Descriptor, DataTypePtr> outM;
        p << new PortDescriptor( od, DataTypePtr( new MapDataType( "out.variations", outM ) ),
            false /*input*/, true /*multi*/ );
    }

    Descriptor dbPath( BaseRequestForSnpWorker::DB_SEQUENCE_PATH, QObject::tr( "Database path" ),
        QObject::tr( "Path to SNP database." ) );
    
    a << new Attribute( dbPath, BaseTypes::STRING_TYPE( ), true, dataPath != NULL ? dataPath->getPath() : "" );

    QMap<QString, PropertyDelegate *> delegates;
    {
        delegates[BaseRequestForSnpWorker::DB_SEQUENCE_PATH] = new URLDelegate( "", "", false, false, false );
    }

    Descriptor protoDesc( AnalyzeTataBoxesWorkerFactory::ACTOR_ID,
        QObject::tr( "Determine SNP effect on TATA-boxes" ),
        QObject::tr( "Define the influence of SNP on TATA-boxes belonging to the sequence" ) );

    ActorPrototype *proto = new IntegralBusActorPrototype( protoDesc, p, a );
    proto->setPrompter( new AnalyzeTataBoxesPrompter( ) );
    proto->setEditor( new DelegateEditor( delegates ) );
    WorkflowEnv::getProtoRegistry( )->registerProto( BaseActorCategories::CATEGORY_SNP_ANNOTATION( ),
        proto );
    WorkflowEnv::getDomainRegistry( )->getById( LocalDomainFactory::ID )->registerEntry(
        new AnalyzeTataBoxesWorkerFactory( ) );
}

Worker *AnalyzeTataBoxesWorkerFactory::createWorker( Actor *a )
{
    return new AnalyzeTataBoxesWorker( a );
}

/************************************************************************/
/* Prompter */
/************************************************************************/

AnalyzeTataBoxesPrompter::AnalyzeTataBoxesPrompter( Actor *p )
    : PrompterBase<AnalyzeTataBoxesPrompter>( p )
{

}

QString AnalyzeTataBoxesPrompter::composeRichDoc( )
{
    QString res = ""; 
    Actor* annProducer = qobject_cast<IntegralBusPort*>(
        target->getPort(BasePorts::IN_VARIATION_TRACK_PORT_ID( ) ) )->getProducer(
        BaseSlots::VARIATION_TRACK_SLOT( ).getId( ) );

    QString unsetStr = "<font color='red'>" + tr( "unset" ) + "</font>";
    QString annUrl = annProducer ? annProducer->getLabel( ) : unsetStr;
    QString path = getHyperlink( BaseRequestForSnpWorker::DB_SEQUENCE_PATH,
        getURL( BaseRequestForSnpWorker::DB_SEQUENCE_PATH ) );

    res.append( tr( "Uses variations from <u>%1</u> as input." ).arg( annUrl ) );
    res.append( tr( " Takes sequences from <u>%1</u> database." ).arg( path.isEmpty( ) ?
        unsetStr : path ) );
    return res;
}

} // namespace LocalWorkflow

} // namespace U2