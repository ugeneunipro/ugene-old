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

#include <U2Core/AppContext.h>
#include <U2Core/GObjectRelationRoles.h>
#include <U2Core/GObjectUtils.h>
#include <U2Core/AnnotationTableObject.h>
#include <U2Core/TextUtils.h>
#include <U2Core/DNASequenceObject.h>
#include <U2Core/DNATranslation.h>
#include <U2Core/AppContext.h>
#include <U2Core/U2AlphabetUtils.h>
#include <U2Core/U2DbiRegistry.h>
#include <U2Core/U2OpStatusUtils.h>

#include "U1AnnotationUtils.h"

namespace U2 {

QString U1AnnotationUtils::lowerCaseAnnotationName( "lower_case" );
QString U1AnnotationUtils::upperCaseAnnotationName( "upper_case" );

AnnotatedRegion::AnnotatedRegion()
    : annotation( Annotation(U2DataId(), NULL)),
      regionIdx(-1){}

AnnotatedRegion::AnnotatedRegion(const Annotation &annotation, int regionIdx)
    : annotation(annotation),
      regionIdx(regionIdx){}

AnnotatedRegion::AnnotatedRegion(const AnnotatedRegion &annRegion)
    : annotation( annRegion.annotation ),
      regionIdx( annRegion.regionIdx ) {}

QList<QVector<U2Region> > U1AnnotationUtils::fixLocationsForReplacedRegion(
    const U2Region &region2Remove, qint64 region2InsertLength, const QVector<U2Region> &original,
    AnnotationStrategyForResize s )
{
    QList<QVector<U2Region> > res;
    const qint64 dLen = region2InsertLength - region2Remove.length;
    if ( AnnotationStrategyForResize_Resize == s ) {
        if ( region2Remove.length == region2InsertLength ) {
            res << original;
            return res;
        }
    }
    res << QVector<U2Region>( );
    QVector<U2Region> &updated =  res[0];

    foreach ( U2Region r, original ) {
        //if location ends before modification
        if ( r.endPos( ) <= region2Remove.startPos ) {
            updated << r;
            continue;
        }
        // if location starts after the modification
        if ( r.startPos >= region2Remove.endPos( ) ) { 
            r.startPos += dLen;
            updated << r;
            continue;
        }
        if ( AnnotationStrategyForResize_Remove == s ) {
            continue;
        } else if ( AnnotationStrategyForResize_Resize == s ) {
            // if location contains modified region -> update it length
            if ( r.contains( region2Remove ) ) {
                // if set A = set B - do nothing
                if ( !( r.startPos == region2Remove.startPos
                    && r.endPos( ) == region2Remove.endPos( ) ) )
                {
                    r.length += dLen;
                    updated << r;
                }
            }
            // if location partly contain (in the end) region2remove - update length
            else if( r.contains( U2Region( region2Remove.startPos, 0 ) ) ) {
                if ( dLen < 0 ) {
                    r.length -= ( r.endPos( ) - region2Remove.startPos );
                }
                updated << r;
            }
            else if ( r.contains( U2Region( region2Remove.endPos( ),0 ) ) ) {
                if ( dLen < 0 ) {
                    int diff = region2Remove.endPos( ) - r.startPos;
                    r.startPos += diff + dLen;
                    r.length -= diff;
                }
                updated << r;
            }
            continue;
        }
        SAFE_POINT( AnnotationStrategyForResize_Split_To_Joined == s
            || AnnotationStrategyForResize_Split_To_Separate == s,
            "Unexpected resize strategy detected!", res );
        //leave left part in original(updated) locations and push right into new one
        const bool join = ( AnnotationStrategyForResize_Split_To_Joined == s );
        const U2Region interR = r.intersect( region2Remove );
        const U2Region leftR = r.startPos < interR.startPos
            ? U2Region( r.startPos, interR.startPos - r.startPos ) : U2Region( );
        const U2Region rightR = ( r.endPos( ) > interR.endPos( ) )
            ? U2Region( interR.endPos( ) + dLen, r.endPos( ) - interR.endPos( ) ) : U2Region( );
        if ( leftR.isEmpty( ) ) {
            if ( !rightR.isEmpty( ) ) {
                updated << rightR;
            }
            continue;
        }
        updated << leftR;
        if ( !rightR.isEmpty( ) ) {
            if ( join ) {
                updated << rightR;
            } else {
                QVector<U2Region> extraAnnReg;
                extraAnnReg << rightR;
                res << extraAnnReg;
            }
        }
    }
    return res;
}

int U1AnnotationUtils::getRegionFrame( int sequenceLen, const U2Strand &strand, bool order,
    int region, const QVector<U2Region> &location )
{
    int frame = 0;
    const U2Region &r = location.at( region );
    if ( strand.isCompementary( ) ) {
        frame = ( sequenceLen - r.endPos( ) ) % 3;
    } else {
        frame = r.startPos % 3;
    }
    if ( !order ) { //join -> need to join region with prev regions to derive frame
        if ( strand.isCompementary( ) ) {
            int offset = 0;
            for ( int i = location.size( ); --i > region; ) {
                const U2Region& rb = location.at( i );
                offset += rb.length;
            }
            int dFrame = offset % 3;
            frame = ( frame + ( 3 - dFrame ) ) % 3;
        } else {
            int offset = 0;
            for ( int i = 0; i < region; i++ ) {
                const U2Region &rb = location.at( i );
                offset += rb.length;
            }
            int dFrame = offset % 3;
            frame = ( frame + ( 3 - dFrame ) ) % 3;
        }
    }
    return frame;
}

bool U1AnnotationUtils::isSplitted( const U2Location &location, const U2Region &seqRange ) {
    QVector<U2Region> regions = location->regions;
    if ( 2 != regions.size( ) ) {
        return false;
    } else if ( regions[0].endPos( ) == seqRange.endPos( )
        && regions[1].startPos == seqRange.startPos )
    {
        return true; 
    }

    return false;
}

bool findCaseRegion( const char *data, int dataLen, int startFrom, int globalOffset, U2Region &reg,
    bool &unfinished, bool isLowerCaseSearching )
{
    bool foundStart = false;
    int strIdx = startFrom;
    unfinished = false;

    // find start of lower case block
    while ( !foundStart && strIdx < dataLen ) {
        QChar c( data[strIdx] );
        if ( isLowerCaseSearching ) {
            foundStart = c.isLower( );
        } else {
            foundStart = c.isUpper( );
        }
        strIdx++;
    }

    if ( foundStart ) {
        int startPos = globalOffset + strIdx - 1;

        // find end of lower case block
        bool foundEnd = false;
        while ( !foundEnd && strIdx < dataLen ) {
            QChar c( data[strIdx] );
            if ( isLowerCaseSearching ) {
                foundEnd = c.isUpper( );
            } else {
                foundEnd = c.isLower( );
            }
            strIdx++;
        }
        int endPos = 0;
        if ( foundEnd ) {
            endPos = globalOffset + strIdx - 2;
        } else {
            endPos = globalOffset + strIdx - 1;
        }
        reg = U2Region( startPos, endPos - startPos + 1 );
        unfinished = !foundEnd;
    }

    return foundStart;
}

QList<AnnotationData> U1AnnotationUtils::getCaseAnnotations( const char *data, int dataLen,
    int globalOffset, bool &isUnfinishedRegion, U2Region &unfinishedRegion, bool isLowerCaseSearching )
{
    QList<AnnotationData> result;

    U2Region reg;
    int startPos = 0;
    bool unfinished = false;
    while ( findCaseRegion( data, dataLen, startPos, globalOffset, reg, unfinished,
        isLowerCaseSearching ) )
    {
        startPos = reg.endPos( ) - globalOffset;
        if ( isUnfinishedRegion ) {
            reg.startPos = unfinishedRegion.startPos;
            reg.length += unfinishedRegion.length;
            isUnfinishedRegion = false;
        }

        if ( unfinished ) {
            isUnfinishedRegion = true;
            unfinishedRegion = reg;
            break;
        } else {
            result << finalizeUnfinishedRegion( true, reg, isLowerCaseSearching );
        }
    }

    return result;
}

QList<AnnotationData> U1AnnotationUtils::finalizeUnfinishedRegion( bool isUnfinishedRegion,
    U2Region &unfinishedRegion, bool isLowerCaseSearching )
{
    QList<AnnotationData> result;

    if ( isUnfinishedRegion ) {
        AnnotationData annData;
        annData.name = isLowerCaseSearching ? lowerCaseAnnotationName : upperCaseAnnotationName;
        annData.location->regions.append(unfinishedRegion);
        annData.caseAnnotation = true;
        result.append( annData );
    }

    return result;
}

void U1AnnotationUtils::addAnnotations( QList<GObject *> &objects,
    const QList<AnnotationData> &annList, const GObjectReference &sequenceRef,
    AnnotationTableObject *annotationsObject )
{
    if ( !annList.isEmpty( ) ) {
        if ( NULL == annotationsObject ) {
            U2OpStatusImpl os;
            const U2DbiRef dbiRef = AppContext::getDbiRegistry( )->getSessionTmpDbiRef( os );
            SAFE_POINT_OP( os, );
            annotationsObject = new AnnotationTableObject( sequenceRef.objName + " features", dbiRef );
            annotationsObject->addObjectRelation(
                GObjectRelation( sequenceRef, ObjectRole_Sequence ) );
        }
        annotationsObject->addAnnotations( annList );
        if ( !objects.contains( annotationsObject ) ) {
            objects.append( annotationsObject );
        }
    }
}

QList<U2Region> U1AnnotationUtils::getRelatedLowerCaseRegions( const U2SequenceObject *so,
    const QList<GObject *> &anns )
{
    QList<GObject *> aos;
    if ( NULL != so->getDocument( ) ) {
        aos = GObjectUtils::findObjectsRelatedToObjectByRole( so, GObjectTypes::ANNOTATION_TABLE,
            ObjectRole_Sequence, anns, UOF_LoadedOnly );
    } else {
        aos = anns;
    }

    QList<U2Region> lowerCaseRegs;
    QList<U2Region> upperCaseRegs;
    foreach ( GObject *o, aos ) {
        AnnotationTableObject *ato = dynamic_cast<AnnotationTableObject *>( o );
        foreach ( const Annotation &a, ato->getAnnotations( ) ) {
            if ( a.getName( ) == lowerCaseAnnotationName ) {
                lowerCaseRegs << a.getRegions( ).toList( );
            } else if ( a.getName( ) == upperCaseAnnotationName ) {
                upperCaseRegs << a.getRegions( ).toList( );
            }
        }
    }

    if ( upperCaseRegs.isEmpty( ) ) {
        return lowerCaseRegs;
    }

    // extract lower case annotations from upper case ones
    qStableSort( upperCaseRegs.begin( ), upperCaseRegs.end( ) );

    qint64 nextStartPos = 0;
    foreach ( const U2Region &reg, upperCaseRegs ) {
        if ( reg.startPos > nextStartPos ) {
            lowerCaseRegs.append( U2Region( nextStartPos, reg.startPos - nextStartPos ) );
        }
        nextStartPos = reg.endPos( );
    }
    if ( nextStartPos < so->getSequenceLength( ) ) {
        lowerCaseRegs.append( U2Region( nextStartPos, so->getSequenceLength( ) - nextStartPos ) );
    }

    return lowerCaseRegs;
}

char * U1AnnotationUtils::applyLowerCaseRegions( char *seq, qint64 first, qint64 len,
    qint64 globalOffset, const QList<U2Region> &regs )
{
    const U2Region seqRegion( first + globalOffset, len );
    foreach ( const U2Region &reg, regs ) {
        const U2Region &intersection = seqRegion.intersect( reg );

        TextUtils::translate( TextUtils::LOWER_CASE_MAP, seq + intersection.startPos - globalOffset,
            intersection.length );
    }

    return seq;
}

QString U1AnnotationUtils::guessAminoTranslation( AnnotationTableObject *ao, const DNAAlphabet *al ) {
    DNATranslation *res = NULL;
    DNATranslationRegistry *tr = AppContext::getDNATranslationRegistry( );

    if ( NULL != ao && NULL != al ) {
        if ( al->isNucleic( ) ) {
            foreach ( const Annotation &ann, ao->getAnnotationsByName( "CDS" ) ) {
                QList<U2Qualifier> ql;
                ann.findQualifiers( "transl_table", ql );
                if ( !ql.isEmpty( ) ) {
                    const QString guess = "NCBI-GenBank #" + ql.first( ).value;
                    res = tr->lookupTranslation( al, DNATranslationType_NUCL_2_AMINO, guess );
                    if ( NULL != res ) {
                        return guess;
                    }
                }
            }
        }
    }
    return "";
}

QList <AnnotatedRegion> U1AnnotationUtils::getAnnotatedRegionsByStartPos(QList<AnnotationTableObject*> annotationObjects,
                                                                        qint64 startPos){
    QList <AnnotatedRegion> result;
    foreach (AnnotationTableObject* annObject, annotationObjects) {
        QList <Annotation> annots = annObject->getAnnotationsByRegion(U2Region(startPos, 1));
        foreach (Annotation a, annots) {
            QVector <U2Region> regions = a.getRegions();
            for (int i = 0; i < regions.size(); i++) {
                if (regions[i].startPos == startPos) {
                    AnnotatedRegion ar(a, i);
                    result.append( ar );
                }
            }
        }
    }
    return result;
}

} //namespace
