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

    int sequenceCount = 0;
    int from = 0;
    while (from>=0) {
        // DNA sequence
        from = rawData.indexOf('@', from);
        if (from>=0) {
            sequenceCount++;
        }
        else {
            break;
        }

        // Quality sequence
        from = rawData.indexOf('+', from);
        sequenceCount = from>=0 ? sequenceCount+1 : sequenceCount;
    }
    sequenceCount /= 2;

    bool hasBinaryBlocks = TextUtils::contains(TextUtils::BINARY, data, size);
    if (hasBinaryBlocks || (sequenceCount == 0)) {
        return FormatDetection_NotMatched;
    }

    FormatCheckResult res(FormatDetection_HighSimilarity);
    res.properties[RawDataCheckResult_Sequence] = true;
    res.properties[RawDataCheckResult_MultipleSequences] = sequenceCount > 1;
    res.properties[RawDataCheckResult_SequenceWithGaps] = false;
    res.properties[RawDataCheckResult_MinSequenceSize] = 10;
    res.properties[RawDataCheckResult_MaxSequenceSize] = 1000;
    return res;
}

#define BUFF_SIZE  4096

static QString readSequenceName(U2OpStatus& os, IOAdapter *io, char beginWith = '@') {

    QByteArray buffArray(BUFF_SIZE+1, 0);
    char* buff = buffArray.data();

    static const QString errorMessage = U2::FastqFormat::tr("Error while trying to find sequence name start");

    bool sequenceNameStartFound = false;
    int readedCount = 0;
    while ((readedCount == 0) && !io->isEof()) { // skip \ns
        readedCount = io->readLine(buff, BUFF_SIZE, &sequenceNameStartFound);
    }
    CHECK_EXT(io->isEof() == false,,"");
    CHECK_EXT(readedCount >= 0, os.setError(errorMessage), "");

    buffArray.resize(readedCount);
    buffArray = buffArray.trimmed();
    CHECK_EXT((buffArray.size() > 0) && (buff[0] == beginWith), os.setError(errorMessage), "");

    QString sequenceName = QString::fromLatin1(buff+1, buffArray.size()-1);
    return sequenceName;
}

static bool checkFirstSymbol(const QByteArray& b, char symbol) {
    return (b.size()>0 && b.data()[0] == symbol);
}

static void readSequence(U2OpStatus& os, IOAdapter *io, QByteArray &sequence, char readUntil = '+') {

    QByteArray buffArray(BUFF_SIZE+1, 0);
    char* buff = buffArray.data();

    // reading until readUntil symbol i.e. quality or dna sequence name start, ignoring whitespace at the beginning and the end of lines

    while (!io->isEof()) {
        bool eolnFound = false;
        int readedCount = io->readUntil(buff, BUFF_SIZE, TextUtils::LINE_BREAKS, IOAdapter::Term_Include, &eolnFound);
        CHECK_EXT(readedCount >= 0, os.setError(U2::FastqFormat::tr("Error while reading sequence")),);

        QByteArray trimmed = QByteArray(buffArray.data(), readedCount);
        trimmed = trimmed.trimmed();

        if (eolnFound && checkFirstSymbol(trimmed, readUntil)) { // read quality sequence name line, reverting back
            io->skip(-readedCount);
            return;
        }

        sequence.append(trimmed);
    }
}

/**
 * FASTQ format specification: http://maq.sourceforge.net/fastq.shtml
 */
static void load(IOAdapter* io, const U2DbiRef& dbiRef, const QVariantMap& hints, const GUrl& docUrl, QList<GObject*>& objects, U2OpStatus& os,
                 int gapSize, int predictedSize, QString& writeLockReason) {
    writeLockReason.clear();

    QByteArray readBuff(BUFF_SIZE+1, 0), secondBuff;
    char* buff = readBuff.data();

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
        QString sequenceName = readSequenceName(os, io, '@');
        // check for eof while trying to read another FASTQ block
        if (io->isEof()) {
            break;
        }

        CHECK_OP(os,);

        if ((merge == false) || (seqNumber == 0)) {
            QString objName = (merge) ? "Sequence" : TextUtils::variate(sequenceName, "_", names);
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
        readSequence(os, io, sequence);
        CHECK_OP(os,);

        seqImporter.addBlock(sequence.data(),sequence.length(),os);
        CHECK_OP(os,);

        QString qualSequenceName = readSequenceName(os, io, '+');
        if (!qualSequenceName.isEmpty()) {
            static const QString err = U2::FastqFormat::tr("Not a valid FASTQ file: %1, sequence name differs from quality scores name").arg(docUrl.getURLString());
            CHECK_EXT(sequenceName == qualSequenceName, os.setError(err), );
        }

        // read qualities
        qualityScores.clear();
        readSequence(os, io, qualityScores, '@');
        CHECK_OP(os,);

        static const QString err = U2::FastqFormat::tr("Not a valid FASTQ file: %1. Bad quality scores: inconsistent size.").arg(docUrl.getURLString());
        CHECK_EXT(sequence.length() == qualityScores.length(), os.setError(err),);

        seqNumber++;
        if (merge) {
            headers.append(sequenceName);
            mergedMapping.append(U2Region(sequenceStart, sequence.length() ));
        }
        else {
            U2Sequence u2seq = seqImporter.finalizeSequence(os);
            dbiObjects.objects << u2seq.id;
            CHECK_OP(os,);

            U2SequenceObject* seqObj = new U2SequenceObject(u2seq.visualName, U2EntityRef(dbiRef, u2seq.id));
            CHECK_EXT(seqObj != NULL, os.setError("U2SequenceObject is NULL"),);
            seqObj->setQuality(DNAQuality(qualityScores));
            objects << seqObj;

            U1AnnotationUtils::addAnnotations(objects, seqImporter.getCaseAnnotations(), sequenceRef, NULL);
        }
    }

    CHECK_OP(os,);
    bool emptyObjects = objects.isEmpty();
    CHECK_EXT(!emptyObjects || merge, os.setError(Document::tr("Document is empty.")), );
    SAFE_POINT(headers.size() == mergedMapping.size(), "headers <-> regions mapping failed!", );

    if (!merge) {
        return;
    }
    U2Sequence u2seq = seqImporter.finalizeSequence(os);
    dbiObjects.objects << u2seq.id;
    CHECK_OP(os,);

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
    Document* doc = new Document(this, io->getFactory(), io->getURL(), dbiRef, objects, hints, lockReason );

    return doc;
}

