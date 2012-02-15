/**
 * UGENE - Integrated Bioinformatics Tools.
 * Copyright (C) 2008-2012 UniPro <ugene@unipro.ru>
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

#ifndef _GB2_HMMER3_HMM_FORMAT_H_
#define _GB2_HMMER3_HMM_FORMAT_H_

#include <QtCore/QByteArray>
#include <QtCore/QString>
#include <QtCore/QStringList>

#include <U2Core/global.h>
#include <U2Core/DocumentModel.h>
#include <U2Core/IOAdapter.h>

namespace U2 {

class UHMMFormat : public DocumentFormat {
    Q_OBJECT
public:
    static const DocumentFormatId   UHHMER_FORMAT_ID;
    static const QString            WRITE_LOCK_REASON;
    static const QString            WRITE_FAILED;
    
public:
    UHMMFormat( QObject* obj );
    
    virtual DocumentFormatId getFormatId() const;
    
    virtual const QString& getFormatName() const;
    
    
    virtual void storeDocument( Document* d, IOAdapter* io, U2OpStatus& os );
    
    virtual FormatCheckResult checkRawData( const QByteArray& data, const GUrl& = GUrl()) const;
    
    struct UHMMWriteException {
        QString what;
        UHMMWriteException( const QString& msg ) : what( msg ) {}
    }; // UHMMWriteException

protected:
    Document* loadDocument(IOAdapter* io, const U2DbiRef& targetDb, const QVariantMap& hints, U2OpStatus& os);
private:
    QString formatName;
    
}; // UHMMFormat

} // U2

#endif // _GB2_HMMER3_HMM_FORMAT_H_
