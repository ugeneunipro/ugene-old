/**
 * UGENE - Integrated Bioinformatics Tools.
 * Copyright (C) 2008-2014 UniPro <ugene@unipro.ru>
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

#include <QtGui/QTextDocument>

#include <U2Core/AnnotationModification.h>
#include <U2Core/DNASequenceObject.h>
#include <U2Core/DNATranslation.h>
#include <U2Core/AnnotationTableObject.h>
#include <U2Core/TextUtils.h>
#include <U2Core/U2Assembly.h>
#include <U2Core/U2FeatureKeys.h>
#include <U2Core/U2FeatureUtils.h>
#include <U2Core/U2OpStatusUtils.h>
#include <U2Core/U2SafePoints.h>

#include "Annotation.h"

namespace U2 {

Annotation::Annotation( const U2DataId &_featureId,AnnotationTableObject *_parentObject )
    : DbiIdBasedData( _featureId ), parentObject( _parentObject )
{
    SAFE_POINT( NULL != parentObject && !dbId.isEmpty( ),
        "Invalid feature table detected!", );
}

Annotation::~Annotation( ) {

}

AnnotationTableObject * Annotation::getGObject( ) const {
    return parentObject;
}

AnnotationData Annotation::getData( ) const {
    U2OpStatusImpl os;
    const AnnotationData result = U2FeatureUtils::getAnnotationDataFromFeature( dbId,
        parentObject->getEntityRef( ).dbiRef, os );
    SAFE_POINT_OP( os, result );
    return result;
}

QString Annotation::getName( ) const {
    U2OpStatusImpl os;
    const U2Feature feature = U2FeatureUtils::getFeatureById( dbId,
        parentObject->getEntityRef( ).dbiRef, os );
    SAFE_POINT_OP( os, QString( ) );
    return feature.name;
}

void Annotation::setName( const QString &name ) {
    SAFE_POINT( !name.isEmpty( ), "Attempting to set an empty name for an annotation!", );
    U2OpStatusImpl os;
    U2FeatureUtils::updateFeatureName( dbId, name, parentObject->getEntityRef( ).dbiRef, os );
    SAFE_POINT_OP( os, );

    parentObject->setModified( true );
    AnnotationModification md( AnnotationModification_NameChanged, *this );
    parentObject->emit_onAnnotationModified( md );
}

bool Annotation::isOrder( ) const {
    U2OpStatusImpl os;
    const AnnotationData data = U2FeatureUtils::getAnnotationDataFromFeature( dbId,
        parentObject->getEntityRef( ).dbiRef, os );
    SAFE_POINT_OP( os, false );
    return data.isOrder( );
}

bool Annotation::isJoin( ) const {
    U2OpStatusImpl os;
    const AnnotationData data = U2FeatureUtils::getAnnotationDataFromFeature( dbId,
        parentObject->getEntityRef( ).dbiRef, os );
    SAFE_POINT_OP( os, false );
    return data.isJoin( );
}

U2Strand Annotation::getStrand( ) const {
    // use only root feature to determine the strand
    U2OpStatusImpl os;
    const U2Feature feature = U2FeatureUtils::getFeatureById( dbId,
        parentObject->getEntityRef( ).dbiRef, os );
    SAFE_POINT_OP( os, U2Strand( ) );
    return feature.location.strand;
}

void Annotation::setStrand( const U2Strand &strand ) {
    U2OpStatusImpl os;
    const AnnotationData data = U2FeatureUtils::getAnnotationDataFromFeature( dbId,
        parentObject->getEntityRef( ).dbiRef, os );
    SAFE_POINT_OP( os, );

    U2Location location = data.location;
    if ( strand != location->strand ) {
        location->strand = strand;
        U2FeatureUtils::updateFeatureLocation( dbId, location,
            parentObject->getEntityRef( ).dbiRef, os );
        SAFE_POINT_OP( os, );

        parentObject->setModified( true );
        AnnotationModification md( AnnotationModification_LocationChanged, *this );
        parentObject->emit_onAnnotationModified( md );
    }
}

U2LocationOperator Annotation::getLocationOperator( ) const {
    U2OpStatusImpl os;
    const AnnotationData data = U2FeatureUtils::getAnnotationDataFromFeature( dbId,
        parentObject->getEntityRef( ).dbiRef, os );
    SAFE_POINT_OP( os, data.location->op );
    return data.location->op;
}

void Annotation::setLocationOperator( U2LocationOperator op ) {
    U2OpStatusImpl os;
    const AnnotationData data = U2FeatureUtils::getAnnotationDataFromFeature( dbId,
        parentObject->getEntityRef( ).dbiRef, os );
    SAFE_POINT_OP( os, );

    U2Location location = data.location;
    if ( op != location->op ) {
        location->op = op;
        U2FeatureUtils::updateFeatureLocation( dbId, location,
            parentObject->getEntityRef( ).dbiRef, os );
        SAFE_POINT_OP( os, );

        parentObject->setModified( true );
        AnnotationModification md( AnnotationModification_LocationChanged, *this );
        parentObject->emit_onAnnotationModified( md );
    }
}

U2Location Annotation::getLocation( ) const {
    U2OpStatusImpl os;
    const AnnotationData data = U2FeatureUtils::getAnnotationDataFromFeature( dbId,
        parentObject->getEntityRef( ).dbiRef, os );
    SAFE_POINT_OP( os, data.location );
    return data.location;
}

void Annotation::setLocation( const U2Location &location ) {
    U2OpStatusImpl os;
    U2FeatureUtils::updateFeatureLocation( dbId, location,
        parentObject->getEntityRef( ).dbiRef, os );
    SAFE_POINT_OP( os, );

    parentObject->setModified( true );
    AnnotationModification md( AnnotationModification_LocationChanged, *this );
    parentObject->emit_onAnnotationModified( md );
}

QVector<U2Region> Annotation::getRegions( ) const {
    return getLocation( )->regions;
}

void Annotation::updateRegions(const QVector<U2Region> &regions ) {
    SAFE_POINT( !regions.isEmpty( ), "Attempting to assign the annotation to an empty region!", );
    U2OpStatusImpl os;
    const AnnotationData data = U2FeatureUtils::getAnnotationDataFromFeature( dbId,
        parentObject->getEntityRef( ).dbiRef, os );
    SAFE_POINT_OP( os, );

    U2Location location = data.location;
    if ( regions != location->regions ) {
        location->regions = regions;
        U2FeatureUtils::updateFeatureLocation( dbId, location,
            parentObject->getEntityRef( ).dbiRef, os );
        SAFE_POINT_OP( os, );

        parentObject->setModified( true );
        AnnotationModification md( AnnotationModification_LocationChanged, *this );
        parentObject->emit_onAnnotationModified( md );
    }
}

void Annotation::addLocationRegion( const U2Region &reg ) {
    SAFE_POINT( !reg.isEmpty( ), "Attempting to annotate an empty region!", );
    U2OpStatusImpl os;
    const AnnotationData data = U2FeatureUtils::getAnnotationDataFromFeature( dbId,
        parentObject->getEntityRef( ).dbiRef, os );
    SAFE_POINT_OP( os, );

    U2Location location = data.location;
    if ( !location->regions.contains( reg ) ) {
        location->regions << reg;
        U2FeatureUtils::updateFeatureLocation( dbId, location,
            parentObject->getEntityRef( ).dbiRef, os );
        SAFE_POINT_OP( os, );

        parentObject->setModified( true );
        AnnotationModification md( AnnotationModification_LocationChanged, *this );
        parentObject->emit_onAnnotationModified( md );
    }
}

QVector<U2Qualifier> Annotation::getQualifiers( ) const {
    U2OpStatusImpl os;
    QList<U2FeatureKey> keys = U2FeatureUtils::getFeatureKeys( dbId,
        parentObject->getEntityRef( ).dbiRef, os );

    for ( int i = 0; i < keys.size( ); ++i ) {
        if ( U2FeatureKeyOperation == keys[i].name ) {
            keys.removeAt( i );
        }
    }

    QVector<U2Qualifier> result;
    foreach ( const U2FeatureKey &key, keys ) {
        result << U2Qualifier( key.name, key.value );
    }
    return result;
}

void Annotation::addQualifier( const U2Qualifier &q ) {
    SAFE_POINT( q.isValid( ), "Invalid annotation qualifier detected!", );
    U2OpStatusImpl os;
    U2FeatureUtils::addFeatureKey( dbId, U2FeatureKey( q.name, q.value ),
        parentObject->getEntityRef( ).dbiRef, os );
    SAFE_POINT_OP( os, );

    parentObject->setModified( true );
    QualifierModification md( AnnotationModification_QualifierAdded, *this, q );
    parentObject->emit_onAnnotationModified( md );
}

void Annotation::removeQualifier( const U2Qualifier &q ) {
    SAFE_POINT( q.isValid( ), "Invalid annotation qualifier detected!", );
    U2OpStatusImpl os;
    U2FeatureUtils::removeFeatureKey( dbId, U2FeatureKey( q.name, q.value ),
        parentObject->getEntityRef( ).dbiRef, os );
    SAFE_POINT_OP( os, );

    parentObject->setModified( true );
    QualifierModification md( AnnotationModification_QualifierRemoved, *this, q );
    parentObject->emit_onAnnotationModified( md );
}

bool Annotation::isCaseAnnotation( ) const {
    U2OpStatusImpl os;
    const bool cased = U2FeatureUtils::isCaseAnnotation( dbId,
        parentObject->getEntityRef( ).dbiRef, os );
    SAFE_POINT_OP( os, false );
    return cased;
}

void Annotation::setCaseAnnotation( bool caseAnnotation ) {
    U2OpStatusImpl os;
    const bool cased = isCaseAnnotation( );
    if ( caseAnnotation && !cased ) {
        U2FeatureUtils::addFeatureKey( dbId, U2FeatureKey( U2FeatureKeyCase, QString( ) ),
            parentObject->getEntityRef( ).dbiRef, os );
    } else if ( !caseAnnotation && cased ) {
        U2FeatureUtils::removeFeatureKey( dbId, U2FeatureKey( U2FeatureKeyCase, QString( ) ),
            parentObject->getEntityRef( ).dbiRef, os );
    }
    SAFE_POINT_OP( os, );
}

AnnotationGroup Annotation::getGroup( ) const {
    U2OpStatusImpl os;
    const U2Feature feature = U2FeatureUtils::getFeatureById( dbId,
        parentObject->getEntityRef( ).dbiRef, os );

    AnnotationGroup result( feature.parentFeatureId, parentObject );
    SAFE_POINT_OP( os, result );
    SAFE_POINT( !feature.parentFeatureId.isEmpty( ), "Invalid annotation detected!", result );
    return result;
}

void Annotation::findQualifiers( const QString &name, QList<U2Qualifier> &res ) const {
    SAFE_POINT( !name.isEmpty( ), "Attempting to find a qualifier having an empty name!", );
    U2OpStatusImpl os;
    const AnnotationData data = U2FeatureUtils::getAnnotationDataFromFeature( dbId,
        parentObject->getEntityRef( ).dbiRef, os );
    SAFE_POINT_OP( os, );

    foreach ( const U2Qualifier &qual, data.qualifiers ) {
        if ( name == qual.name ) {
            res << qual;
        }
    }
}

QString Annotation::findFirstQualifierValue( const QString &name ) const {
    SAFE_POINT( !name.isEmpty( ), "Attempting to find a qualifier having an empty name!",
        QString::null );
    U2OpStatusImpl os;
    const AnnotationData data = U2FeatureUtils::getAnnotationDataFromFeature( dbId,
        parentObject->getEntityRef( ).dbiRef, os );
    SAFE_POINT_OP( os, QString::null );

    foreach ( const U2Qualifier &qual, data.qualifiers ) {
        if ( name == qual.name ) {
            return qual.value;
        }
    }
    return QString::null;
}

bool Annotation::annotationLessThan( const Annotation &first, const Annotation &second ) {
    U2OpStatusImpl os;

    const U2Feature firstFeature = U2FeatureUtils::getFeatureById( first.getId( ),
        first.getGObject( )->getEntityRef( ).dbiRef, os );
    SAFE_POINT_OP( os, false );
    const U2Feature firstFeatureGroup = U2FeatureUtils::getFeatureById(
        firstFeature.parentFeatureId, first.getGObject( )->getEntityRef( ).dbiRef, os );
    SAFE_POINT_OP( os, false );

    const U2Feature secondFeature = U2FeatureUtils::getFeatureById( second.getId( ),
        second.getGObject( )->getEntityRef( ).dbiRef, os );
    SAFE_POINT_OP( os, false );
    const U2Feature secondFeatureGroup = U2FeatureUtils::getFeatureById(
        secondFeature.parentFeatureId, second.getGObject( )->getEntityRef( ).dbiRef, os );
    SAFE_POINT_OP( os, false );

    return ( firstFeatureGroup.name < secondFeatureGroup.name );
}

bool Annotation::annotationLessThanByRegion( const Annotation &first,
    const Annotation &second )
{
    SAFE_POINT( !first.getLocation( )->isEmpty( ) || !second.getLocation( )->isEmpty( ),
        "Invalid annotation's location detected!", false );

    const U2Region &r1 = first.getRegions( ).first( );
    const U2Region &r2 = second.getRegions( ).first( );
    return r1 < r2;
}

bool Annotation::isValidQualifierName( const QString &n ) {
    return !n.isEmpty( ) && 20 > n.length( )
        && TextUtils::fits( TextUtils::QUALIFIER_NAME_CHARS, n.toLocal8Bit( ).data( ), n.length( ) );
}

bool Annotation::isValidQualifierValue( const QString & /*v*/ ) {
    // todo: check whitespaces!
    return true;
}

