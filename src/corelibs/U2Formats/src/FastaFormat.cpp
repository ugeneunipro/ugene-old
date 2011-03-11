#include "FastaFormat.h"

#include "DocumentFormatUtils.h"

#include <U2Core/Task.h>
#include <U2Core/IOAdapter.h>
#include <U2Core/DNAAlphabet.h>
#include <U2Core/L10n.h>

#include <U2Core/DNASequenceObject.h>
#include <U2Core/AnnotationTableObject.h>
#include <U2Core/GObjectTypes.h>
#include <U2Core/TextUtils.h>

namespace U2 {

/* TRANSLATOR U2::FastaFormat */
/* TRANSLATOR U2::IOAdapter */
/* TRANSLATOR U2::Document */

FastaFormat::FastaFormat(QObject* p) 
: DocumentFormat(p, DocumentFormatFlags_SW, QStringList()<<"fa"<<"mpfa"<<"fna"<<"fsa"<<"fas"<<"fasta"<<"sef"<<"seq"<<"seqs")
{
    formatName = tr("FASTA");
    supportedObjectTypes+=GObjectTypes::SEQUENCE;
}

FormatDetectionResult FastaFormat::checkRawData(const QByteArray& rawData, const GUrl&) const {
    const char* data = rawData.constData();
    int size = rawData.size();

    int n = TextUtils::skip(TextUtils::WHITES, data, size);
    int newSize = size - n;
    const char* newData = data + n;
    if (newSize <= 0 || newData[0] != '>' ) {
        return FormatDetection_NotMatched;
    }
    bool hasBinaryBlocks = TextUtils::contains(TextUtils::BINARY, data, size);
    return hasBinaryBlocks ? FormatDetection_NotMatched : FormatDetection_HighSimilarity;
}

#define READ_BUFF_SIZE  4096
static void load(IOAdapter* io, const GUrl& docUrl, QList<GObject*>& objects, TaskStateInfo& ti,
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
            QString objName = TextUtils::variate(headerLine, "_", names);
            names.insert(objName);
            DNASequence seq( headerLine, sequence );
            //TODO parse header
            seq.info.insert(DNAInfo::FASTA_HDR, headerLine);
            seq.info.insert(DNAInfo::ID, headerLine);
            DocumentFormatUtils::addSequenceObject(objects, objName, seq);
        }

        if( mode == DocumentLoadMode_SingleObject ) {
            break;
        }
    }

    assert(headers.size() == mergedMapping.size());

    if (!ti.hasErrors() && !ti.cancelFlag && merge && !headers.isEmpty()) {
        DocumentFormatUtils::addMergedSequenceObject(objects, docUrl, headers, sequence, mergedMapping);
    }
    if (!ti.hasErrors() && !ti.cancelFlag && objects.isEmpty()) {
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

    int gapSize = qBound(-1, DocumentFormatUtils::getIntSettings(fs, MERGE_MULTI_DOC_GAP_SIZE_SETTINGS, -1), 1000*1000);
    int predictedSize = qMax(1000,
        DocumentFormatUtils::getIntSettings(fs, MERGE_MULTI_DOC_SEQUENCE_SIZE_SETTINGS, gapSize==-1 ? 0 : io->left()));

    QString lockReason;
    load(io, io->getURL(), objects, ti, gapSize, predictedSize, lockReason, mode);

    if (ti.hasErrors() || ti.cancelFlag) {
        qDeleteAll(objects);
        return NULL;
    }

    DocumentFormatUtils::updateFormatSettings(objects, fs);
    Document* doc = new Document(this, io->getFactory(), io->getURL(), objects, fs, lockReason);
    return doc;
}

#define LINE_LEN 70
static void saveOneFasta( IOAdapter* io, GObject* fastaObj, TaskStateInfo& tsi ) {
    DNASequenceObject* seqObj = qobject_cast< DNASequenceObject* >( fastaObj );

    if ( NULL == seqObj ) {
        tsi.setError(L10N::badArgument("NULL sequence" ));
        return;
    }

    //writing header;
    QByteArray block;
    // TODO better header out of info tags
    /*QString hdr = seqObj->getDNASequence().info.value(DNAInfo::FASTA_HDR).toString();
    if (hdr.isEmpty()) {
        hdr = seqObj->getGObjectName();
    }*/
    QString hdr = seqObj->getGObjectName();
    block.append('>').append(hdr).append( '\n' );
    try {
        if (io->writeBlock( block ) != block.length()) {
            throw 0;
        }
        const char* seq = seqObj->getSequence().constData();
        int len = seqObj->getSequence().length();
        for (int i = 0; i < len; i += LINE_LEN ) {
            int chunkSize = qMin( LINE_LEN, len - i );
            if (io->writeBlock( seq + i, chunkSize ) != chunkSize
                || !io->writeBlock( "\n", 1 )) {
                    throw 0;
            }
        }
    } catch (int) {
        GUrl url = seqObj->getDocument() ? seqObj->getDocument()->getURL() : GUrl();
        tsi.setError(L10N::errorWritingFile(url));
    }
}

void FastaFormat::storeDocument( Document* doc, TaskStateInfo& ts, IOAdapter* io ) {
    //TODO: check saved op states!!!
    foreach( GObject* o, doc->getObjects() ) {
        saveOneFasta( io, o, ts );
    }
}

}//namespace
