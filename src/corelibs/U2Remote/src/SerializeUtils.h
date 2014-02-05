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

#ifndef _U2_SERIALIZE_UTILS_H_
#define _U2_SERIALIZE_UTILS_H_

#include <U2Core/global.h>
#include <U2Core/AppContext.h>
#include <U2Core/DNAAlphabet.h>
#include <U2Core/DNASequence.h>
#include <U2Core/MAlignment.h>
#include <U2Remote/RemoteMachine.h>
#include <U2Core/VirtualFileSystem.h>
#include <U2Core/U2OpStatusUtils.h>
#include <U2Core/U2SafePoints.h>

#include <stdlib.h> //for calloc

namespace U2 {

class U2REMOTE_EXPORT SerializeUtils {
private:
    static const int SERIALIZED_DNASEQUENCE_LIST_SZ = 3;
    
    static const int SERIALIZED_MALIGNMENT_LIST_SZ = 4;
    
    static const QString REMOTE_MACHINE_VERSION_1_0;
    
public:
    template<typename T> static QVariant serializeValue( const T & t ) {
        return qVariantFromValue( t );
    }
    
    template<typename T> static bool deserializeValue( const QVariant & data, T * val ) {
        FAIL("Obsolete! Not implemented!", false);

        //if( NULL == val ) {
        //    return false;
        //}
        //if( !data.canConvert<T>() ) {
        //    return false;
        //}
        //*val = data.value<T>();
        //return true;
    }
    
    template<typename T> static QVariant serializeArray( T * arr, int sz ) {
        FAIL("Obsolete! Not implemented!", false);

        //if( NULL == arr ) {
        //    return 0;
        //}
        //assert( 0 <= sz );
        //QVariantList res;
        //for( int i = 0; i < sz; ++i ) {
        //    res << serializeValue<T>( arr[i] );
        //}
        //return res;
    }
    
    /* the arr array must be allocated by caller */
    template<typename T> static bool deserializeArray( const QVariant & data, T * arr, int sz ) {
        FAIL("Obsolete! Not implemented!", false);

        //if( data.canConvert( QVariant::Int ) && 0 == data.toInt() ) {
        //    return true;
        //}
        //if( 0 >= sz ) {
        //    return false;
        //}
        //if( !data.canConvert( QVariant::List ) ) {
        //    return false;
        //}
        //QVariantList args = data.toList();
        //if( args.size() != sz ) {
        //    return false;
        //}
        //
        //for( int i = 0; i < sz; ++i ) {
        //    if( !deserializeValue<T>( args[i], arr + i ) ) {
        //        return false;
        //    }
        //}
        //return true;
    }
    
