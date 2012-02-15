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

#ifndef _U2_GENBANK_PLAIN_TEXT_FORMAT_H_
#define _U2_GENBANK_PLAIN_TEXT_FORMAT_H_

#include "EMBLGenbankAbstractDocument.h"

namespace U2 {

class U2FORMATS_EXPORT GenbankPlainTextFormat : public EMBLGenbankAbstractDocument {
    Q_OBJECT
public:
    GenbankPlainTextFormat(QObject* p);

    virtual void storeDocument(Document* doc, IOAdapter* io, U2OpStatus& os);
    
    virtual FormatCheckResult checkRawData(const QByteArray& rawData, const GUrl& = GUrl()) const;

    virtual bool isStreamingSupport() {return true;}

    virtual void storeEntry(IOAdapter *io, U2SequenceObject *seq, const QList<GObject*> &anns, U2OpStatus &os);

protected:

    bool readIdLine(ParserState*);
    bool readEntry(ParserState*, U2SequenceImporter&,int& seqSize,int& fullSeqSize,bool merge, int gapSize,U2OpStatus&);
    void readHeaderAttributes(QVariantMap& tags, DbiConnection& con, U2SequenceObject* so);
    //void readAnnotations(ParserState*, int offset);
};


}//namespace

#endif
