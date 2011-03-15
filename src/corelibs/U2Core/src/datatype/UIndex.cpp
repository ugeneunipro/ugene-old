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


#include "UIndex.h"

namespace {

bool iosHasId( const QList< U2::UIndex::IOSection >& ios, const QString& look_for ) {
    foreach( U2::UIndex::IOSection io, ios ) {
        if( io.sectionId == look_for ) {
            return true;
        }
    }
    return false;
}

bool hasEmptyKeyVal( QHash< QString, QString > keys ) {
    QHash< QString, QString >::const_iterator it = keys.begin();
    while( keys.end() != it ) {
        if( it.key().isEmpty() || it.value().isEmpty() ) {
            return true;
        }
        ++it;
    }
    return false;
}

} // anonymous namespace

namespace U2 {

UIndex::UIndexError UIndex::ItemSection::checkConsistentcy() const {
    if( ioSectionId.isEmpty() ) {
        return EMPTY_IO_ID;
    }
    if( docFormat.isEmpty() ) {
        return EMPTY_DOC_FORMAT;
    }
    if( 0 > startOff || 0 > endOff || endOff <= startOff ) {
        return BAD_OFFSET;
    }
    if( hasEmptyKeyVal( keys ) ) {
        return EMPTY_KEY_VAL;
    }
    return NO_ERR;
}

UIndex::UIndexError UIndex::IOSection::checkConsistentcy() const {
    if( sectionId.isEmpty() ) {
        return EMPTY_IO_ID;
    }
    if( ioAdapterId.isEmpty() ) {
        return EMPTY_IO_ADAPTER;
    }
    if( url.isEmpty() ) {
        return EMPTY_URL;
    }
    if( hasEmptyKeyVal( keys ) ) {
        return EMPTY_KEY_VAL;
    }
    return NO_ERR;
}

UIndex::IOSection UIndex::getIOSection(const QString& id) const {
    foreach( IOSection ioSec, ios ) {
        if( ioSec.sectionId == id ) {
            return ioSec;
        }
    }
    return IOSection(); // there is no section with id
}

bool UIndex::hasItems() const {
    return !items.isEmpty();
}

UIndex::UIndexError UIndex::checkConsistency() const {
    foreach( ItemSection it, items ) {
        if( !iosHasId( ios, it.ioSectionId ) ) {
            return NO_SUCH_IO;
        }
    }
    return NO_ERR;
}

} // U2