    /* serialization of RemoteMachineSettings: human-readable QString instead of binary QVariant 
       4 deserialization functions need for tests */
    static QString serializeRemoteMachineSettings( const RemoteMachineSettingsPtr& machine );
    static RemoteMachineSettingsPtr deserializeRemoteMachineSettings( const QString & data, QString * protoId = NULL );
    //static bool deserializeRemoteMachineSettings( const QString & data, RemoteMachine ** machine );
    static RemoteMachineSettingsPtr deserializeRemoteMachineSettingsFromFile( const QString & machinePath);
    
}; // SerializeUtils

// serializeValue/deserializeValue template realizations: TODO: is it good to make them inline?
template<>
inline QVariant SerializeUtils::serializeValue<char*>( char * const& str ) {
    FAIL("Obsolete! Not implemented!", QVariant());
    Q_UNUSED(str);

    // QVariantList res;
    //if( NULL == str ) {
    //    res << serializeValue<int>( -1 );
    //    res << QVariant();
    //} else {
    //    int len = strlen( str );
    //    res << serializeValue<int>( len );
    //    res << serializeArray<char>( str, len );
    //}
    //return res;
}

template<>
inline bool SerializeUtils::deserializeValue<char*>( const QVariant & data, char ** str ) {
    FAIL("Obsolete! Not implemented!", false);
    Q_UNUSED(data);
    Q_UNUSED(str);

    //if( NULL == str ) {
    //    return false;
    //}
    //*str = NULL;

    //if( !data.canConvert( QVariant::List ) ) {
    //    return false;
    //}
    //QVariantList args = data.toList();
    //if( 2 != args.size() ) {
    //    return false;
    //}

    //int sz = 0;
    //if( !deserializeValue<int>( args[0], &sz ) ) { return false; }
    //if( -1 == sz ) {
    //    return true;
    //}
    //assert( 0 <= sz );
    //*str = (char*)calloc( sz + 1, sizeof( char ) );
    //assert( NULL != str );
    //if( !deserializeArray<char>( args[1], *str, sz ) ) { return false; }
    //(*str)[sz] = '\0';
    //return true;
}

template<>
inline QVariant SerializeUtils::serializeValue<const DNAAlphabet*>( const DNAAlphabet * const& al ) {
    FAIL("Obsolete! Not implemented!", QVariant());
    Q_UNUSED(al);

    // QVariantList res;
    //if( NULL != al ) {
    //    res << serializeValue<bool>( true );
    //    res << serializeValue<QString>( al->getId() );
    //} else {
    //    res << serializeValue<bool>( false );
    //    res << serializeValue<QString>( "" );
    //}
    //return res;
}

template<>
inline bool SerializeUtils::deserializeValue<const DNAAlphabet*>( const QVariant & data, const DNAAlphabet ** al ) {
    FAIL("Obsolete! Not implemented!", false);
    Q_UNUSED(data);
    Q_UNUSED(al);

    //if( NULL == al ) {
    //    return false;
    //}
    //*al = NULL;
    //if( !data.canConvert( QVariant::List ) ) {
    //    return false;
    //}

    //QVariantList args = data.toList();
    //if( 2 != args.size() ) {
    //    return false;
    //}

    //bool notNullAl = false;
    //QString alId;
    //if( !deserializeValue<bool>( args[0], &notNullAl ) ) { return false; }
    //if( !deserializeValue<QString>( args[1], &alId ) ) { return false; }
    //if( notNullAl ) {
    //    DNAAlphabetRegistry * alReg = AppContext::getDNAAlphabetRegistry();
    //    assert( NULL != alReg );
    //    *al = alReg->findById( alId );
    //}

    //return true;
}

template<>
inline QVariant SerializeUtils::serializeValue<DNASequence>( const DNASequence & sequence ) {
    FAIL("Obsolete! Not implemented!", QVariant());
    Q_UNUSED(sequence);

    //assert( !sequence.isNull() && NULL != sequence.alphabet );
    //QVariantList res;
    //res << serializeValue<QVariantMap>( sequence.info );
    //res << serializeValue<QByteArray>( sequence.seq );
    //res << serializeValue<const DNAAlphabet*>( sequence.alphabet );
    //return res;
}

template<>
inline bool SerializeUtils::deserializeValue<DNASequence>( const QVariant & data, DNASequence * seq ) {
    FAIL("Obsolete! Not implemented!", false);
    Q_UNUSED(data);
    Q_UNUSED(seq);

    //if( NULL == seq ) {
    //    return false;
    //}
    //if( !data.canConvert( QVariant::List ) ) {
    //    return false;
    //}
    //QVariantList args = data.toList();
    //if( SERIALIZED_DNASEQUENCE_LIST_SZ != args.size() ) {
    //    return false;
    //}

    //if( !deserializeValue<QVariantMap>( args[0], &seq->info ) ) { return false; }
    //if( !deserializeValue<QByteArray>( args[1], &seq->seq ) ) { return false; }
    //if( !deserializeValue<const DNAAlphabet*>( args[2], &seq->alphabet ) ) { return false; }
    //assert( NULL != seq->alphabet );

    //return true;
}

template<>
inline QVariant SerializeUtils::serializeValue<MAlignmentRow>( const MAlignmentRow & row) {
    FAIL("Obsolete! Not implemented!", QVariant());
    Q_UNUSED(row);

    //QVariantList res;
    //res << serializeValue<QString>( row.getName() );
    //res << serializeValue<QByteArray>( row.getCore() );
    //res << serializeValue<int>( row.getCoreLength() );
    //return res;
}

template<>
inline QVariant SerializeUtils::serializeValue< QList<MAlignmentRow> >( const QList< MAlignmentRow> & items ) {
    FAIL("Obsolete! Not implemented!", QVariant());
    Q_UNUSED(items);

    //QVariantList res;
    //foreach( const MAlignmentRow & item, items ) {
    //    res << serializeValue<MAlignmentRow>( item );
    //}
    //return res;
}

template<>
inline bool SerializeUtils::deserializeValue<MAlignmentRow>( const QVariant & data, MAlignmentRow* row ) {
    FAIL("Obsolete! Not implemented!", false);
    Q_UNUSED(data);
    Q_UNUSED(row);

    //if( !data.canConvert( QVariant::List ) ) {
    //    return false;
    //}
    //QVariantList args = data.toList();
    //if( 3 != args.size() ) {
    //    return false;
    //}

    //QString name;
    //if ( !deserializeValue<QString>( args[0], &name) ) { return false; }

    //QByteArray array;
    //if ( !deserializeValue<QByteArray>( args[1], &array) )  { return false; }

    //int offset = 0;
    //if ( !deserializeValue<int>( args[2], &offset) ) { return false; }

    //U2OpStatus2Log os;
    //MAlignmentRow createdRow = MAlignmentRow::createRow(name, array, offset, os);
    //CHECK_OP_EXT(os, row = NULL, false);

    //*row = createdRow;
    //return true;
}

template<>
inline bool SerializeUtils::deserializeValue< QList<MAlignmentRow> >( const QVariant & data, QList< MAlignmentRow > * itemList ) {
    FAIL("Obsolete! Not implemented!", false);
    Q_UNUSED(data);
    Q_UNUSED(itemList);

    //if( !data.canConvert( QVariant::List ) ) {
    //    return false;
    //}

    //QVariantList args = data.toList();
    //foreach( const QVariant & arg, args ) {
    //    MAlignmentRow item;
    //    if( !deserializeValue<MAlignmentRow>( arg, &item ) ) { return false; }
    //    itemList->append( item );
    //}
    //return true;
}

template<>
inline QVariant SerializeUtils::serializeValue<MAlignment>( const MAlignment & ma ) {
    FAIL("Obsolete! Not implemented!", QVariant());
    Q_UNUSED(ma);

    //QVariantList res;
    //res << serializeValue<const DNAAlphabet*>( ma.getAlphabet());
    //res << serializeValue< QList<MAlignmentRow> >( ma.getRows() );
    //res << serializeValue<int>( ma.getLength() );
    //res << serializeValue<QVariantMap>( ma.getInfo() );
    //return res;
}

template<>
inline bool SerializeUtils::deserializeValue<MAlignment>( const QVariant & data, MAlignment * ma ) {
    FAIL("Obsolete! Not implemented!", false);
    Q_UNUSED(data);
    Q_UNUSED(ma);

    //if( NULL == ma ) {
    //    return false;
    //}
    //if( !data.canConvert( QVariant::List ) ) {
    //    return false;
    //}
    //QVariantList args = data.toList();
    //if( SERIALIZED_MALIGNMENT_LIST_SZ != args.size() ) {
    //    return false;
    //}
    //
    //const DNAAlphabet* al = NULL;
    //if( !deserializeValue<const DNAAlphabet*>( args[0], &al) ) { return false; }

    //QList<MAlignmentRow> rows;
    //if( !deserializeValue< QList<MAlignmentRow> >( args[1], &rows) ) { return false; }

    //int len = 0;
    //if( !deserializeValue<int>( args[2], &len) ) { return false; }

    //QVariantMap info;
    //if( !deserializeValue<QVariantMap>( args[3], &info ) ) { return false; }

    //*ma = MAlignment(MA_OBJECT_NAME, al, rows);
    //ma->setInfo(info);

    //return true;
}

template<>
inline QVariant SerializeUtils::serializeValue<VirtualFileSystem>( const VirtualFileSystem & vfs ) {
    FAIL("Obsolete! Not implemented!", QVariant());
    Q_UNUSED(vfs);

    //QVariantList res;
    //
    //res << serializeValue( vfs.getId() );
    //
    //QVariantMap resMap;
    //QStringList filenames = vfs.getAllFilenames();
    //foreach( const QString & filename, filenames ) {
    //    QByteArray file = vfs.getFileByName( filename );
    //    resMap.insert( filename, serializeValue( file ) );
    //}
    //res << serializeValue( resMap );
    //
    //return res;
}

template<>
inline bool SerializeUtils::deserializeValue<VirtualFileSystem>( const QVariant & data, VirtualFileSystem * val ) {
    FAIL("Obsolete! Not implemented!", false);
    Q_UNUSED(data);
    Q_UNUSED(val);

    //if( NULL == val ) {
    //    return false;
    //}
    //if( !data.canConvert( QVariant::List ) ) {
    //    return false;
    //}
    //QVariantList args = data.toList();
    //if( 2 != args.size() ) {
    //    return false;
    //}
    //
    //QString id;
    //if( !deserializeValue( args[0], &id ) ) { return false; }
    //val->setId( id );
    //
    //QVariantMap filesMap;
    //if( !deserializeValue( args[1], &filesMap ) ) { return false; }
    //foreach( const QString & filename, filesMap.keys() ) {
    //    QByteArray fileData;
    //    if( !deserializeValue( filesMap[filename], &fileData ) ) {
    //        val->removeAllFiles();
    //        return false;
    //    }
    //    val->createFile( filename, fileData );
    //}
    //
    //return true;
}

} // U2

#endif // _U2_SERIALIZE_UTILS_H_
