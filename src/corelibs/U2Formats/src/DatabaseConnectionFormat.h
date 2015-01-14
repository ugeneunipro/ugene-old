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

#ifndef _U2_DATABASE_CONNECTION_FORMAT_H_
#define _U2_DATABASE_CONNECTION_FORMAT_H_

#include <U2Core/DocumentModel.h>

namespace U2 {

class U2FORMATS_EXPORT DatabaseConnectionFormat : public DocumentFormat {
public:
    DatabaseConnectionFormat(QObject* p);

    DocumentFormatId getFormatId() const;
    const QString& getFormatName() const;

    FormatCheckResult checkRawData(QByteArray const &, GUrl const & = GUrl()) const;

private:
    // Ignore incoming U2DbiRef, use U2DbiRef from IOAdapter
    Document* loadDocument(IOAdapter* io, const U2DbiRef& unused, const QVariantMap& hints, U2OpStatus& os);

    QList<GObject*> getObjects(U2Dbi* dbi, U2OpStatus &os);

    QString formatName;
};

}   // namespace U2

#endif // _U2_DATABASE_CONNECTION_FORMAT_H_
