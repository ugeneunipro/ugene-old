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

#include "FastaFormat.h"

#include "DocumentFormatUtils.h"

#include <U2Core/Task.h>
#include <U2Core/IOAdapter.h>
#include <U2Core/DNAAlphabet.h>
#include <U2Core/L10n.h>

#include <U2Core/DNASequenceObject.h>
#include <U2Core/MAlignmentObject.h>
#include <U2Core/AnnotationTableObject.h>
#include <U2Core/GObjectTypes.h>
#include <U2Core/TextUtils.h>
#include <U2Core/AppContext.h>
#include <U2Core/U2SafePoints.h>
#include <U2Core/U2DbiUtils.h>
#include <U2Core/U2SequenceUtils.h>
#include <U2Core/U2AttributeDbi.h>
#include <U2Core/U1AnnotationUtils.h>

namespace U2 {

/* TRANSLATOR U2::FastaFormat */
/* TRANSLATOR U2::IOAdapter */
/* TRANSLATOR U2::Document */

FastaFormat::FastaFormat(QObject* p) 
: DocumentFormat(p, DocumentFormatFlags_SW, QStringList()<<"fa"<<"mpfa"<<"fna"<<"fsa"<<"fas"<<"fasta"<<"sef"<<"seq"<<"seqs")
{
    formatName = tr("FASTA");
    supportedObjectTypes+=GObjectTypes::SEQUENCE;
    supportedObjectTypes+=GObjectTypes::MULTIPLE_ALIGNMENT;
    formatDescription = tr("FASTA format is a text-based format for representing either nucleotide sequences or peptide sequences, in which base pairs or amino acids are represented using single-letter codes. The format also allows for sequence names and comments to precede the sequences.");
}

static QVariantMap analyzeRawData(const QByteArray& data) {
    int hasGaps = false;
    int minLen = -1;
    int maxLen = -1;
    int len = 0;
    int nSequences = 0;
    QTextStream input(data, QIODevice::ReadOnly);
    QString line;
    do {
        line = input.readLine();
        if (line[0] == '>') {
            nSequences++;
            if (len > 0) {
                minLen = minLen == -1 ? len : qMin(minLen, len);
                maxLen = maxLen == -1 ? len : qMax(maxLen, len);
            }
            len = 0;
        } else {
            len += line.length();
            if (!hasGaps && line.contains(MAlignment_GapChar)) {
                hasGaps = true;
            }
        }
    } while (!line.isEmpty());

    QVariantMap res;
    res[RawDataCheckResult_Sequence] = true;
    if (hasGaps) {
        res[RawDataCheckResult_SequenceWithGaps] = true;
    }
    if (nSequences > 1) {
        res[RawDataCheckResult_MultipleSequences] = true;
    }
    if (minLen > 0) {
        res[RawDataCheckResult_MaxSequenceSize] = maxLen;
        res[RawDataCheckResult_MinSequenceSize] = minLen;
    }
    return res;
}

FormatCheckResult FastaFormat::checkRawData(const QByteArray& rawData, const GUrl&) const {
    const char* data = rawData.constData();
    int size = rawData.size();

    int n = TextUtils::skip(TextUtils::WHITES, data, size);
    int newSize = size - n;
    const char* newData = data + n;
    if (newSize <= 0 || newData[0] != '>' ) {
        return FormatDetection_NotMatched;
    }
    bool hasBinaryBlocks = TextUtils::contains(TextUtils::BINARY, data, size);
    if (hasBinaryBlocks) {
        return FormatDetection_NotMatched;
    }
    
    //ok, format is matched -> add hints on sequence sizes
    FormatCheckResult res(FormatDetection_Matched);
    res.properties = analyzeRawData(data);
    return res;
}

#define GObjectHint_CaseAnns   "use-case-annotations"
#define READ_BUFF_SIZE  4096
static void load(IOAdapter* io, const U2DbiRef& dbiRef, const QVariantMap& fs, QList<GObject*>& objects,
                 int gapSize, QString& writeLockReason, U2OpStatus& os) 
{
    static char fastaHeaderStartChar = '>';
    static QBitArray fastaHeaderStart = TextUtils::createBitMap(fastaHeaderStartChar);

    writeLockReason.clear();
    GUrl docUrl = io->getURL();
    QByteArray readBuff(READ_BUFF_SIZE + 1, 0);
    char* buff = readBuff.data();
    qint64 len = 0;
    
    bool merge = gapSize != -1;
    QStringList headers;
    QSet<QString> names;
    QVector<U2Region> mergedMapping;

    TmpDbiObjects dbiObjects(dbiRef, os);

    // for lower case annotations
    GObjectReference sequenceRef;
    
    //skip leading whites if present
    bool lineOk = true;
    static QBitArray nonWhites = ~TextUtils::WHITES;
    io->readUntil(buff, READ_BUFF_SIZE, nonWhites, IOAdapter::Term_Exclude, &lineOk);

    U2SequenceImporter seqImporter;
    if (fs.keys().contains(GObjectHint_CaseAnns)) {
        CaseAnnotationsMode mode = qVariantValue<CaseAnnotationsMode>(fs.value(GObjectHint_CaseAnns, NO_CASE_ANNS));
        seqImporter.setCaseAnnotationsMode(mode);
    }

    qint64 sequenceStart = 0;
    int sequenceNumber = 0;
    while (!os.isCoR()) {
        //read header
        len = io->readUntil(buff, READ_BUFF_SIZE, TextUtils::LINE_BREAKS, IOAdapter::Term_Include, &lineOk);
        if (len == 0) { //end if stream
            break;
        }
        CHECK_EXT(lineOk, os.setError(FastaFormat::tr("Line is too long")), ); 
        
        QString headerLine = QString(QByteArray::fromRawData(buff+1, len-1)).trimmed();
        CHECK_EXT(buff[0] == fastaHeaderStartChar, os.setError(FastaFormat::tr("First line is not a FASTA header")), ); 
        
        //read sequence
        if (sequenceNumber == 0 || !merge) {
            QString objName = merge ? "Sequence" : TextUtils::variate(headerLine, "_", names);
            names.insert(objName);
            seqImporter.startSequence(dbiRef, objName, false, os);
            CHECK_OP(os, );

            sequenceRef = GObjectReference(io->getURL().getURLString(), objName, GObjectTypes::SEQUENCE);
        } 
        if (sequenceNumber >= 1 && merge) {
            seqImporter.addDefaultSymbolsBlock(gapSize, os);
            sequenceStart += gapSize;
            CHECK_OP(os, );
        }
        int sequenceLen = 0;
        while (!os.isCoR()) {
            len = io->readUntil(buff, READ_BUFF_SIZE, fastaHeaderStart, IOAdapter::Term_Exclude);
            if (len <= 0) {
                break;
            }
            len = TextUtils::remove(buff, len, TextUtils::WHITES);
            buff[len] = 0;

            seqImporter.addBlock(buff, len, os);
            sequenceLen += len;
            CHECK_OP(os, );
            os.setProgress(io->getProgress());
        } 

        if (merge) {
            headers.append(headerLine);
            mergedMapping.append(U2Region(sequenceStart, sequenceLen));
        } else {
            U2Sequence seq = seqImporter.finalizeSequence(os);
            CHECK_OP(os, );
            dbiObjects.objects << seq.id;
            
            //TODO parse header
            DbiConnection con(dbiRef, os);
            U2StringAttribute attr(seq.id, DNAInfo::FASTA_HDR, headerLine);
            con.dbi->getAttributeDbi()->createStringAttribute(attr, os);
            CHECK_OP(os, );
            
            objects << new U2SequenceObject(seq.visualName, U2EntityRef(dbiRef, seq.id));
            CHECK_OP(os, );

            U1AnnotationUtils::addAnnotations(objects, seqImporter.getCaseAnnotations(), sequenceRef, NULL);
        }
        sequenceStart += sequenceLen;
        sequenceNumber++;
    }
    CHECK_OP(os, );
    CHECK_EXT(!objects.isEmpty() || merge, os.setError(Document::tr("Document is empty.")), );
    SAFE_POINT(headers.size() == mergedMapping.size(), "headers <-> regions mapping failed!", );

    if (!merge) {
        return;
    }

    U2Sequence seq = seqImporter.finalizeSequence(os);
    CHECK_OP(os, );
    dbiObjects.objects << seq.id;

    U1AnnotationUtils::addAnnotations(objects, seqImporter.getCaseAnnotations(), sequenceRef, NULL);
    objects << new U2SequenceObject(seq.visualName, U2EntityRef(dbiRef, seq.id));
    objects << DocumentFormatUtils::addAnnotationsForMergedU2Sequence(docUrl, headers, seq, mergedMapping, os);
    if (headers.size() > 1) {
        writeLockReason = DocumentFormat::MERGED_SEQ_LOCK;
    }
}


Document* FastaFormat::loadDocument(IOAdapter* io, const U2DbiRef& dbiRef, const QVariantMap& _fs, U2OpStatus& os) {
    CHECK_EXT(io!=NULL && io->isOpen(), os.setError(L10N::badArgument("IO adapter")), NULL);

    QVariantMap fs = _fs;
    QList<GObject*> objects;

    int gapSize = qBound(-1, DocumentFormatUtils::getMergeGap(fs), 1000 * 1000);
    
    QString lockReason;
    load(io, dbiRef, _fs, objects, gapSize, lockReason, os);
    CHECK_OP_EXT(os, qDeleteAll(objects), NULL);

    Document* doc = new Document(this, io->getFactory(), io->getURL(), dbiRef, dbiRef.isValid(), objects, fs, lockReason);
    return doc;
}

#define SAVE_LINE_LEN 70
static void saveSequence(IOAdapter* io, const DNASequence& sequence, GObject*,  U2OpStatus& os) {
    //writing header;

    // TODO better header out of info tags
    /*QString hdr = seqObj->getDNASequence().info.value(DNAInfo::FASTA_HDR).toString();
    if (hdr.isEmpty()) {
        hdr = seqObj->getGObjectName();
    }*/

    QByteArray block;
    QString hdr = sequence.getName();
    block.append('>').append(hdr).append('\n');
    if (io->writeBlock( block ) != block.length()) {
        os.setError(L10N::errorWritingFile(io->getURL()));
        return;
    }
    const char* seq = sequence.seq.constData();
    int len = sequence.seq.length();
    for (int i = 0; i < len; i += SAVE_LINE_LEN ) {
        int chunkSize = qMin( SAVE_LINE_LEN, len - i );
        if (io->writeBlock( seq + i, chunkSize ) != chunkSize || !io->writeBlock( "\n", 1 )) {
            os.setError(L10N::errorWritingFile(io->getURL()));
            return;
        }
    }
}


void FastaFormat::storeDocument( Document* doc, IOAdapter* io, U2OpStatus& os ) {
    //TODO: check saved op states!!!
    foreach( GObject* o, doc->getObjects() ) {
        QList<DNASequence> sequences = DocumentFormatUtils::toSequences(o);
        foreach(const DNASequence& s, sequences) {
            saveSequence(io, s, o, os);
            CHECK_OP(os, );
        }
    }
}


DNASequence *FastaFormat::loadSequence(IOAdapter* io, U2OpStatus& os) {
    static char fastaHeaderStartChar = '>';
    static QBitArray fastaHeaderStart = TextUtils::createBitMap(fastaHeaderStartChar);
    static QBitArray nonWhites = ~TextUtils::WHITES;

    CHECK_EXT(io != NULL && io->isOpen(), os.setError(L10N::badArgument("IO adapter")), NULL);
    
    QByteArray readBuff(READ_BUFF_SIZE+1, 0);
    char* buff = readBuff.data();
    qint64 len = 0;

    //skip leading whites if present
    bool lineOk = true;
    io->readUntil(buff, READ_BUFF_SIZE, nonWhites, IOAdapter::Term_Exclude, &lineOk);

    //read header
    len = io->readUntil(buff, READ_BUFF_SIZE, TextUtils::LINE_BREAKS, IOAdapter::Term_Include, &lineOk);
    CHECK(len > 0, NULL); //end of stream
    CHECK_EXT(lineOk, os.setError(FastaFormat::tr("Line is too long")), NULL);
    QByteArray headerLine = QByteArray(buff + 1, len-1).trimmed();
    CHECK_EXT(buff[0] == fastaHeaderStartChar, os.setError(FastaFormat::tr("First line is not a FASTA header")), NULL);
    
    //read sequence
    QByteArray sequence;
    int predictedSize = 1000;
    sequence.reserve(predictedSize);
    do {
        len = io->readUntil(buff, READ_BUFF_SIZE, fastaHeaderStart, IOAdapter::Term_Exclude);
        if (len <= 0) {
            break;
        }
        len = TextUtils::remove(buff, len, TextUtils::WHITES);
        buff[len] = 0;
        sequence.append(buff);
    } while (!os.isCoR());
    
    DNASequence *seq = new DNASequence(headerLine, sequence);
    seq->alphabet = AppContext::getDNAAlphabetRegistry()->findById(BaseDNAAlphabetIds::NUCL_DNA_EXTENDED());
    assert(seq->alphabet!=NULL);

    if (!seq->alphabet->isCaseSensitive()) {
        TextUtils::translate(TextUtils::UPPER_CASE_MAP, const_cast<char*>(seq->seq.constData()), seq->seq.length());
    }

    return seq;
}

void FastaFormat::storeSequence(const DNASequence& sequence, IOAdapter* io, U2OpStatus& os) {
    saveSequence(io, sequence, NULL, os);
}

}//namespace
