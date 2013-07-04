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

#include <U2Lang/ActorPrototypeRegistry.h>
#include <U2Lang/BaseActorCategories.h>
#include <U2Lang/BasePorts.h>
#include <U2Lang/BaseSlots.h>
#include <U2Lang/BaseTypes.h>
#include <U2Lang/WorkflowEnv.h>

#include <U2Designer/DelegateEditors.h>

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

QList<Task *> AnalyzeTataBoxesWorker::createVariationProcessingTasks( const U2Variant &var,
    const U2VariantTrack &track, U2Dbi *dbi )
{
    QList<Task *> result;
    qint64 sequenceStartPos = 0;
    result << new AnalyzeTataBoxesTask( getSequenceForVariant( var, track, dbi, sequenceStartPos ) );
    return result;
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
        delegates[BaseRequestForSnpWorker::DB_SEQUENCE_PATH] = new URLDelegate( "", "", false );
    }

    Descriptor protoDesc( AnalyzeTataBoxesWorkerFactory::ACTOR_ID,
        QObject::tr( "Determine SNP effect on TATA-boxes" ),
        QObject::tr( "Define the influence of SNP on TATA-boxes belonging to the sequence" ) );

    ActorPrototype *proto = new IntegralBusActorPrototype( protoDesc, p, a );
    proto->setPrompter( new AnalyzeTataBoxesPrompter( ) );
    proto->setEditor( new DelegateEditor( delegates ) );
    WorkflowEnv::getProtoRegistry( )->registerProto( BaseActorCategories::CATEGORY_SCHEMAS( ),
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