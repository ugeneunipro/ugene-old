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

#include <U2Core/BaseDocumentFormats.h>
#include <U2Core/DocumentModel.h>
#include <U2Core/AppContext.h>
#include <U2Core/IOAdapter.h>
#include <U2Core/U2OpStatusUtils.h>

#include <U2Formats/SAMFormat.h>
#include <U2Formats/FastaFormat.h>

#include "StreamSequenceWriter.h"

namespace U2 {


StreamShortReadsWriter::StreamShortReadsWriter(const GUrl& url, const QString& refName , int refLength ) 
: numSeqWritten(0), refSeqLength(refLength)
{
    refSeqName = QString(refName).replace(QRegExp("\\s|\\t"), "_").toAscii();

    IOAdapterFactory* iof = AppContext::getIOAdapterRegistry()->getIOAdapterFactoryById(BaseIOAdapters::LOCAL_FILE);
    io = iof->createIOAdapter();
    bool res = io->open(url, IOAdapterMode_Write);
    assert(res == true);
    Q_UNUSED(res);

}

bool StreamShortReadsWriter::writeNextAlignedRead( int offset, const DNASequence& seq ) {
    bool writeOk = format.storeAlignedRead(offset, seq, io, refSeqName, refSeqLength, numSeqWritten == 0);
    if (writeOk) {
        ++numSeqWritten;
        return true;
    }

    return false;

}

void StreamShortReadsWriter::close() {
    io->close();
}

StreamShortReadsWriter::~StreamShortReadsWriter() {
    delete io;
}


StreamShortReadWriter::StreamShortReadWriter( bool writeQualityExplicitly) : writeQuality(writeQualityExplicitly) {
    DocumentFormat* df = AppContext::getDocumentFormatRegistry()->getFormatById(BaseDocumentFormats::FASTA);
    fastaFormat = qobject_cast<FastaFormat*> (df);
    assert(fastaFormat != NULL);

    IOAdapterFactory* iof = AppContext::getIOAdapterRegistry()->getIOAdapterFactoryById(BaseIOAdapters::LOCAL_FILE);
    io = iof->createIOAdapter();
}


bool StreamShortReadWriter::init( const GUrl& url ) {
    ouputPath = url;
    bool res = io->open(url, IOAdapterMode_Write);
    return res;   
}

bool StreamShortReadWriter::writeNextSequence( const DNASequence& seq ) {   
    U2OpStatus2Log os;
    fastaFormat->storeSequence(seq, io, os);
    
    return !os.hasError();
}

void StreamShortReadWriter::close() {
    io->close();
}

} //namespace 