bool Annotation::isValidAnnotationName( const QString &n ) {
    if ( n.isEmpty( ) || 100 < n.length( ) ) {
        return false;
    }

    QBitArray validChars = TextUtils::ALPHA_NUMS;
    validChars['_'] = true;
    validChars['-'] = true;
    validChars[' '] = true;
    validChars['\''] = true;
    validChars['*']  = true;

    QByteArray name = n.toLocal8Bit( );
    if ( !TextUtils::fits( validChars, name.constData( ), name.size( ) ) ) {
        return false;
    }
    if ( ' ' == name[0] || ' ' == name[name.size( ) - 1] ) {
        return false;
    }
    return true;
}

static QList<U2CigarToken> parceCigar( const QString &cigar) {
    QList<U2CigarToken> cigarTokens;

    QRegExp rx( "(\\d+)(\\w)" );

    int pos = 0;
    while ( -1 != ( pos = rx.indexIn( cigar, pos ) ) ) {
        if ( 2 != rx.captureCount( ) ) {
            break;
        }
        int count = rx.cap( 1 ).toInt( );
        QString cigarChar = rx.cap( 2 );

        if ( cigarChar == "M" ) {
            cigarTokens.append( U2CigarToken( U2CigarOp_M, count ) );
        } else if ( cigarChar == "I") {
            cigarTokens.append( U2CigarToken( U2CigarOp_I, count ) );
        } else if ( cigarChar == "D" ) {
            cigarTokens.append( U2CigarToken( U2CigarOp_D, count ) );
        } else if ( cigarChar == "X" ) {
            cigarTokens.append( U2CigarToken( U2CigarOp_X, count ) );
        } else {
            break;
        }

        pos += rx.matchedLength( );
    }

    return cigarTokens;
}

