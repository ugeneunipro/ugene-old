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

#ifndef _U2_SCF_FORMAT_H_
#define _U2_SCF_FORMAT_H_

#include <U2Core/BaseDocumentFormats.h>
#include <U2Core/DocumentModel.h>

namespace U2 {

class IOAdapter;
class SeekableBuf;
class DNAChromatogram;
class DNASequence;
class U2OpStatus;

class U2FORMATS_EXPORT  SCFFormat : public DocumentFormat {
    Q_OBJECT
public:
    SCFFormat(QObject* p);

    virtual DocumentFormatId getFormatId() const {return BaseDocumentFormats::SCF;}

    virtual const QString& getFormatName() const {return formatName;}

    virtual FormatCheckResult checkRawData(const QByteArray& rawData, const GUrl& = GUrl()) const;

    static void exportDocumentToSCF(const QString& fileName, const DNAChromatogram& cd, const QByteArray& seq, U2OpStatus& ts);

protected:
    virtual Document* loadDocument(IOAdapter* io, const U2DbiRef& dbiRef, const QVariantMap& fs, U2OpStatus& os);

private:
    Document* parseSCF(const U2DbiRef& dbiRef, SeekableBuf*, IOAdapterFactory* io, const GUrl& url, const QVariantMap& fs, U2OpStatus& os);

    QString formatName;
};

}//namespace

#endif
