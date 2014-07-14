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

#include "U2Type.h"

namespace U2 {

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

namespace {

bool registerMetaInfo() {
    qRegisterMetaType<U2DbiRef>("U2::U2DbiRef");
    qRegisterMetaTypeStreamOperators<U2DbiRef>("U2::U2DbiRef");
    return true;
}

}

bool U2DbiRef::metaInfoRegistered = registerMetaInfo();

} // U2
