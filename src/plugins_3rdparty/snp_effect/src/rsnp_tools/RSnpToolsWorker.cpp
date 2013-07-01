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

#include "RSnpToolsWorker.h"

namespace U2 {

namespace LocalWorkflow {

const QString RSnpToolsWorkerFactory::ACTOR_ID( "rSnp-tools" );

static const QString DB_PATH("db_path");
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

QVariantMap RSnpToolsWorker::getInputDataForRequest( const U2Variant& variant,
    const U2VariantTrack& track, U2Dbi* dataBase )
{
    QVariantMap inputData;
    qint64 sequenceStart = 0;
    QByteArray seq1 = getSequenceForVariant( variant, track, dataBase, sequenceStart );
    CHECK( !seq1.isEmpty( ), inputData );

    const qint64 mutationPos = variant.startPos - sequenceStart;
    QByteArray seq2 = seq1;
    seq2.replace( mutationPos, 1, variant.obsData );

    //sample data
    //inputData[SnpRequestKeys::R_SNP_FIRST_SEQUENCE] = "cctcagtgctgagggccaagcaaatatttgtggttatggaTtaactcgaactccaggctgtcatggcggcaggacggcgaa";
    //inputData[SnpRequestKeys::R_SNP_SECOND_SEQUENCE] = "cctcagtgctgagggccaagcaaatatttgtggttatggaCtaactcgaactccaggctgtcatggcggcaggacggcgaa";
    //inputData[SnpRequestKeys::R_SNP_FIRST_SITE_STATE] = 1;
    //inputData[SnpRequestKeys::R_SNP_SECOND_SITE_STATE] = 0.5;
    //inputData[SnpRequestKeys::R_SNP_SIGNIFICANCE] = 0.00025;

    inputData[SnpRequestKeys::R_SNP_FIRST_SEQUENCE] = seq1;
    inputData[SnpRequestKeys::R_SNP_SECOND_SEQUENCE] = seq2;

    return inputData;
}

QString RSnpToolsWorker::getRequestingScriptName( ) const
{
    return SnpRequestingScripts::R_SNP_TOOLS_SCRIPT;
}

QString RSnpToolsWorker::getDatabasePath( ) const
{
    return getValue<QString>( DB_PATH );
}

QList<SnpResponseKey> RSnpToolsWorker::getResultKeys( ) const
{
    QList<SnpResponseKey> result;
    result << SnpResponseKeys::R_SNP_PRESENT_TFBS;
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
        Descriptor dbPath( DB_PATH, QObject::tr( "Database path" ),
            QObject::tr( "Path to SNP database." ) );
        Descriptor firstSiteState( FIRST_SITE_STATE, QObject::tr( "First site state" ),
            QObject::tr( "First sequence TFBS state." ) );
        Descriptor secondSiteState( SECOND_SITE_STATE, QObject::tr( "Second site state" ),
            QObject::tr( "Second sequence TFBS state." ) );
        Descriptor snpSignificance( SNP_SIGNIFICANCE, QObject::tr( "SNP significance" ),
            QObject::tr( "Significance value for the SNP." ) );

        a << new Attribute( dbPath, BaseTypes::STRING_TYPE( ), true, QVariant( "" ) );
        a << new Attribute( firstSiteState, BaseTypes::NUM_TYPE( ), true, QVariant( 1 ) );
        a << new Attribute( secondSiteState, BaseTypes::NUM_TYPE( ), true, QVariant( 0.5 ) );
        a << new Attribute( snpSignificance, BaseTypes::NUM_TYPE( ), true, QVariant( 0.00025 ) );
    }

    QMap<QString, PropertyDelegate*> delegates;
    {
        delegates[DB_PATH] = new URLDelegate( "", "", false );
        delegates[FIRST_SITE_STATE] = new ComboBoxDelegate( getAllBindingSiteStates( ) );
        delegates[SECOND_SITE_STATE] = new ComboBoxDelegate( getAllBindingSiteStates( ) );
        QVariantMap snpSignificanceValues;
        snpSignificanceValues["minimum"] = 0.000005;
        snpSignificanceValues["maximum"] = 0.33;
        snpSignificanceValues["singleStep"] = 0.0001;
        snpSignificanceValues["decimals"] = 6;
        delegates[SNP_SIGNIFICANCE] = new DoubleSpinBoxDelegate( snpSignificanceValues );
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

QVariantMap RSnpToolsWorkerFactory::getAllBindingSiteStates( )
{
    static QVariantMap siteStates;
    if ( siteStates.isEmpty( ) ) {
        const QString extendedState = QObject::tr( "Extended (1.5)" );
        const QString normalState = QObject::tr( "Normal (1.0)" );
        const QString weakenedState = QObject::tr( "Weakened (0.5)" );
        const QString undetectableState = QObject::tr( "Undetectable (0.0)" );
        const QString unspecificState = QObject::tr( "Unspecific (-0.5)" );
        const QString noBindingState = QObject::tr( "No binding (-1.0)" );
        siteStates[extendedState] = 1.5;
        siteStates[normalState] = 1.0;
        siteStates[weakenedState] = 0.5;
        siteStates[undetectableState] = 0.0;
        siteStates[unspecificState] = -0.5;
        siteStates[noBindingState] = -1.0;
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