void FastqFormat::storeDocument( Document* d, IOAdapter* io, U2OpStatus& os) {
    foreach (GObject* obj, d->getObjects()) {
        U2SequenceObject* seqObj = qobject_cast< U2SequenceObject* >( obj);
        storeEntry(io, seqObj, QList<GObject*>(), os);
        CHECK_OP(os, );
    }
}

void writeSequence(U2OpStatus &os, IOAdapter *io, const char* seq, int len, const QString& errorMessage) {

    CHECK_EXT(io != NULL, os.setError("can't write sequence"),);

    static const int lineLength = 80;

    for (int i = 0; i < len; i += lineLength ) {
        int chunkSize = qMin(lineLength, len - i);

        QByteArray toWrite(seq + i, chunkSize);
        int writtenCount = io->writeBlock(toWrite);
        int writtenEoln = io->writeBlock("\n", 1);
        CHECK_EXT((writtenCount == chunkSize) && (writtenEoln > 0), os.setError(errorMessage),);
    }
}

void FastqFormat::storeEntry(IOAdapter *io, U2SequenceObject *seqObj, const QList<GObject*> &anns, U2OpStatus &os) {
    Q_UNUSED(anns);

    CHECK_EXT(seqObj!=NULL, os.setError(L10N::badArgument("NULL sequence" )), );

    GUrl url = seqObj->getDocument() ? seqObj->getDocument()->getURL() : GUrl();
    static QString errorMessage = L10N::errorWritingFile(url);

    //write header;
    QByteArray block;

    QString sequenceName = seqObj->getGObjectName();
    block.append('@').append(sequenceName).append('\n');

    int writtenCount = io->writeBlock(block);
    CHECK_EXT(writtenCount == block.length(), os.setError(errorMessage),);

    // write sequence
    DNASequence wholeSeq = seqObj->getWholeSequence();
    writeSequence(os, io, wholeSeq.constData(), wholeSeq.length(), errorMessage);

    //write transition
    block.clear();
    block.append("+\n");

    writtenCount = io->writeBlock(block);
    CHECK_EXT(writtenCount == block.length(), os.setError(errorMessage),);

    //write quality
    QByteArray buf;
    const char* qualityData = NULL;
    if (wholeSeq.hasQualityScores()) {
        const QByteArray& quality = wholeSeq.quality.qualCodes;
        CHECK_EXT(wholeSeq.length() == quality.length(), os.setError(errorMessage),);
        qualityData = quality.constData();
    }
    else {
        // record the highest possible quality
        buf.fill('I', wholeSeq.length());
        qualityData = buf.constData();
    }

    writeSequence(os, io, qualityData, wholeSeq.length(), errorMessage);
}

DNASequence *FastqFormat::loadSequence(IOAdapter* io, U2OpStatus& os) {
    CHECK_EXT((io != NULL) && (io->isOpen() == true), os.setError(L10N::badArgument("IO adapter")), NULL);

    QByteArray readBuff;
    QByteArray sequence;
    QByteArray qualityScores;
    int predictedSize = 1000;
    sequence.reserve(predictedSize);
    qualityScores.reserve(predictedSize);

    //read header
    readBuff.clear();
    QString sequenceName = readSequenceName(os, io, '@');
    // check for eof while trying to read another FASTQ block
    CHECK_EXT(!io->isEof(), "", NULL);
    CHECK_OP(os, NULL);

    sequence.clear();  
    readSequence(os, io, sequence);
    CHECK_OP(os, NULL);

    QString qualSequenceName = readSequenceName(os, io, '+');
    if (!qualSequenceName.isEmpty()) {
        static const QString err = U2::FastqFormat::tr("Not a valid FASTQ file, sequence name differs from quality scores name");
        CHECK_EXT(sequenceName == qualSequenceName, os.setError(err), NULL);
    }

    // read qualities
    qualityScores.clear();
    readSequence(os, io, qualityScores, '@');
    CHECK_OP(os, NULL);

    static const QString err = U2::FastqFormat::tr("Not a valid FASTQ file. Bad quality scores: inconsistent size.");
    CHECK_EXT(sequence.length() == qualityScores.length(), os.setError(err), NULL);

    DNASequence *seq = new DNASequence(sequenceName, sequence);
    seq->quality = DNAQuality(qualityScores);
    seq->alphabet = U2AlphabetUtils::getById(BaseDNAAlphabetIds::NUCL_DNA_EXTENDED());
    SAFE_POINT(seq->alphabet != NULL, "FastqFormat::loadSequence alphabet is NULL", NULL);

    if (!seq->alphabet->isCaseSensitive()) {
        TextUtils::translate(TextUtils::UPPER_CASE_MAP, const_cast<char*>(seq->seq.constData()), seq->seq.length());
    }

    return seq;
}

} //namespace