static QString getAlignmentTip( const QString &ref, const QList<U2CigarToken> &tokens,
    int maxVisibleSymbols )
{
    QString alignmentTip;

    if ( tokens.isEmpty( ) ) {
        return ref;
    }

    int pos = 0;

    QList<int> mismatchPositions;

    foreach ( const U2CigarToken &t, tokens ) {
        if ( U2CigarOp_M == t.op ) {
            alignmentTip += ref.mid( pos, t.count );
            pos += t.count;
        } else if ( t.op == U2CigarOp_X ) {
            alignmentTip += ref.mid( pos, t.count );
            mismatchPositions.append( pos );
            pos += t.count;
        } else if ( U2CigarOp_I == t.op ) {
            // gap already present in sequence?
            pos += t.count;
        }
    }

    if ( maxVisibleSymbols < alignmentTip.length( ) ) {
        alignmentTip = alignmentTip.left( maxVisibleSymbols );
        alignmentTip += " ... ";
    }

    // make mismatches bold
    int offset = 0;
    static const int OFFSET_LEN = QString( "<b></b>" ).length( );
    foreach ( int pos, mismatchPositions ) {
        int newPos = pos + offset;
        if ( newPos + 1 >= alignmentTip.length( ) ) {
            break;
        }
        alignmentTip.replace( newPos, 1,  QString( "<b>%1</b>" ).arg( alignmentTip.at( newPos ) ) );
        offset += OFFSET_LEN;
    }

    return alignmentTip;
}

