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

#include <U2Designer/DelegateEditors.h>

#include "SnpChIpToolsWorker.h"

namespace U2 {

namespace LocalWorkflow {

const QString SnpChipToolsWorkerFactory::ACTOR_ID( "snp-chip-tools" );

/************************************************************************/
/* Worker */
/************************************************************************/

SnpChipToolsWorker::SnpChipToolsWorker( Actor *p )
    : BaseRequestForSnpWorker( p )
{

}

QList<QVariantMap> SnpChipToolsWorker::getInputDataForRequest( const U2Variant& variant,
    const U2VariantTrack& track, U2Dbi* dataBase )
{
    QList<QVariantMap> result;
    QVariantMap inputData;

    //sample data
    //inputData[SnpRequestKeys::SNP_CHIP_TOOLS_SNP_ID] = "11466315";
    //inputData[SnpRequestKeys::SNP_CHIP_TOOLS_UG] = true;
    if (!variant.publicId.startsWith("rs")){
        return result;
    }

    inputData[SnpRequestKeys::SNP_CHIP_TOOLS_SNP_ID] = variant.publicId;
    inputData[SnpRequestKeys::SNP_CHIP_TOOLS_UG] = true;
    
    result.append(inputData);

    return result;
}

QString SnpChipToolsWorker::getRequestingScriptName( ) const
{
    return SnpRequestingScripts::SNP_CHIP_TOOLS_SCRIPT;
}

QList<SnpResponseKey> SnpChipToolsWorker::getResultKeys( ) const
{
    QList<SnpResponseKey> result;
    result << SnpResponseKeys::SNP_CHIP_TOOLS_;
    return result;
}

/************************************************************************/
/* Factory */
/************************************************************************/

SnpChipToolsWorkerFactory::SnpChipToolsWorkerFactory( )
    : DomainFactory( ACTOR_ID )
{

}

void SnpChipToolsWorkerFactory::init( )
{
    //init data path
    U2DataPath* dataPath = NULL;
    U2DataPathRegistry* dpr =  AppContext::getDataPathRegistry();
    if (dpr){
        U2DataPath* dp = dpr->getDataPathByName(BaseRequestForSnpWorker::DB_SEQUENCE_PATH);
        if (dp && dp->isValid()){
            dataPath = dp;
        }
    }    QList<PortDescriptor*> p;
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
    // TODO: revise the description
    Descriptor protoDesc( SnpChipToolsWorkerFactory::ACTOR_ID,
        QObject::tr( "SNP ChIP Tools" ),
        QObject::tr( "SNPs influence on TFBSs" ) );

    ActorPrototype *proto = new IntegralBusActorPrototype( protoDesc, p, a );
    proto->setPrompter( new SnpChipToolsPrompter( ) );
    proto->setEditor( new DelegateEditor( delegates ) );
    WorkflowEnv::getProtoRegistry( )->registerProto( BaseActorCategories::CATEGORY_SCHEMAS( ),
        proto );
    WorkflowEnv::getDomainRegistry( )->getById( LocalDomainFactory::ID )->registerEntry(
        new SnpChipToolsWorkerFactory( ) );
}

Worker *SnpChipToolsWorkerFactory::createWorker( Actor *a )
{
    return new SnpChipToolsWorker( a );
}

/************************************************************************/
/* Prompter */
/************************************************************************/

SnpChipToolsPrompter::SnpChipToolsPrompter( Actor *p )
    : PrompterBase<SnpChipToolsPrompter>( p )
{

}

QString SnpChipToolsPrompter::composeRichDoc( )
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

} // LocalWorkflow

} // U2
