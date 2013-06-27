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
#include <U2Lang/BaseTypes.h>
#include <U2Lang/BaseSlots.h>
#include <U2Lang/BasePorts.h>
#include <U2Lang/WorkflowEnv.h>

#include "RSnpToolsWorker.h"

namespace U2 {

namespace LocalWorkflow {

const QString RSnpToolsWorkerFactory::ACTOR_ID( "rSnp-tools" );

/************************************************************************/
/* Worker */
/************************************************************************/

RSnpToolsWorker::RSnpToolsWorker( Actor *p )
    : BaseRequestForSnpWorker( p )
{

}

void RSnpToolsWorker::cleanup( )
{

}

QVariantMap RSnpToolsWorker::getInputDataForRequest( )
{
    QVariantMap inputData;
    // TODO: obtain sequences from DB
    inputData[SnpRequestKeys::R_SNP_FIRST_SEQUENCE] = "cctcagtgctgagggccaagcaaatatttgtggttatggaTtaactcgaactccaggctgtcatggcggcaggacggcgaa";
    inputData[SnpRequestKeys::R_SNP_SECOND_SEQUENCE] = "cctcagtgctgagggccaagcaaatatttgtggttatggaCtaactcgaactccaggctgtcatggcggcaggacggcgaa";
    return inputData;
}

QString RSnpToolsWorker::getRequestingScriptName( ) const
{
    return SnpRequestingScripts::R_SNP_TOOLS_SCRIPT;
}

/************************************************************************/
/* Factory */
/************************************************************************/

RSnpToolsWorkerFactory::RSnpToolsWorkerFactory( )
    : DomainFactory( ACTOR_ID )
{

}

void RSnpToolsWorkerFactory::init( )
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

    Descriptor protoDesc( RSnpToolsWorkerFactory::ACTOR_ID,
        QObject::tr( "Detect transcription factors with rSNP-Tools" ),
        QObject::tr( "Identification of transcription factor binding sites in the DNA "
            "which have been modified by polymorphic mutation." ) );

    ActorPrototype *proto = new IntegralBusActorPrototype( protoDesc, p );
    proto->setPrompter( new RSnpToolsPrompter( ) );
    WorkflowEnv::getProtoRegistry( )->registerProto( BaseActorCategories::CATEGORY_SCHEMAS( ),
        proto );
    WorkflowEnv::getDomainRegistry( )->getById( LocalDomainFactory::ID )->registerEntry(
        new RSnpToolsWorkerFactory( ) );
}

Worker *RSnpToolsWorkerFactory::createWorker( Actor *a )
{
    return new RSnpToolsWorker( a );
}

/************************************************************************/
/* Prompter */
/************************************************************************/

RSnpToolsPrompter::RSnpToolsPrompter( Actor *p )
    : PrompterBase<RSnpToolsPrompter>( p )
{

}

QString RSnpToolsPrompter::composeRichDoc( )
{
    QString res = ""; 
    Actor* annProducer = qobject_cast<IntegralBusPort*>(
        target->getPort(BasePorts::IN_VARIATION_TRACK_PORT_ID( ) ) )->getProducer(
        BaseSlots::VARIATION_TRACK_SLOT( ).getId( ) );

    QString unsetStr = "<font color='red'>" + tr("unset") + "</font>";
    QString annUrl = annProducer ? annProducer->getLabel( ) : unsetStr;

    res.append(tr("Uses variations from <u>%1</u> as input.").arg(annUrl));

    return res;
}

} // LocalWorkflow
} // U2