QString Annotation::getQualifiersTip( int maxRows, U2SequenceObject *seqObj,
    DNATranslation *complTT, DNATranslation *aminoTT ) const
{
    SAFE_POINT( 0 < maxRows && NULL != seqObj, "Invalid parameters passed!", QString( ) );
    QString tip;

    U2OpStatusImpl os;
    const AnnotationData data = U2FeatureUtils::getAnnotationDataFromFeature( dbId,
        parentObject->getEntityRef( ).dbiRef, os );
    SAFE_POINT_OP( os, tip );

    int rows = 0;
    const int QUALIFIER_VALUE_CUT = 40;

    QString cigar, ref;
    if ( !data.qualifiers.isEmpty( ) ) {
        tip += "<nobr>";
        bool first = true;
        foreach ( const U2Qualifier &q, data.qualifiers ) {
            if ( ++rows > maxRows ) {
                break;
            }
            if ( q.name == QUALIFIER_NAME_CIGAR ) {
                cigar = q.value;
            } else if ( q.name == QUALIFIER_NAME_SUBJECT ) {
                ref = q.value;
                continue;
            }
            QString val = q.value;
            if ( val.length( ) > QUALIFIER_VALUE_CUT ) {
                val = val.left( QUALIFIER_VALUE_CUT ) + " ...";
            }
            if ( first ) {
                first = false;
            } else {
                tip += "<br>";
            }
#if ( QT_VERSION >= 0x050000 )
            tip += "<b>" + q.name.toHtmlEscaped( ) + "</b> = " + val.toHtmlEscaped( );
#else
            tip += "<b>" + Qt::escape( q.name ) + "</b> = " + Qt::escape( val );
#endif
        }
        tip += "</nobr>";
    }

    if ( !cigar.isEmpty( ) && !ref.isEmpty( ) ) {
        QList<U2CigarToken> tokens = parceCigar( cigar );
        QString alignmentTip = getAlignmentTip( ref, tokens, QUALIFIER_VALUE_CUT );
        tip += "<br><b>Reference</b> = " + alignmentTip;
        rows++;
    }

    bool canShowSeq = true;
    int seqLen = ( NULL != seqObj ) ? seqObj->getSequenceLength( ) : 0;
    foreach ( const U2Region &r, data.location->regions ) {
        if ( r.endPos( ) > seqLen ) {
            canShowSeq = false;
        }
    }

    if ( seqObj && rows <= maxRows && ( data.location->strand.isCompementary( ) || complTT != NULL )
        && canShowSeq )
    {
        QVector<U2Region> loc = data.location->regions;
        if ( data.location->strand.isCompementary( ) ) {
            qStableSort( loc.begin( ), loc.end( ), qGreater<U2Region>( ) );
        }
        QString seqVal;
        QString aminoVal;
        bool complete = true;
        for ( int i = 0; i < loc.size( ); i++ ) {
            if ( !seqVal.isEmpty( ) ) {
                seqVal += "^";
            }
            if ( !aminoVal.isEmpty( ) ) {
                aminoVal += "^";
            }
            const U2Region& r = loc.at( i );
            const int len = qMin( int( r.length ), QUALIFIER_VALUE_CUT - seqVal.length( ) );
            if ( len != r.length ) {
                complete = false;
            }
            if ( data.location->strand.isCompementary() && NULL != complTT ) {
                QByteArray ba = seqObj->getSequenceData( U2Region( r.endPos( ) - len, len ) );
                complTT->translate( ba.data( ), len );
                TextUtils::reverse( ba.data( ), len );
                seqVal += QString::fromLocal8Bit( ba.data( ), len );
                if ( NULL != aminoTT ) {
                    int aminoLen = aminoTT->translate( ba.data( ), len );
                    aminoVal += QString::fromLocal8Bit( ba.data( ), aminoLen );
                }
            } else {
                QByteArray ba = seqObj->getSequenceData( U2Region( r.startPos, len ) );
                seqVal += QString::fromLocal8Bit( ba.constData( ), len );
                if ( NULL != aminoTT ) {
                    int aminoLen = aminoTT->translate( ba.data( ), len );
                    aminoVal += QString::fromLocal8Bit( ba.data( ), aminoLen );
                }
            }
            if ( seqVal.length( ) >= QUALIFIER_VALUE_CUT ) {
                complete &= ( i == loc.size( ) - 1 );
                break;
            }
        }
        if ( !complete || seqVal.length( ) > QUALIFIER_VALUE_CUT ) {
            seqVal = seqVal.left( QUALIFIER_VALUE_CUT ) + " ...";
        }
        if ( !complete || aminoVal.length( ) > QUALIFIER_VALUE_CUT ) {
            aminoVal = aminoVal.left( QUALIFIER_VALUE_CUT ) + " ...";
        }
        if ( !tip.isEmpty( ) ) {
            tip += "<br>";
        }
        SAFE_POINT( !seqVal.isEmpty( ), "Empty sequence detected!", QString( ) );
#if ( QT_VERSION >= 0x050000 )
        tip += "<nobr><b>" + QObject::tr( "Sequence" ) + "</b> = " + seqVal.toHtmlEscaped( )
            + "</nobr>";
#else
        tip += "<nobr><b>" + QObject::tr( "Sequence" ) + "</b> = " + Qt::escape( seqVal )
            + "</nobr>";
#endif
        rows++;

        if ( rows <= maxRows && NULL != aminoTT ) {
            tip += "<br>";
#if ( QT_VERSION >= 0x050000 )
            tip += "<nobr><b>" + QObject::tr( "Translation" ) + "</b> = "
                + aminoVal.toHtmlEscaped( ) + "</nobr>";
#else
            tip += "<nobr><b>" + QObject::tr("Translation") + "</b> = " + Qt::escape(aminoVal)
                + "</nobr>";
#endif
        }
    }
    return tip;
}

} // namespace U2
