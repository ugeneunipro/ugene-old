
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
