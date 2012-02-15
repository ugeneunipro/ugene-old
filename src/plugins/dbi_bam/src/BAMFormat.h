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

#ifndef _U2_BAM_FORMAT_H_
#define _U2_BAM_FORMAT_H_

#include <U2Core/DocumentModel.h>

namespace U2 {
namespace BAM {

class BAMFormat : public DocumentFormat
{
    Q_OBJECT
public:
    BAMFormat(QObject *parent = NULL);

    virtual DocumentFormatId getFormatId()const;
    virtual const QString &getFormatName()const;
    virtual void storeDocument(Document *d, IOAdapter *io, U2OpStatus &os);
    virtual FormatCheckResult checkRawData(const QByteArray &rawData, const GUrl& url = GUrl())const;

protected:
    virtual Document* loadDocument(IOAdapter* io, const U2DbiRef& targetDb, const QVariantMap& hints, U2OpStatus& os);

private:
    static const QString FORMAT_ID;
    const QString formatName;
};

} // namespace BAM
} // namespace U2

#endif // _U2_BAM_FORMAT_H_
