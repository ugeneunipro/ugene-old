/**
 * UGENE - Integrated Bioinformatics Tools.
 * Copyright (C) 2008-2011 UniPro <ugene@unipro.ru>
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


#include <QtCore/QStringList>

#include "MAlignmentInfo.h"

namespace U2 {

const QString MAlignmentInfo::NAME                  = "NAME";
const QString MAlignmentInfo::ACCESSION             = "ACCESSION";
const QString MAlignmentInfo::DESCRIPTION           = "DESCRIPTION";
const QString MAlignmentInfo::SS_CONSENSUS          = "SS_CONSENSUS";
const QString MAlignmentInfo::REFERENCE_LINE        = "REFERENCE_LINE";
const QString MAlignmentInfo::CUTOFFS               = "CUTOFFS";

bool MAlignmentInfo::isValid( const QVariantMap& map ) {
    return hasName( map );
}

static QVariant getValue( const QString& tag, const QVariantMap& map ) {
    return map.value( tag );
}

static void setValue( QVariantMap& map, const QString& tag, const QVariant& val ) {
    assert( !tag.isEmpty() );
    if( !val.isNull() ) {
        map.insert( tag, val );
    }
}

QString MAlignmentInfo::getName( const QVariantMap& map ) {
    return getValue( NAME, map ).toString();
}

void MAlignmentInfo::setName( QVariantMap& map, const QString& name ) {
    setValue( map, NAME, name );
}

bool MAlignmentInfo::hasName( const QVariantMap& map ) {
    return !getName( map ).isEmpty();
}

QString MAlignmentInfo::getAccession( const QVariantMap& map ) {
    return getValue( ACCESSION, map ).toString();
}

void MAlignmentInfo::setAccession( QVariantMap& map, const QString& acc ) {
    setValue( map, ACCESSION, acc );
}

bool MAlignmentInfo::hasAccession( const QVariantMap& map ) {
    return !getAccession( map ).isEmpty();
}

QString MAlignmentInfo::getDescription( const QVariantMap& map ) {
    return getValue( DESCRIPTION, map ).toString();
}

void MAlignmentInfo::setDescription( QVariantMap& map, const QString& desc ) {
    setValue( map, DESCRIPTION, desc );
}

bool MAlignmentInfo::hasDescription( const QVariantMap& map ) {
    return !getDescription( map ).isEmpty();
}

QString MAlignmentInfo::getSSConsensus( const QVariantMap& map ) {
    return getValue( SS_CONSENSUS, map ).toString();
}

void MAlignmentInfo::setSSConsensus( QVariantMap& map, const QString& cs ) {
    setValue( map, SS_CONSENSUS, cs );
}

bool MAlignmentInfo::hasSSConsensus( const QVariantMap& map ) {
    return !getSSConsensus( map ).isEmpty();
}

QString MAlignmentInfo::getReferenceLine( const QVariantMap& map ) {
    return getValue( REFERENCE_LINE, map ).toString();
}

void MAlignmentInfo::setReferenceLine( QVariantMap& map ,const QString& rf ) {
    setValue( map, REFERENCE_LINE, rf );
}

bool MAlignmentInfo::hasReferenceLine( const QVariantMap& map ) {
    return !getReferenceLine( map ).isEmpty();
}

void MAlignmentInfo::setCutoff( QVariantMap& map, Cutoffs coff, float val ) {
    setValue( map, CUTOFFS + QString::number( static_cast< int >( coff ) ), val );
}

float MAlignmentInfo::getCutoff( const QVariantMap& map, Cutoffs coff ) {
    return static_cast< float >( getValue( CUTOFFS + QString::number( static_cast< int >( coff ) ), map ).toDouble() );
}

bool MAlignmentInfo::hasCutoff( const QVariantMap& map, Cutoffs coff ) {
    bool ok = false;
    getValue( CUTOFFS + QString::number( static_cast< int >( coff ) ), map ).toDouble( &ok );
    return ok;
}

} // U2
