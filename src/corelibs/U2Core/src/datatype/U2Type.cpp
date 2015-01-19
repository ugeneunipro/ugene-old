/**
 * UGENE - Integrated Bioinformatics Tools.
 * Copyright (C) 2008-2015 UniPro <ugene@unipro.ru>
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

#include "U2Type.h"

namespace U2 {

//////////////////////////////////////////////////////////////////////////
/// U2Type
//////////////////////////////////////////////////////////////////////////

/** Type is unknown. Default value. */
const U2DataType U2Type::Unknown                     = 0;

/** Object types */
const U2DataType U2Type::Sequence                    = 1;
const U2DataType U2Type::Msa                         = 2;
const U2DataType U2Type::Assembly                    = 4;
const U2DataType U2Type::VariantTrack                = 5;
const U2DataType U2Type::VariantType                 = 6;
const U2DataType U2Type::FilterTableType             = 7;
const U2DataType U2Type::KnownMutationsTrackType     = 8;
const U2DataType U2Type::AnnotationTable             = 10;

/** UDR object types 101..199 */
const U2DataType U2Type::Text                        = 101;
const U2DataType U2Type::PhyTree                     = 102;
const U2DataType U2Type::Chromatogram                = 103;
const U2DataType U2Type::BioStruct3D                 = 104;
const U2DataType U2Type::PFMatrix                    = 105;
const U2DataType U2Type::PWMatrix                    = 106;

const U2DataType U2Type::CrossDatabaseReference      = 999;

/** SCO (non-object, non-root) types */
const U2DataType U2Type::Annotation                  = 1000;
const U2DataType U2Type::AnnotationGroup             = 1001;

/**  Assembly read */
const U2DataType U2Type::AssemblyRead                = 1100;

/** Sequence feature */
const U2DataType U2Type::Feature                     = 1300;

const U2DataType U2Type::UdrRecord                   = 1400;

/**  Attribute types */
const U2DataType U2Type::AttributeInteger            = 2001;
const U2DataType U2Type::AttributeReal               = 2002;
const U2DataType U2Type::AttributeString             = 2003;
const U2DataType U2Type::AttributeByteArray          = 2004;

bool U2Type::isObjectType(U2DataType type) {
    return type > 0 && type <= 999;
}

bool U2Type::isUdrObjectType(U2DataType type) {
    return type > 100 && type < 200;
}

bool U2Type::isAttributeType(U2DataType type) {
    return type >=2000 && type < 2100;
}

//////////////////////////////////////////////////////////////////////////
/// U2Entity implementation
//////////////////////////////////////////////////////////////////////////

U2Entity::U2Entity( U2DataId id )
    : id( id )
{

}

U2Entity::U2Entity( const U2Entity &other )
    : id( other.id )
{

}

U2Entity::~U2Entity( ) {

}

bool U2Entity::hasValidId( ) const {
    return !id.isEmpty( );
}

U2Entity U2Entity::operator =( const U2Entity &other ) {
    id = other.id;
    return *this;
}

bool U2Entity::operator ==( const U2Entity &other ) const {
    return id == other.id;
}

bool U2Entity::operator !=( const U2Entity &other ) const {
    return id != other.id;
}

bool U2Entity::operator <( const U2Entity &other ) const {
    return id < other.id;
}

//////////////////////////////////////////////////////////////////////////
/// U2DbiRef implementation
//////////////////////////////////////////////////////////////////////////

U2DbiRef::U2DbiRef(const U2DbiFactoryId &dbiFactoryId, const U2DbiId &dbiId)
    : dbiFactoryId(dbiFactoryId), dbiId(dbiId)
{

}

bool U2DbiRef::isValid() const {
    return !dbiFactoryId.isEmpty() && !dbiId.isEmpty();
}

bool U2DbiRef::operator!=(const U2DbiRef &r2) const {
    return !(operator ==(r2));
}

bool U2DbiRef::operator==(const U2DbiRef &r2) const {
    return dbiFactoryId == r2.dbiFactoryId && dbiId == r2.dbiId;
}

bool U2DbiRef::operator <(const U2DbiRef &r2) const {
    return dbiFactoryId + dbiId < r2.dbiFactoryId + r2.dbiId;
}

QDataStream & operator <<(QDataStream &out, const U2DbiRef &dbiRef) {
    out << dbiRef.dbiFactoryId << dbiRef.dbiId;
    return out;
}

QDataStream & operator >>(QDataStream &in, U2DbiRef &dbiRef) {
    in >> dbiRef.dbiFactoryId;
    in >> dbiRef.dbiId;
    return in;
}

//////////////////////////////////////////////////////////////////////////
/// U2EntityRef implementation
//////////////////////////////////////////////////////////////////////////

U2EntityRef::U2EntityRef()
    : version(0)
{

}

U2EntityRef::U2EntityRef(const U2DbiRef &dbiRef, const U2DataId &entityId)
    : dbiRef(dbiRef), entityId(entityId), version(0)
{

}

bool U2EntityRef::isValid() const {
    return dbiRef.isValid() && !entityId.isEmpty();
}

bool U2EntityRef::operator == (const U2EntityRef &other) const {
    return (entityId == other.entityId) && (version == other.version) && (dbiRef == other.dbiRef);
}

bool U2EntityRef::operator !=(const U2EntityRef &other) const {
    return !(operator ==(other));
}

bool U2EntityRef::operator <(const U2EntityRef &other) const {
    return dbiRef.dbiFactoryId + dbiRef.dbiId + entityId + QString::number(version)
        < other.dbiRef.dbiFactoryId + other.dbiRef.dbiId + other.entityId + QString::number(other.version);
}

namespace {

bool registerMetaInfo() {
    qRegisterMetaType<U2DbiRef>("U2::U2DbiRef");
    qRegisterMetaTypeStreamOperators<U2DbiRef>("U2::U2DbiRef");
    return true;
}

}

bool U2DbiRef::metaInfoRegistered = registerMetaInfo();

} // U2
