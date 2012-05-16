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

#include "RawDNASequenceFormat.h"

#include "DocumentFormatUtils.h"
#include "PlainTextFormat.h"

#include <U2Core/U2OpStatus.h>
#include <U2Core/IOAdapter.h>
#include <U2Core/U2SafePoints.h>
#include <U2Core/GObjectTypes.h>
#include <U2Core/DNASequenceObject.h>
#include <U2Core/U1AnnotationUtils.h>
#include <U2Core/U2SequenceUtils.h>

#include <U2Core/TextUtils.h>

namespace U2 {

/* TRANSLATOR U2::RawDNASequenceFormat */
/* TRANSLATOR U2::IOAdapter */

RawDNASequenceFormat::RawDNASequenceFormat(QObject* p) : DocumentFormat(p, DocumentFormatFlags_W1)
{
    formatName = tr("Raw sequence");
    fileExtensions << "seq" << "txt";
    supportedObjectTypes+=GObjectTypes::SEQUENCE;
    formatDescription = tr("Raw sequence file - a whole content of the file is treated either as a single nucleotide or peptide sequence UGENE will remove all non-alphabetic chars from the result sequence");
}


static void load(IOAdapter* io, const U2DbiRef& dbiRef,  QList<GObject*>& objects, const QVariantMap& fs, U2OpStatus& os) {
    DbiOperationsBlock opBlock(dbiRef, os);
    CHECK_OP(os, );
    static const int READ_BUFF_SIZE = 4096;

    U2SequenceImporter  seqImporter(fs, true);

    QByteArray readBuffer(READ_BUFF_SIZE, '\0');
    char* buff  = readBuffer.data();

    const QBitArray& ALPHAS = TextUtils::ALPHA_NUMS;

    QByteArray seq;

    //reading sequence
    QBuffer writer(&seq);
    writer.open(QIODevice::WriteOnly);
    bool ok = true;
    int len = 0;
    bool isStarted = false;

    while (ok && (len = io->readBlock(buff, READ_BUFF_SIZE)) > 0) {
        seq.clear();
        bool isSeek = writer.seek(0);
                assert(isSeek); Q_UNUSED(isSeek);
        if (os.isCoR()) {
            break;
        }
        
        for (int i=0; i<len && ok; i++) {
            char c = buff[i];
            if (ALPHAS[(uchar)c]) {
                ok = writer.putChar(c);
            }
        }
        if(seq.size()>0 && isStarted == false ){
            isStarted = true;
            seqImporter.startSequence(dbiRef,"Sequence",false,os);
        }
        if(isStarted){
            seqImporter.addBlock(seq.data(),seq.size(),os);
        }
        if (os.isCoR()) {
            break;
        }
        os.setProgress(io->getProgress());
    }
    writer.close();

    CHECK_OP(os, );

    CHECK_EXT(isStarted == true, os.setError(RawDNASequenceFormat::tr("Sequence is empty")), );
    U2Sequence u2seq = seqImporter.finalizeSequence(os);
    TmpDbiObjects dbiObjects(dbiRef, os);
    dbiObjects.objects << u2seq.id;
    CHECK_OP(os, );

    GObjectReference sequenceRef(io->getURL().getURLString(), u2seq.visualName, GObjectTypes::SEQUENCE);
    U1AnnotationUtils::addAnnotations(objects, seqImporter.getCaseAnnotations(), sequenceRef, NULL);

    objects << new U2SequenceObject(u2seq.visualName,U2EntityRef(dbiRef, u2seq.id));
}

Document* RawDNASequenceFormat::loadDocument(IOAdapter* io, const U2DbiRef& dbiRef, const QVariantMap& fs, U2OpStatus& os) {
    QList<GObject*> objects;
        load(io, dbiRef, objects, fs, os);
    CHECK_OP(os, NULL);
    Document* doc = new Document(this, io->getFactory(), io->getURL(), dbiRef, objects, fs);
    return doc;
}

FormatCheckResult RawDNASequenceFormat::checkRawData(const QByteArray& rawData, const GUrl&) const {
    const char* data = rawData.constData();
    int size = rawData.size();
    if(QRegExp("[a-zA-Z\r\n]*").exactMatch(rawData)) {
        return FormatDetection_VeryHighSimilarity;
    }
    bool hasBinaryData = TextUtils::contains(TextUtils::BINARY, data, size);
    //returning 'very low chance' here just because it's impossible to have 100% detection for this format
    return hasBinaryData ? FormatDetection_NotMatched : FormatDetection_VeryLowSimilarity; 
}

void RawDNASequenceFormat::storeDocument(Document* d, IOAdapter* io, U2OpStatus& os) {
    QList<GObject*> objects = d->findGObjectByType(GObjectTypes::SEQUENCE);
    assert(objects.size() == 1);
    GObject* obj = objects.first();
    U2SequenceObject* so = qobject_cast<U2SequenceObject*>(obj);
    assert(so!=NULL);
    PlainTextFormat::storeRawData(so->getWholeSequenceData(), os, io);
}

void RawDNASequenceFormat::storeEntry(IOAdapter *io, U2SequenceObject *seq, const QList<GObject*> &anns, U2OpStatus &os) {
    Q_UNUSED(anns);
    PlainTextFormat::storeRawData(seq->getWholeSequenceData(), os, io);
    CHECK_OP(os, );

    io->writeBlock("\n", 1);
}


}//namespace
