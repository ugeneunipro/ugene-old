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

#include <U2Core/IOAdapter.h>
#include <U2Core/L10n.h>
#include <U2Core/DNASequenceObject.h>
#include <U2Core/AnnotationTableObject.h>
#include <U2Core/GObjectTypes.h>
#include <U2Core/TextUtils.h>
#include <U2Core/U2OpStatus.h>
#include <U2Core/U2SafePoints.h>
#include <U2Core/U2AlphabetUtils.h>
#include <U2Core/U2DbiUtils.h>
#include <U2Core/U2SequenceUtils.h>
#include <U2Core/U1AnnotationUtils.h>

#include "DocumentFormatUtils.h"
#include "FastqFormat.h"

/* TRANSLATOR U2::FastqFormat */

namespace U2 {

FastqFormat::FastqFormat(QObject* p) 
: DocumentFormat(p, DocumentFormatFlags_SW, QStringList("fastq")), fn(tr("FASTQ")) 
{
    supportedObjectTypes+=GObjectTypes::SEQUENCE;
    formatDescription  = tr("FASTQ format is a text-based format for storing both a biological sequence (usually nucleotide sequence) and its corresponding quality scores. \
Both the sequence letter and quality score are encoded with a single ASCII character for brevity. \
It was originally developed at the Wellcome Trust Sanger Institute to bundle a FASTA sequence and its quality data, \
but has recently become the de facto standard for storing the output of high throughput sequencing instruments.");
}

FormatCheckResult FastqFormat::checkRawData(const QByteArray& rawData, const GUrl&) const {
    const char* data = rawData.constData();
    int size = rawData.size();

    if (size <= 0 || data[0] != '@' ) {
        return FormatDetection_NotMatched;
    }
    bool hasBinaryBlocks = TextUtils::contains(TextUtils::BINARY, data, size);
    if (hasBinaryBlocks){
        return FormatDetection_NotMatched;
    }
    FormatCheckResult res(FormatDetection_AverageSimilarity);
    // here we propagate some property values without actual parsing
    // this must be fixed some day
    res.properties[RawDataCheckResult_Sequence] = true;
    res.properties[RawDataCheckResult_SequenceWithGaps] = false;
    res.properties[RawDataCheckResult_MultipleSequences] = true;
    res.properties[RawDataCheckResult_MinSequenceSize] = 10;
    res.properties[RawDataCheckResult_MaxSequenceSize] = 1000;
    return res;
}


#define BUFF_SIZE  4096

static bool readLine(QByteArray& target, IOAdapter* io, U2OpStatus& os, bool last = false, int * howManyRead = NULL) {
    bool lineOK = false;
    qint64 len, total = target.size();
    do
    {
        if (target.capacity() - total < BUFF_SIZE) {
            target.reserve(target.capacity() + BUFF_SIZE);
        }
        char* buff = target.data() + total;
        len = io->readUntil(buff, BUFF_SIZE, TextUtils::LINE_BREAKS, IOAdapter::Term_Exclude, &lineOK);
        if(howManyRead != NULL) { *howManyRead += len; }
        os.setProgress(io->getProgress());
        total += len;
    } while (!os.isCoR() && !lineOK && len == BUFF_SIZE);
    if (lineOK) {
        target.resize(total);
        //put back start of another line
        //io->skip(len - BUFF_SIZE);
        //eat trailing white
        char ch;
        bool gotChar = io->getChar(&ch);
        bool get_white = gotChar && TextUtils::LINE_BREAKS[uchar(ch)];
        if(gotChar && howManyRead != NULL) {*howManyRead = *howManyRead + 1;}
        assert(get_white);
        lineOK = get_white;
    } else if (!last) {
        os.setError(U2::FastqFormat::tr("Unexpected end of file"));
    }
    return lineOK;
}

static const QByteArray SEQ_QUAL_SEPARATOR = "+";

// reads until new line, starting with separator
// separator not included in read data
static bool readUntil(QByteArray & target, IOAdapter * io, U2OpStatus & ti, const QByteArray & separator) {
    QByteArray currentLine;
    while(1) {
        currentLine.clear();
        int howMany = 0;
        readLine(currentLine, io, ti, false, &howMany);
        if( ti.hasError() ) {
            return false;
        } else if( currentLine.startsWith(separator) ) {
            io->skip(-1 * (howMany));
            break;
        } else if( currentLine.isEmpty() ) {
            ti.setError( "Unexpected end of file" );
            return false;
        }
        
        target.append(currentLine.trimmed());
    }
    return true;
}

static bool readBlock( QByteArray & block, IOAdapter * io, U2OpStatus & ti, qint64 size ) {
    assert(size >= 0);
    while( block.size() < size ) {
        QByteArray curBlock;
        readLine(curBlock, io, ti);
        if(ti.hasError()) {
            return false;
        } else if( curBlock.isEmpty() ) {
            ti.setError( "Unexpected end of file" );
            return false;
        }
        
        block.append(curBlock);
    }
    return true;
}

/**
 * FASTQ format specification: http://maq.sourceforge.net/fastq.shtml
 */
static void load(IOAdapter* io, const U2DbiRef& dbiRef, const QVariantMap& hints, const GUrl& docUrl, QList<GObject*>& objects, U2OpStatus& os,
                 int gapSize, int predictedSize, QString& writeLockReason) {
     writeLockReason.clear();
     QByteArray readBuff, secondBuff;

     bool merge = gapSize!=-1;
     QByteArray sequence;
     QByteArray qualityScores;
     QStringList headers;
     QSet<QString> names;
     QVector<U2Region> mergedMapping;
     QByteArray gapSequence((merge ? gapSize : 0), 0);
     sequence.reserve(predictedSize);
     qualityScores.reserve(predictedSize);

     TmpDbiObjects dbiObjects(dbiRef, os);

     // for lower case annotations
     GObjectReference sequenceRef;

     qint64 sequenceStart = 0;

     U2SequenceImporter seqImporter(hints);

     int seqNumber = 0;

     while (!os.isCoR()) {
         //read header
         readBuff.clear();
         if (!readLine(readBuff, io, os, (merge && !headers.isEmpty()) || !names.isEmpty())) {
             break;
         }
         if (readBuff[0]!= '@') {
             os.setError(U2::FastqFormat::tr("Not a valid FASTQ file: %1. The @ identifier is not found.").arg(docUrl.getURLString()));
             break;
         }

         QString headerLine = QString::fromLatin1(readBuff.data()+1, readBuff.length()-1);

         if (merge == false   || seqNumber == 0){		
                QString objName = (merge) ? "Sequence" : TextUtils::variate(headerLine, "_", names);
                names.insert(objName);
                seqImporter.startSequence(dbiRef,objName,false,os);
                CHECK_OP(os,);
                sequenceRef = GObjectReference(io->getURL().getURLString(), objName, GObjectTypes::SEQUENCE);
         }

         //read sequence
         if (merge && sequence.length() > 0) {
                    seqImporter.addDefaultSymbolsBlock(gapSize,os);
                    sequenceStart += sequence.length();
                    sequenceStart+=gapSize;
                    CHECK_OP(os,);
         }
         sequence.clear();  
         if(!readUntil(sequence, io, os, SEQ_QUAL_SEPARATOR)) {
            break;
         }
         seqImporter.addBlock(sequence.data(),sequence.length(),os);
         CHECK_OP(os,);

         // read +<seqname>
         secondBuff.clear();
         secondBuff.reserve(readBuff.length());
         if (!readLine(secondBuff, io, os)) {
             break;
         }
         if (secondBuff[0]!= '+' || (secondBuff.length() != 1 && secondBuff.length() != readBuff.length() )
             || (readBuff.length()  == secondBuff.length() && strncmp(readBuff.data()+1, secondBuff.data()+1, readBuff.length() - 1))) {
             os.setError(U2::FastqFormat::tr("Not a valid FASTQ file: %1").arg(docUrl.getURLString()));
             break;
         }
         
         // read qualities
         qualityScores.clear();
         
         if( !readBlock(qualityScores, io, os, sequence.length()) ) {
            break;
         }
         
         if ( qualityScores.length() != sequence.length()) {
             os.setError(U2::FastqFormat::tr("Not a valid FASTQ file: %1. Bad quality scores: inconsistent size.").arg(docUrl.getURLString()));   
         }
         seqNumber++;
         if (merge) {
             headers.append(headerLine);
             mergedMapping.append(U2Region(sequenceStart, sequence.length() ));
         } else {
             U2Sequence u2seq = seqImporter.finalizeSequence(os);
             CHECK_OP(os,);

             U2SequenceObject* seqObj = new U2SequenceObject(u2seq.visualName, U2EntityRef(dbiRef, u2seq.id));

             objects << seqObj;
             dbiObjects.objects << seqObj->getSequenceRef().entityId;
             seqObj->setQuality(DNAQuality(qualityScores));

             U1AnnotationUtils::addAnnotations(objects, seqImporter.getCaseAnnotations(), sequenceRef, NULL);
         }
     }

     CHECK_OP(os, );
     CHECK_EXT(!objects.isEmpty() || merge, os.setError(Document::tr("Document is empty.")), );
     SAFE_POINT(headers.size() == mergedMapping.size(), "headers <-> regions mapping failed!", );

     if (!merge) {
         return;
     }
     U2Sequence u2seq = seqImporter.finalizeSequence(os);
     CHECK_OP(os,);

     dbiObjects.objects << u2seq.id;

     U1AnnotationUtils::addAnnotations(objects, seqImporter.getCaseAnnotations(), sequenceRef, NULL);
     objects << new U2SequenceObject(u2seq.visualName, U2EntityRef(dbiRef, u2seq.id));
     objects << DocumentFormatUtils::addAnnotationsForMergedU2Sequence(docUrl, headers, u2seq, mergedMapping, os);
     if (headers.size() > 1) {
         writeLockReason = DocumentFormat::MERGED_SEQ_LOCK;
     }
}

Document* FastqFormat::loadDocument(IOAdapter* io, const U2DbiRef& dbiRef, const QVariantMap& _hints, U2OpStatus& os) {
    CHECK_EXT(io != NULL && io->isOpen(), os.setError(L10N::badArgument("IO adapter")), NULL);
    QVariantMap hints = _hints;
    QList<GObject*> objects;

    int gapSize = qBound(-1, DocumentFormatUtils::getMergeGap(_hints), 1000*1000);
    int predictedSize = qMax(100*1000, DocumentFormatUtils::getMergedSize(hints, gapSize==-1 ? 0 : io->left()));

    QString lockReason;
    load(io, dbiRef, _hints, io->getURL(), objects, os, gapSize, predictedSize, lockReason);

    CHECK_OP_EXT(os, qDeleteAll(objects), NULL);
    DocumentFormatUtils::updateFormatHints(objects, hints);
    Document* doc = new Document(this, io->getFactory(), io->getURL(), dbiRef, dbiRef.isValid(), objects, hints, lockReason );
    return doc;
}

#define SAVE_LINE_LEN 70

void FastqFormat::storeDocument( Document* d, IOAdapter* io, U2OpStatus& os) {
    foreach (GObject* obj, d->getObjects()) {
        U2SequenceObject* seqObj = qobject_cast< U2SequenceObject* >( obj);
        storeEntry(io, seqObj, QList<GObject*>(), os);
        CHECK_OP(os, );
    }
}

void FastqFormat::storeEntry(IOAdapter *io, U2SequenceObject *seqObj, const QList<GObject*> &anns, U2OpStatus &os) {
    Q_UNUSED(anns);

    CHECK_EXT(seqObj!=NULL, os.setError(L10N::badArgument("NULL sequence" )), );
    try {
        //write header;
        QByteArray block;

        QString hdr = seqObj->getGObjectName();
        block.append('@').append(hdr).append( '\n' );
        if (io->writeBlock( block ) != block.length()) {
            throw 0;
        }
        // write sequence
        DNASequence wholeSeq = seqObj->getWholeSequence();
        const char* seq = wholeSeq.constData();

        int len = wholeSeq.length();
        for (int i = 0; i < len; i += SAVE_LINE_LEN ) {
            int chunkSize = qMin( SAVE_LINE_LEN, len - i );
            if (io->writeBlock( seq + i, chunkSize ) != chunkSize
                || !io->writeBlock( "\n", 1 )) {
                    throw 0;
            }
        }

        //write transition
        block.clear();
        block.append("+\n");

        if (io->writeBlock( block ) != block.length()) {
            throw 0;
        }

        //write quality
        QByteArray buf;
        const char* quality = NULL;
        if (wholeSeq.hasQualityScores()) {
            quality = wholeSeq.quality.qualCodes.constData();
            len = wholeSeq.quality.qualCodes.length();
        } else {
            // record the highest possible quality
            buf.fill('I',len);
            quality = buf.constData();
        } 

        for (int i = 0; i < len; i += SAVE_LINE_LEN ) {
            int chunkSize = qMin( SAVE_LINE_LEN, len - i );
            if (io->writeBlock( quality + i, chunkSize ) != chunkSize || !io->writeBlock( "\n", 1 )) {
                throw 0;
            }
        }

    } catch (int) {
        GUrl url = seqObj->getDocument() ? seqObj->getDocument()->getURL() : GUrl();
        os.setError(L10N::errorWritingFile(url));
    }
}

DNASequence *FastqFormat::loadSequence(IOAdapter* io, U2OpStatus& ti) {
    if( NULL == io || !io->isOpen() ) {
        ti.setError(L10N::badArgument("IO adapter"));
        return NULL;
    }

    QByteArray readBuff, secondBuff;
    QByteArray sequence;
    QByteArray qualityScores;
    int predictedSize = 1000;
    sequence.reserve(predictedSize);
    qualityScores.reserve(predictedSize);

    //read header
    if (!readLine(readBuff, io, ti)) {
        return NULL;
    }
    if (readBuff[0]!= '@') {
        ti.setError("Not a valid FASTQ file. The @ identifier is not found.");
        return NULL;
    }

    //read sequence
    if(!readUntil(sequence, io, ti, SEQ_QUAL_SEPARATOR)) {
        return NULL;
    }
    int seqLen = sequence.size();

    // read +<seqname>
    secondBuff.reserve(readBuff.size());
    if (!readLine(secondBuff, io, ti)) {
        return NULL;
    }
    if (secondBuff[0]!= '+' || (secondBuff.size() != 1 && secondBuff.size() != readBuff.size())
     || (readBuff.size() == secondBuff.size() && strncmp(readBuff.data()+1, secondBuff.data()+1, readBuff.size() - 1))) {
        ti.setError("Not a valid FASTQ file");
        return NULL;
    }

    // read qualities
    if(!readBlock(qualityScores, io, ti, seqLen) ) {
        return NULL;
    }

    if ( qualityScores.length() != sequence.length() ) {
        ti.setError("Not a valid FASTQ file. Bad quality scores: inconsistent size.");
    }

    QByteArray headerLine = QByteArray::fromRawData(readBuff.data()+1, readBuff.length()-1);
    DNASequence *seq = new DNASequence(headerLine, sequence);
    seq->quality = DNAQuality(qualityScores);
    seq->alphabet = U2AlphabetUtils::getById(BaseDNAAlphabetIds::NUCL_DNA_EXTENDED());
    assert(seq->alphabet!=NULL);

    if (!seq->alphabet->isCaseSensitive()) {
        TextUtils::translate(TextUtils::UPPER_CASE_MAP, const_cast<char*>(seq->seq.constData()), seq->seq.length());
    }

    return seq;
}

}//namespace
