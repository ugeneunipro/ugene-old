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

#define READ_BUFF_SIZE  4096
static void load(IOAdapter* io, const GUrl& docUrl, QList<GObject*>& objects, const QVariantMap& hints, TaskStateInfo& ti,
                 int gapSize, int predictedSize, QString& writeLockReason, DocumentLoadMode mode) {
    writeLockReason.clear();
    QByteArray readBuff(READ_BUFF_SIZE+1, 0);
    char* buff = readBuff.data();
    qint64 len = 0;
    static char fastaHeaderStartChar = '>';
    static QBitArray fastaHeaderStart = TextUtils::createBitMap(fastaHeaderStartChar);

    bool merge = gapSize!=-1;
    QByteArray sequence;
    QStringList headers;
    QSet<QString> names;
    QVector<U2Region> mergedMapping;
    QByteArray gapSequence((merge ? gapSize : 0), 0);

    sequence.reserve(predictedSize);

    //skip leading whites if present
    bool lineOk = true;
    static QBitArray nonWhites = ~TextUtils::WHITES;
    io->readUntil(buff, READ_BUFF_SIZE, nonWhites, IOAdapter::Term_Exclude, &lineOk);

    int sequenceStart = 0;
    while (!ti.cancelFlag) {
        //read header
        len = io->readUntil(buff, READ_BUFF_SIZE, TextUtils::LINE_BREAKS, IOAdapter::Term_Include, &lineOk);
        if (len == 0) { //end if stream
            break;
        }
        if (!lineOk) {
            ti.setError(FastaFormat::tr("Line is too long"));
        }
        QString headerLine = QString(QByteArray::fromRawData(buff+1, len-1)).trimmed();
        if (buff[0]!= fastaHeaderStartChar) {
            ti.setError(FastaFormat::tr("First line is not a FASTA header"));
        }

        //read sequence
        if (!merge) {
            sequence.clear();
        } else if (sequence.size() > 0) {
            sequence.append(gapSequence);
            sequenceStart = sequence.size();
        }
        do {
            len = io->readUntil(buff, READ_BUFF_SIZE, fastaHeaderStart, IOAdapter::Term_Exclude);
            if (len <= 0) {
                break;
            }
            len = TextUtils::remove(buff, len, TextUtils::WHITES);
            buff[len] = 0;
            sequence.append(buff);
            ti.progress = io->getProgress();
        } while (!ti.cancelFlag);

        if (merge) {
            headers.append(headerLine);
            mergedMapping.append(U2Region(sequenceStart, sequence.size() - sequenceStart));
        } else {
            DNASequence seq(headerLine, sequence);
            QString objName;
            if (mode != DocumentLoadMode_SingleObject) {
                objName = TextUtils::variate(headerLine, "_", names);
                names.insert(objName);
                //TODO parse header
                seq.info.insert(DNAInfo::FASTA_HDR, headerLine);
                seq.info.insert(DNAInfo::ID, headerLine);
            } else {
                objName = headerLine;
            }
            DocumentFormatUtils::addSequenceObject(objects, objName, seq, hints, ti);
            if (ti.hasError()) {
                qDeleteAll(objects);
                objects.clear();
                return;
            }
        }

        if( mode == DocumentLoadMode_SingleObject ) {
            break;
        }
    }

    assert(headers.size() == mergedMapping.size());

    if (!ti.hasError() && !ti.cancelFlag && merge && !headers.isEmpty()) {
        DocumentFormatUtils::addMergedSequenceObject(objects, docUrl, headers, sequence, mergedMapping, hints, ti);
    }
    if (!ti.hasError() && !ti.cancelFlag && objects.isEmpty()) {
        ti.setError(Document::tr("Document is empty."));
    }

    if (merge && headers.size() > 1) {
        writeLockReason = DocumentFormat::MERGED_SEQ_LOCK;
    }
}


Document* FastaFormat::loadDocument( IOAdapter* io, TaskStateInfo& ti, const QVariantMap& _fs, DocumentLoadMode mode) {
    if( NULL == io || !io->isOpen() ) {
        ti.setError(L10N::badArgument("IO adapter"));
        return NULL;
    }
    QVariantMap fs = _fs;
    QList<GObject*> objects;

    int gapSize = qBound(-1, DocumentFormatUtils::getMergeGap(fs), 1000*1000);
    int predictedSize = qMax(1000,
        DocumentFormatUtils::getIntSettings(fs, DocumentReadingMode_SequenceMergingFinalSizeHint, gapSize==-1 ? 0 : io->left()));

    QString lockReason;
    load(io, io->getURL(), objects, fs, ti, gapSize, predictedSize, lockReason, mode);
    if (ti.hasError() || ti.cancelFlag) {
        qDeleteAll(objects);
        objects.clear();
        return NULL;
    }

    DocumentFormatUtils::updateFormatSettings(objects, fs);
    Document* doc = new Document(this, io->getFactory(), io->getURL(), objects, fs, lockReason);
    return doc;
}

