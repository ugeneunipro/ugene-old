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
#include <U2Core/S3TablesUtils.h>
#include <U2Core/Gene.h>
#include <U2Core/VariationPropertiesUtils.h>

#include <U2Formats/S3DatabaseUtils.h>

#include <U2Lang/ActorPrototypeRegistry.h>
#include <U2Lang/BaseActorCategories.h>
#include <U2Lang/BaseTypes.h>
#include <U2Lang/BaseSlots.h>
#include <U2Lang/BasePorts.h>
#include <U2Lang/WorkflowEnv.h>

#include <U2Designer/DelegateEditors.h>

#include "ProtStability3DWorker.h"

namespace U2 {

namespace LocalWorkflow {

const QString ProtStability3DWorkerFactory::ACTOR_ID( "prot-stability-3d" );

static const QString DB_PATH("db_path");

/************************************************************************/
/* Worker */
/************************************************************************/

ProtStability3DWorker::ProtStability3DWorker( Actor *p )
    : BaseRequestForSnpWorker( p )
{

}

QList< QVariantMap > ProtStability3DWorker::getInputDataForRequest( const U2Variant& variant,
    const U2VariantTrack& track, U2Dbi* dataBase )
{
    QList<QVariantMap> res;
    U2ObjectDbi* objDbi = dataBase->getObjectDbi();
    SAFE_POINT(objDbi!=NULL, "No object DBI", res);
    U2FeatureDbi* featureDbi = dataBase->getFeatureDbi();
    SAFE_POINT(featureDbi!=NULL, "No feature DBI", res);

    U2DataId seqId = track.sequence.isEmpty() ? S3DatabaseUtils::getSequenceId(track.sequenceName, objDbi) : track.sequence;
    U2OpStatusImpl os;
    QList<Gene> genes = S3TablesUtils::findGenes(seqId, VARIATION_REGION(variant), featureDbi, os);
    CHECK_OP(os, res);
    foreach(const Gene& gene, genes){
        QVariantMap inputData;

        int aaPos = -1;
        QPair<QByteArray, QByteArray> aaSubs = VariationPropertiesUtils::getAASubstitution(dataBase, gene, seqId, variant, &aaPos, os);
        if (aaPos == -1 || aaSubs.first.isEmpty() || aaSubs.second.isEmpty()){
            continue;
        }

        //sample data
        //inputData[SnpRequestKeys::PROT_STAB_3D_PDB_ID] = "1GZH";
        //inputData[SnpRequestKeys::PROT_STAB_3D_CHAIN] = "A";
        //inputData[SnpRequestKeys::PROT_STAB_3D_MUTATION_POS] = 100;
        //inputData[SnpRequestKeys::PROT_STAB_3D_REPLACEMENT] = "V";

        //WARNING! UNIPROT ID is sent. It is converted to pdb with a script. The scripts defines the chain as well
        inputData[SnpRequestKeys::SNP_FEATURE_ID_KEY] = gene.getFeatureId();
        inputData[SnpRequestKeys::PROT_STAB_3D_PDB_ID] = gene.getAccession();
        inputData[SnpRequestKeys::PROT_STAB_3D_CHAIN] = "A";
        inputData[SnpRequestKeys::PROT_STAB_3D_MUTATION_POS] = aaPos + 1;
        inputData[SnpRequestKeys::PROT_STAB_3D_REPLACEMENT] = aaSubs.second;
        res.append(inputData);
    }

    return res;
}

QString ProtStability3DWorker::getRequestingScriptName( ) const
{
    return SnpRequestingScripts::SNP_PROT_STABILITY_3D_SCRIPT;
}

QString ProtStability3DWorker::getDatabasePath() const
{
    return getValue<QString>( DB_PATH );
}

QList<SnpResponseKey> ProtStability3DWorker::getResultKeys( ) const
{
    QList<SnpResponseKey> result;
    result << SnpResponseKeys::PROT_STABILITY_3D;
    return result;
}

/************************************************************************/
/* Factory */
/************************************************************************/

ProtStability3DWorkerFactory::ProtStability3DWorkerFactory( )
    : DomainFactory( ACTOR_ID )
{

}

void ProtStability3DWorkerFactory::init( )
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

    Descriptor dbPath( DB_PATH, QObject::tr( "Database path" ),
        QObject::tr( "Path to SNP database." ) );
    a << new Attribute( dbPath, BaseTypes::STRING_TYPE( ), true, "" );

    QMap<QString, PropertyDelegate *> delegates;
    {
        delegates[DB_PATH] = new URLDelegate( "", "", false );
    }

    Descriptor protoDesc( ProtStability3DWorkerFactory::ACTOR_ID,
        QObject::tr( "SNP affect on protein tertiary structure thermodynamic stability" ),
        QObject::tr( "Identification of the SNP influence on"
            "protein tertiary structure thermodynamic stability" ) );

    ActorPrototype *proto = new IntegralBusActorPrototype( protoDesc, p, a );
    proto->setPrompter( new ProtStability3DPrompter( ) );
    proto->setEditor( new DelegateEditor( delegates ) );
    WorkflowEnv::getProtoRegistry( )->registerProto( BaseActorCategories::CATEGORY_SCHEMAS( ),
        proto );
    WorkflowEnv::getDomainRegistry( )->getById( LocalDomainFactory::ID )->registerEntry(
        new ProtStability3DWorkerFactory( ) );
}

Worker *ProtStability3DWorkerFactory::createWorker( Actor *a )
{
    return new ProtStability3DWorker( a );
}

/************************************************************************/
/* Prompter */
/************************************************************************/

ProtStability3DPrompter::ProtStability3DPrompter( Actor *p )
    : PrompterBase<ProtStability3DPrompter>( p )
{

}

QString ProtStability3DPrompter::composeRichDoc( )
{
    QString res = ""; 
    Actor* annProducer = qobject_cast<IntegralBusPort*>(
        target->getPort(BasePorts::IN_VARIATION_TRACK_PORT_ID( ) ) )->getProducer(
        BaseSlots::VARIATION_TRACK_SLOT( ).getId( ) );

    QString unsetStr = "<font color='red'>" + tr("unset") + "</font>";
    QString annUrl = annProducer ? annProducer->getLabel( ) : unsetStr;
    QString path = getHyperlink(DB_PATH, getURL(DB_PATH));

    res.append(tr("Uses variations from <u>%1</u> as input.").arg(annUrl));
    res.append(tr(" Takes annotations from <u>%1</u> database.").arg(path.isEmpty() ?
        unsetStr : path));

    return res;
}

} // LocalWorkflow

} // U2
