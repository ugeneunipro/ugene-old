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
#include <U2Core/U2AttributeDbi.h>

#include <U2Formats/S3DatabaseUtils.h>

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

#define SNP_NEAR_REGION_LENGTH 40

QVariantMap RSnpToolsWorker::getInputDataForRequest( const U2Variant& variant, const U2VariantTrack& track, U2Dbi* dataBase ){
    QVariantMap inputData;

    SAFE_POINT(dataBase != NULL, "No database dbi", inputData);

    QString dbPath = getValue<QString>(DB_PATH);

    U2SequenceDbi* seqDbi = dataBase->getSequenceDbi();
    SAFE_POINT(seqDbi != NULL, "No sequence dbi", inputData);

    U2ObjectDbi* objDbi = dataBase->getObjectDbi();
    SAFE_POINT(objDbi != NULL, "No object dbi", inputData);

    U2DataId seqId = track.sequence.isEmpty() ? S3DatabaseUtils::getSequenceId(track.sequenceName, objDbi) : track.sequence;

    qint64 start = qMax((qint64)0, variant.startPos - SNP_NEAR_REGION_LENGTH);
    qint64 end = variant.startPos + SNP_NEAR_REGION_LENGTH + 1; //include last char
    U2Region regAround(start, end - start);

    U2OpStatusImpl os;
    QByteArray seq1 = seqDbi->getSequenceData(seqId, regAround, os);
    CHECK_OP(os, inputData);

    qint64 replPos = variant.startPos - start;
    QByteArray seq2 = seq1;
    seq2.replace(replPos, 1, variant.obsData);

    //sample data
    //inputData[SnpRequestKeys::R_SNP_FIRST_SEQUENCE] = "cctcagtgctgagggccaagcaaatatttgtggttatggaTtaactcgaactccaggctgtcatggcggcaggacggcgaa";
    //inputData[SnpRequestKeys::R_SNP_SECOND_SEQUENCE] = "cctcagtgctgagggccaagcaaatatttgtggttatggaCtaactcgaactccaggctgtcatggcggcaggacggcgaa";

    inputData[SnpRequestKeys::R_SNP_FIRST_SEQUENCE] = seq1;
    inputData[SnpRequestKeys::R_SNP_SECOND_SEQUENCE] = seq2;

    return inputData;
}

QString RSnpToolsWorker::getRequestingScriptName( ) const
{
    return SnpRequestingScripts::R_SNP_TOOLS_SCRIPT;
}

QString RSnpToolsWorker::getDatabasePath() const{
    return getValue<QString>(DB_PATH);
}

void RSnpToolsWorker::handleResult( const U2Variant& variant, const QVariantMap& result, U2Dbi* sessionDbi ){
    SAFE_POINT(sessionDbi != NULL, "no session dbi", );

    U2AttributeDbi* attrDbi = sessionDbi->getAttributeDbi();
    SAFE_POINT(attrDbi != NULL, "no Attribute Dbi", );

    U2OpStatusImpl os;

    if (result.isEmpty()){
        return;
    }

    QString res = "";
    if (result.contains(SnpRequestKeys::R_SNP_PRESENT_TFBS)){
        res = result.value(SnpRequestKeys::R_SNP_PRESENT_TFBS, "").toString();
    }

    if (res.isEmpty()){
        return;
    }
    
    U2StringAttribute resAtr(variant.id, SnpRequestKeys::R_SNP_PRESENT_TFBS, res);
    attrDbi->createStringAttribute(resAtr, os);
    CHECK_OP(os, );

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

    Descriptor dbPath(DB_PATH,
        RSnpToolsWorker::tr("Database path"),
        RSnpToolsWorker::tr("Path to SNP database."));


    a << new Attribute(dbPath, BaseTypes::STRING_TYPE(), true, "");

    QMap<QString, PropertyDelegate*> delegates;
    {
        delegates[DB_PATH] = new URLDelegate("", "", false);
    }

    Descriptor protoDesc( RSnpToolsWorkerFactory::ACTOR_ID,
        QObject::tr( "Detect transcription factors with rSNP-Tools" ),
        QObject::tr( "Identification of transcription factor binding sites in the DNA "
            "which have been modified by polymorphic mutation." ) );

    ActorPrototype *proto = new IntegralBusActorPrototype( protoDesc, p, a );
    proto->setPrompter( new RSnpToolsPrompter( ) );
    proto->setEditor(new DelegateEditor(delegates));
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
    QString path = getHyperlink(DB_PATH, getURL(DB_PATH));

    res.append(tr("Uses variations from <u>%1</u> as input.").arg(annUrl));
    res.append(tr(" Takes annotations from <u>%1</u> database.").arg(path.isEmpty() ?
        unsetStr : path));


    return res;
}

} // LocalWorkflow
} // U2
