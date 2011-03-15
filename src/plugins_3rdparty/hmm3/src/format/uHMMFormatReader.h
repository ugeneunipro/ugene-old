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

#ifndef _GB2_HMMER3_HMM_FORMAT_READER_H_
#define _GB2_HMMER3_HMM_FORMAT_READER_H_

#include <QtCore/QObject>
#include <QtCore/QString>

#include <U2Core/IOAdapter.h>
#include <U2Core/Task.h>

#include <hmmer3/hmmer.h>

namespace U2 {

class UHMMFormatReader : public QObject {
    Q_OBJECT
public:
    static const QString HMM_FORMAT_READER_ERROR_PREFIX;
    static const QString READ_FAILED;
    
    enum HMMER_VERSIONS {
        UNKNOWN_VERSION = -1,
        HMMER2_VERSION,
        HMMER3_VERSION
    }; // HMMER_VERSIONS
    static const QString HMMER2_VERSION_HEADER;
    static const QString HMMER3_VERSION_HEADER;
    
public:
    UHMMFormatReader( IOAdapter * i, TaskStateInfo & tsi );
    P7_HMM * getNextHmm();
    
    struct UHMMFormatReaderException {
        QString what;
        UHMMFormatReaderException( const QString& msg ) : what( msg ) {}
    }; // UHMMFormatReaderException
    
private:
    P7_HMM * readHMMER3ASCII();
    P7_HMM * readHMMER2ASCII(); /* for backward compatibility */
    HMMER_VERSIONS getVersion( const QByteArray & header ) const;
    
private:
    IOAdapter *     io; // opened io adapter
    TaskStateInfo & ti;
    
}; // UHMMFormatReader

} // U2

#endif // _GB2_HMMER3_HMM_FORMAT_READER_H_
