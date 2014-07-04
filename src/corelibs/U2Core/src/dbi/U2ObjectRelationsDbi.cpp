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

#include <U2Core/GObjectTypes.h>
#include <U2Core/U2SafePoints.h>

#include "U2ObjectRelationsDbi.h"

namespace U2 {

U2ObjectRelation::U2ObjectRelation( )
    : referencedType( GObjectTypes::UNKNOWN ), relationRole( ObjectRole_Sequence )
{

}

bool U2ObjectRelation::operator ==( const U2ObjectRelation &other ) const {
    return referencedObject == other.referencedObject && referencedName == other.referencedName
        && referencedType == other.referencedType && relationRole == other.relationRole;
}

U2ObjectRelationsDbi::U2ObjectRelationsDbi( U2Dbi *rootDbi )
    : U2ChildDbi( rootDbi )
{

}

GObjectType U2ObjectRelationsDbi::toGObjectType( const U2DataType &dbType ) {
    GObjectType result = GObjectTypes::UNKNOWN;
    if ( U2Type::Sequence == dbType ) {
        result = GObjectTypes::SEQUENCE;
    } else if ( U2Type::AnnotationTable == dbType ) {
        result = GObjectTypes::ANNOTATION_TABLE;
    } else if ( U2Type::PhyTree == dbType ) {
        result = GObjectTypes::PHYLOGENETIC_TREE;
    } else if ( U2Type::Chromatogram == dbType ) {
        result = GObjectTypes::CHROMATOGRAM;
    } else if ( U2Type::BioStruct3D == dbType ) {
        result = GObjectTypes::BIOSTRUCTURE_3D;
    }
    SAFE_POINT( GObjectTypes::UNKNOWN != result, "Unsupported object relation type detected!",
        result );
    return result;
}

U2DataType U2ObjectRelationsDbi::toDataType( const GObjectType &gType ) {
    U2DataType result = U2Type::Unknown;
    if ( GObjectTypes::SEQUENCE == gType ) {
        result = U2Type::Sequence;
    } else if ( GObjectTypes::ANNOTATION_TABLE == gType ) {
        result = U2Type::AnnotationTable;
    } else if ( GObjectTypes::PHYLOGENETIC_TREE == gType ) {
        result = U2Type::PhyTree;
    } else if ( GObjectTypes::CHROMATOGRAM == gType ) {
        result = U2Type::Chromatogram;
    } else if ( GObjectTypes::BIOSTRUCTURE_3D == gType ) {
        result = U2Type::BioStruct3D;
    }
    SAFE_POINT( U2Type::Unknown != result, "Unsupported object relation type detected!",
        result );
    return result;
}

} // namespace U2