#define LINE_LEN 70
static void saveOneFasta( IOAdapter* io, const DNASequence& sequence, GObject*,  TaskStateInfo& tsi ) {
    //writing header;

    // TODO better header out of info tags
    /*QString hdr = seqObj->getDNASequence().info.value(DNAInfo::FASTA_HDR).toString();
    if (hdr.isEmpty()) {
        hdr = seqObj->getGObjectName();
    }*/

    QByteArray block;
    QString hdr = sequence.getName();
    block.append('>').append(hdr).append( '\n' );
    if (io->writeBlock( block ) != block.length()) {
        tsi.setError(L10N::errorWritingFile(io->getURL()));
        return;
    }
    const char* seq = sequence.seq.constData();
    int len = sequence.seq.length();
    for (int i = 0; i < len; i += LINE_LEN ) {
        int chunkSize = qMin( LINE_LEN, len - i );
        if (io->writeBlock( seq + i, chunkSize ) != chunkSize || !io->writeBlock( "\n", 1 )) {
            tsi.setError(L10N::errorWritingFile(io->getURL()));
            return;
        }
    }
}


void FastaFormat::storeDocument( Document* doc, TaskStateInfo& ts, IOAdapter* io ) {
    //TODO: check saved op states!!!
    foreach( GObject* o, doc->getObjects() ) {
        QList<DNASequence> sequences = DocumentFormatUtils::toSequences(o);
        foreach(const DNASequence& s, sequences) {
            saveOneFasta( io, s, o, ts );
            if (ts.isCoR()) {
                break;
            }
        }
    }
}

GObject *FastaFormat::loadObject(IOAdapter *io, TaskStateInfo &ti) {
    DNASequence *seq = loadSequence(io, ti);
    if (ti.hasError()) {
        return NULL;
    }

    return new DNASequenceObject(seq->getName(), *seq);
}

DNASequence *FastaFormat::loadSequence(IOAdapter* io, TaskStateInfo& ti) {
    static char fastaHeaderStartChar = '>';
    static QBitArray fastaHeaderStart = TextUtils::createBitMap(fastaHeaderStartChar);
    static QBitArray nonWhites = ~TextUtils::WHITES;

    if( NULL == io || !io->isOpen() ) {
        ti.setError(L10N::badArgument("IO adapter"));
        return NULL;
    }
    QByteArray readBuff(READ_BUFF_SIZE+1, 0);
    char* buff = readBuff.data();
    qint64 len = 0;

    //skip leading whites if present
    bool lineOk = true;
    io->readUntil(buff, READ_BUFF_SIZE, nonWhites, IOAdapter::Term_Exclude, &lineOk);

    //read header
    len = io->readUntil(buff, READ_BUFF_SIZE, TextUtils::LINE_BREAKS, IOAdapter::Term_Include, &lineOk);
    if (len == 0) { //end of stream
        return NULL;
    }
    if (!lineOk) {
        ti.setError(FastaFormat::tr("Line is too long"));
    }
    QByteArray headerLine = QByteArray(buff + 1, len-1).trimmed();
    if (buff[0]!= fastaHeaderStartChar) {
        ti.setError(FastaFormat::tr("First line is not a FASTA header"));
    }

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
    } while (!ti.cancelFlag);
    
    DNASequence *seq = new DNASequence(headerLine, sequence);
    seq->alphabet = AppContext::getDNAAlphabetRegistry()->findById(BaseDNAAlphabetIds::NUCL_DNA_EXTENDED());
    assert(seq->alphabet!=NULL);

    if (!seq->alphabet->isCaseSensitive()) {
        TextUtils::translate(TextUtils::UPPER_CASE_MAP, const_cast<char*>(seq->seq.constData()), seq->seq.length());
    }

    return seq;
}

void FastaFormat::storeSequence( const DNASequence& sequence, IOAdapter* io, TaskStateInfo& ti )
{
    saveOneFasta(io, sequence, NULL, ti);
}

}//namespace
