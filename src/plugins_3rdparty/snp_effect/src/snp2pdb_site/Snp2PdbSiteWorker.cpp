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
#include <U2Core/Gene.h>
#include <U2Core/SNPTablesUtils.h>
#include <U2Core/VariationPropertiesUtils.h>

#include <U2Formats/SNPDatabaseUtils.h>

#include <U2Lang/ActorPrototypeRegistry.h>
#include <U2Lang/BaseActorCategories.h>
#include <U2Lang/BaseTypes.h>
#include <U2Lang/BaseSlots.h>
#include <U2Lang/BasePorts.h>
#include <U2Lang/WorkflowEnv.h>

#include <U2Designer/DelegateEditors.h>

#include "Snp2PdbSiteWorker.h"

namespace U2 {

namespace LocalWorkflow {

const QString Snp2PdbSiteWorkerFactory::ACTOR_ID( "snp2pdb-site" );

/************************************************************************/
/* Worker */
/************************************************************************/

Snp2PdbSiteWorker::Snp2PdbSiteWorker( Actor *p )
    : BaseRequestForSnpWorker( p )
{

}

QList<QVariantMap> Snp2PdbSiteWorker::getInputDataForRequest( const U2Variant& variant,
    const U2VariantTrack& track, U2Dbi* dataBase )
{
    QList<QVariantMap> res;
    U2ObjectDbi* objDbi = dataBase->getObjectDbi();
    SAFE_POINT(objDbi!=NULL, "No object DBI", res);
    U2FeatureDbi* featureDbi = dataBase->getFeatureDbi();
    SAFE_POINT(featureDbi!=NULL, "No feature DBI", res);

    U2DataId seqId = track.sequence.isEmpty() ? SNPDatabaseUtils::getSequenceId(track.sequenceName, objDbi) : track.sequence;
    U2OpStatusImpl os;
    QList<Gene> genes = SNPTablesUtils::findGenes(seqId, VARIATION_REGION(variant), featureDbi, os);
    CHECK_OP(os, res);
    foreach(const Gene& gene, genes){
        QVariantMap inputData;

        int aaPos = -1;
        QPair<QByteArray, QByteArray> aaSubs = VariationPropertiesUtils::getAASubstitution(dataBase, gene, seqId, variant, &aaPos, os);
        if (aaPos == -1 || aaSubs.first.isEmpty() || aaSubs.second.isEmpty() || gene.getAccession().isEmpty()){
            continue;
        }
        //sample data
        //inputData[SnpRequestKeys::SNP_2_PDB_SITE_PDB_ID] = "1GZH";
        //inputData[SnpRequestKeys::SNP_2_PDB_SITE_CHAIN] = "A";
        //inputData[SnpRequestKeys::SNP_2_PDB_SITE_MUTATIONS] = "A 110       R -> C";

        //WARNING! UNIPROT ID is sent. It is converted to pdb with a script. The scripts defines the chain as well
        inputData[SnpRequestKeys::SNP_FEATURE_ID_KEY] = gene.getFeatureId();
        inputData[SnpRequestKeys::SNP_2_PDB_SITE_PDB_ID] = gene.getAccession();
        inputData[SnpRequestKeys::SNP_2_PDB_SITE_CHAIN] = "A";
        // <`pdb_chain` `snp_position`       `source_aminoacid` -> `replacing_aminoacid`>
        inputData[SnpRequestKeys::SNP_2_PDB_SITE_MUTATIONS] = QString("\"A %1       %2 -> %3\"").arg(aaPos+1).arg(QString::fromLatin1(aaSubs.first)).arg(QString::fromLatin1(aaSubs.second));

        res.append(inputData);
    }
    return res;
}

QString Snp2PdbSiteWorker::getRequestingScriptName( ) const
{
    return SnpRequestingScripts::SNP_2_PDB_SITE_SCRIPT;
}

QList<SnpResponseKey> Snp2PdbSiteWorker::getResultKeys( ) const
{
    QList<SnpResponseKey> result;
    // TODO: add other keys
    result << SnpResponseKeys::SNP_2_PDB_SITE_;
    return result;
}

/************************************************************************/
/* Factory */
/************************************************************************/

Snp2PdbSiteWorkerFactory::Snp2PdbSiteWorkerFactory( )
    : DomainFactory( ACTOR_ID )
{

}

void Snp2PdbSiteWorkerFactory::init( )
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

    Descriptor protoDesc( Snp2PdbSiteWorkerFactory::ACTOR_ID,
        QObject::tr( "SNP Effect on PDB Sites" ),
        QObject::tr( "Identification of the SNP influence on PDB sites." ) );

    ActorPrototype *proto = new IntegralBusActorPrototype( protoDesc, p, a );
    proto->setPrompter( new Snp2PdbSitePrompter( ) );
    proto->setEditor( new DelegateEditor( delegates ) );
    WorkflowEnv::getProtoRegistry( )->registerProto( BaseActorCategories::CATEGORY_SNP_ANNOTATION( ),
        proto );
    WorkflowEnv::getDomainRegistry( )->getById( LocalDomainFactory::ID )->registerEntry(
        new Snp2PdbSiteWorkerFactory( ) );
}

Worker *Snp2PdbSiteWorkerFactory::createWorker( Actor *a )
{
    return new Snp2PdbSiteWorker( a );
}

/************************************************************************/
/* Prompter */
/************************************************************************/

Snp2PdbSitePrompter::Snp2PdbSitePrompter( Actor *p )
    : PrompterBase<Snp2PdbSitePrompter>( p )
{

}

QString Snp2PdbSitePrompter::composeRichDoc( )
{
    QString res = ""; 
    Actor* annProducer = qobject_cast<IntegralBusPort*>(
        target->getPort(BasePorts::IN_VARIATION_TRACK_PORT_ID( ) ) )->getProducer(
        BaseSlots::VARIATION_TRACK_SLOT( ).getId( ) );

    QString unsetStr = "<font color='red'>" + tr( "unset" ) + "</font>";
    QString annUrl = annProducer ? annProducer->getLabel( ) : unsetStr;
    QString path = getHyperlink( BaseRequestForSnpWorker::DB_SEQUENCE_PATH,
        getURL(BaseRequestForSnpWorker::DB_SEQUENCE_PATH ) );

    res.append( tr( "Uses variations from <u>%1</u> as input." ).arg( annUrl ) );
    res.append( tr( " Takes sequences from <u>%1</u> database." ).arg(path.isEmpty( ) ?
        unsetStr : path ) );

    return res;
}

} // LocalWorkflow

} // U2
