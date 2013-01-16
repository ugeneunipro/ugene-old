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

#ifndef _U2_ANSTRACTDIFFERENTIALFORMAT_H_
#define _U2_ANSTRACTDIFFERENTIALFORMAT_H_

#include <U2Core/AnnotationData.h>
#include <U2Core/DocumentModel.h>
#include <U2Core/U2OpStatus.h>

#include "ColumnDataParser.h"

namespace U2 {

class U2FORMATS_EXPORT AbstractDifferentialFormat : public DocumentFormat {
    Q_OBJECT
public:
    AbstractDifferentialFormat(QObject *parent);

    void storeDocument(Document *d, IOAdapter *io, U2OpStatus &os);
    FormatCheckResult checkRawData(const QByteArray &rawData,
        const GUrl &url = GUrl()) const;

protected:
    Document * loadDocument(IOAdapter *io, const U2DbiRef &targetDb,
        const QVariantMap &hints, U2OpStatus &os);
    virtual QList<ColumnDataParser::Column> getColumns() const = 0;
    virtual QString getAnnotationName() const = 0;

private:
    static const int BUFFER_SIZE;

private:
    QList<SharedAnnotationData> parseAnnotations(const ColumnDataParser &parser,
        IOAdapter *io, QByteArray &buffer, U2OpStatus &os);
    void writeHeader(IOAdapter *io);

    static QString readLine(IOAdapter *io, QByteArray &buffer, U2OpStatus &os);
    static bool parseLocus(const QString &locus, SharedAnnotationData &data, U2OpStatus &os);
    static QString createLocus(SharedAnnotationData data, U2OpStatus &os);
    static QString createValue(SharedAnnotationData data, const ColumnDataParser::Column &column, U2OpStatus &os);
};

} // U2

#endif // _U2_ANSTRACTDIFFERENTIALFORMAT_H_
