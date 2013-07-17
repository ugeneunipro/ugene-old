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
#include <U2Core/U2SequenceDbi.h>

#include <U2Lang/ActorPrototypeRegistry.h>
#include <U2Lang/BaseActorCategories.h>
#include <U2Lang/BaseTypes.h>
#include <U2Lang/BaseSlots.h>
#include <U2Lang/BasePorts.h>
#include <U2Lang/WorkflowEnv.h>

#include <U2Formats/S3DatabaseUtils.h>

#include <U2Designer/DelegateEditors.h>

#include "RSnpToolsWorker.h"

namespace U2 {

const int SNP_NEAR_REGION_LENGTH = 40;

namespace LocalWorkflow {

const QString RSnpToolsWorkerFactory::ACTOR_ID( "rSnp-tools" );

static const QString FIRST_SITE_STATE( "first_site_state" );
static const QString SECOND_SITE_STATE( "second_site_state" );
static const QString SNP_SIGNIFICANCE( "snp_significance" );

/************************************************************************/
/* Worker */
/************************************************************************/

RSnpToolsWorker::RSnpToolsWorker( Actor *p )
    : BaseRequestForSnpWorker( p )
{

}

QList<QVariantMap> RSnpToolsWorker::getInputDataForRequest( const U2Variant& variant,
    const U2VariantTrack& track, U2Dbi* dataBase )
{
    QList< QVariantMap > res;
    QVariantMap inputData;
    qint64 sequenceStart = 0;
    QByteArray seq1 = getSequenceForVariant( variant, track, dataBase, sequenceStart );
    CHECK( !seq1.isEmpty( ), res );

    const qint64 mutationPos = variant.startPos - sequenceStart;
    QByteArray seq2 = seq1;
    seq2.replace( mutationPos, 1, variant.obsData );

    //sample data
    //inputData[SnpRequestKeys::R_SNP_FIRST_SEQUENCE] = "cctcagtgctgagggccaagcaaatatttgtggttatggaTtaactcgaactccaggctgtcatggcggcaggacggcgaa";
    //inputData[SnpRequestKeys::R_SNP_SECOND_SEQUENCE] = "cctcagtgctgagggccaagcaaatatttgtggttatggaCtaactcgaactccaggctgtcatggcggcaggacggcgaa";
    //inputData[SnpRequestKeys::R_SNP_FIRST_SITE_STATE] = 1;
    //inputData[SnpRequestKeys::R_SNP_SECOND_SITE_STATE] = 2;
    //inputData[SnpRequestKeys::R_SNP_SIGNIFICANCE] = 10;

    inputData[SnpRequestKeys::R_SNP_FIRST_SEQUENCE] = seq1;
    inputData[SnpRequestKeys::R_SNP_SECOND_SEQUENCE] = seq2;
    inputData[SnpRequestKeys::R_SNP_FIRST_SITE_STATE]
        = actor->getParameter( FIRST_SITE_STATE )->getAttributeValue<int>( context );
    inputData[SnpRequestKeys::R_SNP_SECOND_SITE_STATE]
        = actor->getParameter( SECOND_SITE_STATE )->getAttributeValue<int>( context );
    inputData[SnpRequestKeys::R_SNP_SIGNIFICANCE]
        = actor->getParameter( SNP_SIGNIFICANCE )->getAttributeValue<int>( context );

    res.append(inputData);

    return res;
}

QString RSnpToolsWorker::getRequestingScriptName( ) const
{
    return SnpRequestingScripts::R_SNP_TOOLS_SCRIPT;
}

QList<SnpResponseKey> RSnpToolsWorker::getResultKeys( ) const
{
    QList<SnpResponseKey> result;
    result << SnpResponseKeys::R_SNP_PRESENT_TFBS;
    return result;
}

QByteArray RSnpToolsWorker::getSequenceForVariant( const U2Variant &variant,
                                                          const U2VariantTrack &track, U2Dbi *dataBase, qint64 &sequenceStart ) const
{
    QByteArray result;
    SAFE_POINT( NULL != dataBase, "No database dbi", result );

    U2SequenceDbi* seqDbi = dataBase->getSequenceDbi( );
    SAFE_POINT( NULL != seqDbi, "No sequence dbi", result );

    U2ObjectDbi* objDbi = dataBase->getObjectDbi( );
    SAFE_POINT( NULL != objDbi, "No object dbi", result );

    const U2DataId seqId = track.sequence.isEmpty( ) ?
        S3DatabaseUtils::getSequenceId( track.sequenceName, objDbi ) : track.sequence;

    qint64 start = qMax( ( qint64 )0, variant.startPos - SNP_NEAR_REGION_LENGTH );
    qint64 end = variant.startPos + SNP_NEAR_REGION_LENGTH + 1; //include last char
    U2Region regAround(start, end - start);

    U2OpStatusImpl os;
    result = seqDbi->getSequenceData( seqId, regAround, os );
    CHECK_OP( os, result );
    sequenceStart = start;

    return result;
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

    QList<Attribute*> a;
    {
        Descriptor dbPath( BaseRequestForSnpWorker::DB_SEQUENCE_PATH,
            QObject::tr( "Database path" ), QObject::tr( "Path to SNP database." ) );
        Descriptor firstSiteState( FIRST_SITE_STATE, QObject::tr( "First site state" ),
            QObject::tr( "First sequence TFBS state." ) );
        Descriptor secondSiteState( SECOND_SITE_STATE, QObject::tr( "Second site state" ),
            QObject::tr( "Second sequence TFBS state." ) );
        Descriptor snpSignificance( SNP_SIGNIFICANCE, QObject::tr( "SNP significance" ),
            QObject::tr( "Significance value for the SNP." ) );

        
        a << new Attribute( dbPath, BaseTypes::STRING_TYPE( ), true, QVariant( dataPath != NULL ? dataPath->getPath() : "" ) );
        a << new Attribute( firstSiteState, BaseTypes::NUM_TYPE( ), true, QVariant( 1 ) );
        a << new Attribute( secondSiteState, BaseTypes::NUM_TYPE( ), true, QVariant( 2 ) );
        a << new Attribute( snpSignificance, BaseTypes::NUM_TYPE( ), true, QVariant( 10 ) );
    }

    QMap<QString, PropertyDelegate*> delegates;
    {
        delegates[BaseRequestForSnpWorker::DB_SEQUENCE_PATH] = new URLDelegate( "", "", false, false, false );
        delegates[FIRST_SITE_STATE] = new ComboBoxDelegate( getAllBindingSiteStates( ) );
        delegates[SECOND_SITE_STATE] = new ComboBoxDelegate( getAllBindingSiteStates( ) );
        delegates[SNP_SIGNIFICANCE] = new ComboBoxDelegate( getSignificanceSiteStates() );
    }

    Descriptor protoDesc( RSnpToolsWorkerFactory::ACTOR_ID,
        QObject::tr( "Detect transcription factors with rSNP-Tools" ),
        QObject::tr( "Identification of transcription factor binding sites in the DNA "
            "which have been modified by polymorphic mutation." ) );

    ActorPrototype *proto = new IntegralBusActorPrototype( protoDesc, p, a );
    proto->setPrompter( new RSnpToolsPrompter( ) );
    proto->setEditor( new DelegateEditor( delegates ) );
    WorkflowEnv::getProtoRegistry( )->registerProto( BaseActorCategories::CATEGORY_SCHEMAS( ),
        proto );
    WorkflowEnv::getDomainRegistry( )->getById( LocalDomainFactory::ID )->registerEntry(
        new RSnpToolsWorkerFactory( ) );
}

QVariantMap RSnpToolsWorkerFactory::getAllBindingSiteStates( ){
    static QVariantMap siteStates;
    if ( siteStates.isEmpty( ) ) {
        QString extendedState = QObject::tr( "Extended (1.5)" );
        QString normalState = QObject::tr( "Normal (1.0)" );
        QString weakenedState = QObject::tr( "Weakened (0.5)" );
        QString undetectableState = QObject::tr( "Undetectable (0.0)" );
        QString unspecificState = QObject::tr( "Unspecific (-0.5)" );
        QString noBindingState = QObject::tr( "No binding (-1.0)" );
        siteStates[extendedState] = 0;
        siteStates[normalState] = 1;
        siteStates[weakenedState] = 2;
        siteStates[undetectableState] = 3;
        siteStates[unspecificState] = 4;
        siteStates[noBindingState] = 6;
    }
    return siteStates;
}

QVariantMap RSnpToolsWorkerFactory::getSignificanceSiteStates(){
    static QVariantMap siteStates;
    if ( siteStates.isEmpty( ) ) {
        QString s0 = QObject::tr( "0.33" );
        QString s1 = QObject::tr( "0.25" );
        QString s2 = QObject::tr( "0.1" );
        QString s3 = QObject::tr( "0.05" );
        QString s4 = QObject::tr( "0.025" );
        QString s5 = QObject::tr( "0.01" );
        QString s6 = QObject::tr( "0.005" );
        QString s7 = QObject::tr( "0.0025" );
        QString s8 = QObject::tr( "0.001" );
        QString s9 = QObject::tr( "0.0005" );
        QString s10 = QObject::tr( "0.00025" );
        QString s11 = QObject::tr( "0.0001" );
        QString s12 = QObject::tr( "0.00005" );
        QString s13 = QObject::tr( "0.000025" );
        QString s14 = QObject::tr( "0.00001" );
        QString s15 = QObject::tr( "0.000005" );
        siteStates[s0] = 0;
        siteStates[s1] = 1;
        siteStates[s2] = 2;
        siteStates[s3] = 3;
        siteStates[s4] = 4;
        siteStates[s5] = 5;
        siteStates[s6] = 6;
        siteStates[s7] = 7;
        siteStates[s8] = 8;
        siteStates[s9] = 9;
        siteStates[s10] = 10;
        siteStates[s11] = 11;
        siteStates[s12] = 12;
        siteStates[s13] = 13;
        siteStates[s14] = 14;
        siteStates[s15] = 15;
    }
    return siteStates;
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
