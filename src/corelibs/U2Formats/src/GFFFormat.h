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

#ifndef _U2_GFF_FORMAT_H_
#define _U2_GFF_FORMAT_H_

#include <U2Core/BaseDocumentFormats.h>
#include <U2Core/DocumentModel.h>

namespace U2 {

class IOAdapter;

class U2FORMATS_EXPORT  GFFFormat : public DocumentFormat {
    Q_OBJECT
public:
    GFFFormat(QObject* p);

    virtual DocumentFormatId getFormatId() const {return BaseDocumentFormats::GFF;}

    virtual const QString& getFormatName() const {return formatName;}

    virtual Document* loadDocument( IOAdapter* io, TaskStateInfo& ti, const QVariantMap& fs, DocumentLoadMode mode = DocumentLoadMode_Whole);

    void load(IOAdapter* io, QList<GObject*>& objects, TaskStateInfo& si);

    virtual void storeDocument( Document* d, TaskStateInfo& ts, IOAdapter* io );

    virtual FormatDetectionResult checkRawData(const QByteArray& rawData, const GUrl& = GUrl()) const;

private:
    QStringList parseLine(QString line) const;

    QString formatName;
};

}//namespace

#endif
