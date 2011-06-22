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

#include "RawDNASequenceFormat.h"

#include "DocumentFormatUtils.h"
#include "PlainTextFormat.h"

#include <U2Core/Task.h>
#include <U2Core/IOAdapter.h>

#include <U2Core/GObjectTypes.h>
#include <U2Core/DNASequenceObject.h>

#include <U2Core/TextUtils.h>

namespace U2 {

/* TRANSLATOR U2::RawDNASequenceFormat */
/* TRANSLATOR U2::IOAdapter */

RawDNASequenceFormat::RawDNASequenceFormat(QObject* p) : DocumentFormat(p, DocumentFormatFlags_W1)
{
    formatName = tr("Raw sequence");
    fileExtensions << "seq" << "txt";
    supportedObjectTypes+=GObjectTypes::SEQUENCE;
}


static void load(IOAdapter* io, QList<GObject*>& objects, TaskStateInfo& ti) {
    static int READ_BUFF_SIZE = 4096;

    QByteArray readBuffer(READ_BUFF_SIZE, '\0');
    char* buff  = readBuffer.data();

    int wholeSize = io->left();
    const QBitArray& ALPHAS = TextUtils::ALPHA_NUMS;

    QByteArray seq;
    if (wholeSize != -1) {
        seq.reserve(wholeSize);
    }

    //reading sequence
    QBuffer writer(&seq);
    writer.open( QIODevice::WriteOnly | QIODevice::Append );
    bool ok = true;
    int len = 0;
    while (ok && (len = io->readBlock(buff, READ_BUFF_SIZE)) > 0) {
        if (ti.cancelFlag) {
            break;
        }
        for (int i=0; i<len && ok; i++) {
            char c = buff[i];
            if (ALPHAS[(uchar)c]) {
                ok = writer.putChar(c);
            }
        }
        ti.progress = io->getProgress();
    }
    writer.close();
    if (ti.hasError()) {
        return;
    }
    if (seq.size() == 0) {
        ti.setError(RawDNASequenceFormat::tr("Seqeunce is empty"));
        return;
    }
    DNASequence dnaseq(seq);
    DocumentFormatUtils::addSequenceObject(objects, "Sequence", dnaseq);
}

Document* RawDNASequenceFormat::loadDocument(IOAdapter* io, TaskStateInfo& ti, const QVariantMap& fs, DocumentLoadMode) {
    QList<GObject*> objects;
    load(io, objects, ti);
    
    if (ti.hasError()) {
        return NULL;
    }

    Document* doc = new Document(this, io->getFactory(), io->getURL(), objects, fs);
    return doc;
}

FormatDetectionScore RawDNASequenceFormat::checkRawData(const QByteArray& rawData, const GUrl&) const {
    const char* data = rawData.constData();
    int size = rawData.size();
    if(QRegExp("[a-zA-Z\r\n]*").exactMatch(rawData)) {
        return FormatDetection_VeryHighSimilarity;
    }
    bool hasBinaryData = TextUtils::contains(TextUtils::BINARY, data, size);
    //returning 'very low chance' here just because it's impossible to have 100% detection for this format
    return hasBinaryData ? FormatDetection_NotMatched : FormatDetection_VeryLowSimilarity; 
}

void RawDNASequenceFormat::storeDocument( Document* d, TaskStateInfo& ts, IOAdapter* io) {
    assert(d->getObjects().size() ==1);
    GObject* obj = d->getObjects().first();
    DNASequenceObject* so = qobject_cast<DNASequenceObject*>(obj);
    assert(so!=NULL);
    PlainTextFormat::storeRawData(so->getDNASequence().seq, ts, io);
}


}//namespace
