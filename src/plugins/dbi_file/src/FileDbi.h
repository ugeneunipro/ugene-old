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

#ifndef _U2_FILE_DBI_H__
#define _U2_FILE_DBI_H__

#include <U2Core/U2AbstractDbi.h>
#include <U2Core/DocumentModel.h>

namespace U2 {

class FileDbiFactory;

class FileDbi : public U2AbstractDbi {
public:
    FileDbi(FileDbiFactory * f);
    
    virtual void init(const QHash<QString, QString>& properties, const QVariantMap& persistentData, U2OpStatus& os);
    virtual QVariantMap shutdown(U2OpStatus & op);
    virtual U2SequenceDbi * getSequenceDbi();
    
private:
    DocumentFormatId fid;
    // seqDbi is deleted by dna seq obj that holds it
    U2SequenceDbi * seqDbi;
    
}; // FileDbi

class FileDbiFactory : public U2DbiFactory {
    static const U2DbiFactoryId ID_PREFIX;

public:
    FileDbiFactory(const DocumentFormatId & fid);
    
    /** Creates new DBI instance */
    virtual U2Dbi *createDbi();

    /** Returns DBI type ID */
    virtual U2DbiFactoryId getId() const;

    /** Checks that data pointed by properties is a valid DBI resource */
    virtual bool isValidDbi(const QHash<QString, QString>& properties, const QByteArray& rawData, U2OpStatus& os) const;
    
    DocumentFormatId getDocumentFormatId() const {return fid;}
    
private:
    DocumentFormatId fid;
    
}; // FileDbiFactory

class FileDbiL10N : public QObject {
    Q_OBJECT
}; // FileDbiL10N

} // U2

#endif // _U2_FILE_DBI_H__
