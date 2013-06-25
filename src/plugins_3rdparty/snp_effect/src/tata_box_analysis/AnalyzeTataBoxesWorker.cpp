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

#include "AnalyzeTataBoxesTask.h"
#include "AnalyzeTataBoxesWorker.h"

namespace U2 {

namespace LocalWorkflow {

const QString AnalyzeTataBoxesWorkerFactory::ACTOR_ID( "tata-box-snp" );

/************************************************************************/
/* Worker */
/************************************************************************/

AnalyzeTataBoxesWorker::AnalyzeTataBoxesWorker( Actor *p )
    : BaseWorker( p ), inChannel( NULL ), outChannel( NULL )
{

}

void AnalyzeTataBoxesWorker::init( )
{
    inChannel = ports.value( BasePorts::IN_VARIATION_TRACK_PORT_ID( ) );
    outChannel = ports.value( BasePorts::OUT_VARIATION_TRACK_PORT_ID( ) );
}

void AnalyzeTataBoxesWorker::cleanup( )
{

}

Task* AnalyzeTataBoxesWorker::tick( )
{
    U2OpStatus2Log os;
    if ( inChannel->hasMessage( ) ) {
        // TODO: obtain `sequence` from DB
        QString sequence;
        Task* t = new AnalyzeTataBoxesTask( sequence );
        connect( t, SIGNAL( si_stateChanged( ) ), SLOT( sl_taskFinished( ) ) );
        return t;
    }
    if ( inChannel->isEnded( ) ) {
        setDone( );
        outChannel->setEnded( );
    }
    return NULL;
}


void AnalyzeTataBoxesWorker::sl_taskFinished( )
{
    AnalyzeTataBoxesTask *t = dynamic_cast<AnalyzeTataBoxesTask *>( sender( ) );
    SAFE_POINT( NULL != t, "Invalid task is encountered", );
    if ( !t->isFinished( ) || t->hasError( ) ) {
        return;
    }
    // TODO: put task report to DB
    outChannel->put( Message::getEmptyMapMessage( ) );
    if ( inChannel->isEnded( ) && !inChannel->hasMessage( ) ) {
        setDone( );
        outChannel->setEnded( );
    }
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

    Descriptor protoDesc( AnalyzeTataBoxesWorkerFactory::ACTOR_ID,
        QObject::tr( "Determine SNP effect on TATA-boxes" ),
        QObject::tr( "Define the influence of SNP on TATA-boxes belonging to the sequence" ) );

    ActorPrototype *proto = new IntegralBusActorPrototype( protoDesc, p );
    proto->setPrompter( new AnalyzeTataBoxesPrompter( ) );
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
    //Actor* annProducer = qobject_cast<IntegralBusPort*>(
    //    target->getPort(BasePorts::IN_VARIATION_TRACK_PORT_ID( ) ) )->getProducer(
    //    BaseSlots::VARIATION_TRACK_SLOT( ).getId( ) );

    //QString unsetStr = "<font color='red'>" + tr("unset") + "</font>";
    //QString annUrl = annProducer ? annProducer->getLabel( ) : unsetStr;

    //QString path = getHyperlink(DB_PATH, getURL(DB_PATH));

    //res.append(tr("Uses variations from <u>%1</u> as input.").arg(annUrl));
    //res.append(tr(" Takes annotations from <u>%1</u> database.").arg(path.isEmpty() ? unsetStr : path));

    return res;
}

} // namespace LocalWorkflow

} // namespace U2