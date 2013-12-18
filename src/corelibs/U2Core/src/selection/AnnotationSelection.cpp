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

#include <U2Core/DNATranslation.h>
#include <U2Core/AnnotationTableObject.h>
#include <U2Core/TextUtils.h>
#include <U2Core/U2SafePoints.h>
#include <U2Core/U2DbiUtils.h>
#include <U2Core/U2SequenceDbi.h>
#include <U2Core/U2OpStatus.h>
#include <U2Core/U2SequenceUtils.h>

#include "AnnotationSelection.h"

namespace U2 {

static QList<Annotation> emptyAnnotations;

AnnotationSelectionData::AnnotationSelectionData( const Annotation &a, int lIdx )
    : annotation( a ), locationIdx( lIdx )
{

}

bool AnnotationSelectionData::operator ==( const AnnotationSelectionData &d ) const {
    return d.annotation == annotation && d.locationIdx == locationIdx;
}

QVector<U2Region> AnnotationSelectionData::getSelectedRegions( ) const {
    QVector<U2Region> res;
    if ( -1 != locationIdx ) {
        res << annotation.getRegions( )[locationIdx];
    } else {
        res << annotation.getRegions( );
    }
    return res;
}

int AnnotationSelectionData::getSelectedRegionsLen( ) const {
    if ( -1 != locationIdx ) {
        return annotation.getRegions( )[locationIdx].length;
    }
    int len = 0;
    foreach ( const U2Region &r, annotation.getRegions( ) ) {
        len += r.length;
    }
    return len;
}

AnnotationSelection::AnnotationSelection( QObject *p )
    : GSelection( GSelectionTypes::ANNOTATIONS, p )
{

}

const QList<AnnotationSelectionData> & AnnotationSelection::getSelection( ) const {
    return selection;
}

bool AnnotationSelection::isEmpty( ) const {
    return selection.isEmpty( );
}

void AnnotationSelection::clear( ) {
    if ( selection.isEmpty( ) ) {
        return;
    }
    QList<Annotation> tmpRemoved;
    foreach ( const AnnotationSelectionData &asd, selection ) {
        tmpRemoved.push_back( asd.annotation );
    }
    selection.clear( );
    emit si_selectionChanged( this, emptyAnnotations, tmpRemoved );
}

bool AnnotationSelection::contains( const Annotation &a ) const {
    return NULL != getAnnotationData( a );
}

void AnnotationSelection::removeObjectAnnotations( AnnotationTableObject *obj ) {
    QList<Annotation> removed;
    foreach ( const Annotation &a, obj->getAnnotations( ) ) {
        for( int i = 0; i < selection.size( ); i++ ) {
            if ( selection[i].annotation == a ) {
                removed.append( a );
                selection.removeAt( i );
                --i;
            }
        }
    }
    emit si_selectionChanged( this, emptyAnnotations, removed );
}

void AnnotationSelection::addToSelection( const Annotation &a, int locationIdx ) {
    const int nRegionsTotal = a.getRegions( ).size( );
    SAFE_POINT( locationIdx >=-1 && locationIdx < nRegionsTotal, "Invalid location index!", );

    int nRegionsBefore = 0;
    foreach ( const AnnotationSelectionData &asd, selection ) {
        if ( asd.annotation == a ) {
            nRegionsBefore++;
            if ( asd.locationIdx == locationIdx || asd.locationIdx == -1 ) {
                return; //nothing changed
            }
        }
    }
    SAFE_POINT( nRegionsBefore < nRegionsTotal, "Invalid annotation region count!", );

    //remove old regions if needed
    const bool removeFromSelection = nRegionsBefore > 0
        && ( ( nRegionsBefore == nRegionsTotal - 1 ) || locationIdx == -1 );
    if ( removeFromSelection ) {
        QList<AnnotationSelectionData> newSelection;
        foreach ( const AnnotationSelectionData &asd, selection ) {
            if ( asd.annotation != a ) {
                newSelection.append( asd );
            }
        }
        selection = newSelection;
    }
    int updatedLocationIdx = ( nRegionsBefore == nRegionsTotal - 1 ) ? -1 : locationIdx;
    selection.append( AnnotationSelectionData( a, updatedLocationIdx ) );
    QList<Annotation> tmp;
    tmp.append( a );
    emit si_selectionChanged( this, tmp, emptyAnnotations );
}

void AnnotationSelection::removeFromSelection( const Annotation &a, int locationIdx ) {
    bool inSel = false;
    bool inSelPartial = false;
    int nRegionsSelected = 0;
    const int nRegionsTotal = a.getRegions().size();
    foreach(const AnnotationSelectionData& asd, selection) {
        if (asd.annotation == a) {
            inSel = true;
            inSelPartial = asd.locationIdx != -1;
            nRegionsSelected++;
        }
    }
    if (!inSel) {
        return;
    }
    bool leftInSelection = false;
    if (locationIdx == -1) { // remove all regions from selection
        QList<AnnotationSelectionData> newSelection;
        foreach(const AnnotationSelectionData& asd, selection) {
            if (asd.annotation != a) {
                newSelection.append(asd);
            } 
        }
        selection = newSelection;
    } else if (inSelPartial) { //remove 1 region from set of regions
        for (int i = 0, n = selection.size(); i<n; i++) {
            const AnnotationSelectionData& asd = selection[i];
            if (asd.annotation == a && asd.locationIdx == locationIdx) {
                selection.removeAt(i);    
                break;
            } 
        }
        leftInSelection = nRegionsSelected - 1 > 0;
    } else { // selection contains whole annotation, only 1 region is removed -> split
        assert(!inSelPartial && locationIdx!=-1);
        for (int i = 0, n = selection.size(); i<n; i++) {
            const AnnotationSelectionData& asd = selection[i];
            if (asd.annotation == a) {
                assert(asd.locationIdx == -1);
                selection.removeAt(i);
                break;
            } 
        }
        for (int i = 0, n = nRegionsTotal; i<n; i++) {
            if (i != locationIdx) {
                selection.append(AnnotationSelectionData(a, i));
            }
        }
        leftInSelection = nRegionsTotal > 1;
    }
    QList<Annotation> tmp;
    tmp << a;
    emit si_selectionChanged( this, leftInSelection ? tmp : emptyAnnotations, tmp );
}

const AnnotationSelectionData * AnnotationSelection::getAnnotationData( const Annotation &a ) const {
    foreach ( const AnnotationSelectionData &asd, selection ) {
        if ( asd.annotation == a ) {
            return &asd;
        }
    }
    return NULL;
}

void AnnotationSelection::getAnnotationSequence( QByteArray &res, const AnnotationSelectionData &ad,
    char gapSym, const U2EntityRef &seqRef, DNATranslation *complTT, DNATranslation *aminoTT,
    U2OpStatus &os )
{
    QVector<U2Region> regions;
    if ( -1 == ad.locationIdx ) {
        regions = ad.annotation.getLocation( )->regions;
    } else {
        regions << ad.annotation.getLocation( )->regions.at( ad.locationIdx );
    }
    QList<QByteArray> parts  = U2SequenceUtils::extractRegions( seqRef, regions, complTT, aminoTT,
        false, os );
    CHECK_OP( os, );
    qint64 resLen = 0;
    foreach ( const QByteArray &p, parts ) {
        resLen += p.length( );
    }
    res.reserve( resLen );
    foreach ( const QByteArray &p, parts ) {
        if ( !res.isEmpty( ) ) {
            res.append( gapSym );
        }
        res.append( p );
    }
}


bool AnnotationSelection::contains( const Annotation &a, int locationIdx ) const {
    foreach ( const AnnotationSelectionData &asd, selection ) {
        if ( asd.annotation == a && ( asd.locationIdx == -1 || asd.locationIdx == locationIdx ) ) {
            return true;
        }
    }
    return false;
}


//////////////////////////////////////////////////////////////////////////
// Annotation group selection
static QList<AnnotationGroup> emptyGroups;

AnnotationGroupSelection::AnnotationGroupSelection( QObject *p )
    : GSelection( GSelectionTypes::ANNOTATION_GROUPS, p )
{

}

const QList<AnnotationGroup> & AnnotationGroupSelection::getSelection( ) const {
    return selection;
}

bool AnnotationGroupSelection::isEmpty( ) const {
    return selection.isEmpty( );
}

bool AnnotationGroupSelection::contains( const AnnotationGroup &g ) const {
    return selection.contains( g );
}

void AnnotationGroupSelection::clear( ) {
    if ( selection.isEmpty( ) ) {
        return;
    }
    QList<AnnotationGroup> tmpRemoved = selection;
    selection.clear( );
    emit si_selectionChanged( this, emptyGroups, tmpRemoved );
}

void AnnotationGroupSelection::removeObjectGroups( AnnotationTableObject *obj ) {
    QList<AnnotationGroup> tmpRemoved;
    foreach ( const AnnotationGroup &g, selection ) {
        if ( g.getGObject( ) == obj ) {
            tmpRemoved.append( g );
        }
    }
    foreach ( const AnnotationGroup &g, tmpRemoved ) {
        selection.removeOne( g );
    }
    emit si_selectionChanged( this, emptyGroups, tmpRemoved );
}

void AnnotationGroupSelection::addToSelection( const AnnotationGroup &g ) {
    if ( selection.contains( g ) ) {
        return;
    }
    selection.append( g );
    QList<AnnotationGroup> tmpAdded;
    tmpAdded.append( g );
    emit si_selectionChanged( this, tmpAdded, emptyGroups );
}


void AnnotationGroupSelection::removeFromSelection( const AnnotationGroup &g ) {
    const bool found = selection.removeOne( g );
    if ( found ) {
        QList<AnnotationGroup> tmpRemoved;
        tmpRemoved.append( g );
        emit si_selectionChanged( this, emptyGroups, tmpRemoved );
    }
}

// Returns list of locations of all selected annotations
QVector<U2Region> AnnotationSelection::getSelectedLocations(
    const QSet<AnnotationTableObject *> &objects ) const
{
    QVector<U2Region> result;
    if ( objects.isEmpty( ) ) {
        return result;
    }
    foreach ( const AnnotationSelectionData &d, selection ) {
        if ( objects.contains( d.annotation.getGObject( ) ) ) {
            result << d.getSelectedRegions( );
        }
    }
    return result;
}
}//namespace
