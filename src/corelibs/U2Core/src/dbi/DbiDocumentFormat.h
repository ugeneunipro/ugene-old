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

#ifndef _U2_SQLITE_DOCUMENT_FORMAT_H_
#define _U2_SQLITE_DOCUMENT_FORMAT_H_

#include <U2Core/U2Dbi.h>
#include <U2Core/BaseDocumentFormats.h>
#include <U2Core/DocumentModel.h>

namespace U2 {

/** UGENE v1 wrapper over DBI (UGENE v2 data access) interface */
class U2CORE_EXPORT DbiDocumentFormat: public DocumentFormat {
    Q_OBJECT
public:
    DbiDocumentFormat(const U2DbiFactoryId& id, const DocumentFormatId& formatId, 
        const QString& formatName, const QStringList& exits, QObject* p = NULL);

    virtual DocumentFormatId getFormatId() const {return formatId;}

    virtual const QString& getFormatName() const {return formatName;}

    virtual Document* createNewDocument(IOAdapterFactory* io, const QString& url, const QVariantMap& fs = QVariantMap());

    virtual Document* loadDocument(IOAdapter* io, TaskStateInfo& ti, const QVariantMap& fs, DocumentLoadMode mode = DocumentLoadMode_Whole);

    virtual void storeDocument(Document* d, TaskStateInfo& ts, IOAdapter* io);

    virtual RawDataCheckResult checkRawData(const QByteArray& rawData, const GUrl& url = GUrl()) const;

private:
    QString             formatName;
    U2DbiFactoryId      id;
    DocumentFormatId    formatId;
};

}//namespace

#endif
