#include "FastqFormat.h"
#include "DocumentFormatUtils.h"

#include <U2Core/Task.h>
#include <U2Core/IOAdapter.h>
#include <U2Core/DNAAlphabet.h>
#include <U2Core/L10n.h>

#include <U2Core/DNASequenceObject.h>
#include <U2Core/AnnotationTableObject.h>
#include <U2Core/GObjectTypes.h>
#include <U2Core/TextUtils.h>

/* TRANSLATOR U2::FastqFormat */

namespace U2 {

FastqFormat::FastqFormat(QObject* p) 
: DocumentFormat(p, DocumentFormatFlags_SW, QStringList("fastq")), fn(tr("FASTQ")) 
{
    supportedObjectTypes+=GObjectTypes::SEQUENCE;
}

FormatDetectionResult FastqFormat::checkRawData(const QByteArray& rawData, const GUrl&) const {
    const char* data = rawData.constData();
    int size = rawData.size();

    if (size <= 0 || data[0] != '@' ) {
        return FormatDetection_NotMatched;
    }
    bool hasBinaryBlocks = TextUtils::contains(TextUtils::BINARY, data, size);
    return hasBinaryBlocks ? FormatDetection_NotMatched: FormatDetection_AverageSimilarity;
}


#define BUFF_SIZE  4096

static bool readLine(QByteArray& target, IOAdapter* io, TaskStateInfo& ti, bool last = false, int * howManyRead = NULL) {
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
        ti.progress = io->getProgress();
        total += len;
    } while (!ti.cancelFlag && !lineOK && len == BUFF_SIZE);
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
        ti.setError(U2::FastqFormat::tr("Unexpected end of file"));
    }
    return lineOK;
}

static const QByteArray SEQ_QUAL_SEPARATOR = "+";

// reads until new line, starting with separator
// separator not included in read data
static bool readUntil(QByteArray & target, IOAdapter * io, TaskStateInfo & ti, const QByteArray & separator) {
    QByteArray currentLine;
    while(1) {
        currentLine.clear();
        int howMany = 0;
        readLine(currentLine, io, ti, false, &howMany);
        if( ti.hasErrors() ) {
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

static bool readBlock( QByteArray & block, IOAdapter * io, TaskStateInfo & ti, qint64 size ) {
    assert(size >= 0);
    while( block.size() < size ) {
        QByteArray curBlock;
        readLine(curBlock, io, ti);
        if(ti.hasErrors()) {
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
static void load(IOAdapter* io, const GUrl& docUrl, QList<GObject*>& objects, TaskStateInfo& ti,
                 int gapSize, int predictedSize, QString& writeLockReason, DocumentLoadMode mode) {
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

     int sequenceStart = 0;
     while (!ti.cancelFlag) {
         //read header
         readBuff.clear();
         if (!readLine(readBuff, io, ti, (merge && !headers.isEmpty()) || !names.isEmpty())) {
             break;
         }
         if (readBuff[0]!= '@') {
             ti.setError(U2::FastqFormat::tr("Not a valid FASTQ file: %1. The @ identifier is not found.").arg(docUrl.getURLString()));
             break;
         }

         //read sequence
         if (!merge) {
             sequence.clear();
         } else if (sequence.size() > 0) {
             sequence.append(gapSequence);
         }
         sequenceStart = sequence.size();
         if(!readUntil(sequence, io, ti, SEQ_QUAL_SEPARATOR)) {
            break;
         }
         int seqLen = sequence.size() - sequenceStart;

         // read +<seqname>
         secondBuff.clear();
         secondBuff.reserve(readBuff.size());
         if (!readLine(secondBuff, io, ti)) {
             break;
         }
         if (secondBuff[0]!= '+' || (secondBuff.size() != 1 && secondBuff.size() != readBuff.size())
             || (readBuff.size() == secondBuff.size() && strncmp(readBuff.data()+1, secondBuff.data()+1, readBuff.size() - 1))) {
             ti.setError(U2::FastqFormat::tr("Not a valid FASTQ file: %1").arg(docUrl.getURLString()));
             break;
         }
         
         // read qualities
         qualityScores.clear();
         
         if( !readBlock(qualityScores, io, ti, seqLen) ) {
            break;
         }
         
         if ( qualityScores.length() != sequence.length() ) {
             ti.setError(U2::FastqFormat::tr("Not a valid FASTQ file: %1. Bad quality scores: inconsistent size.").arg(docUrl.getURLString()));   
         }
         
         QString headerLine = QString::fromLatin1(readBuff.data()+1, readBuff.length()-1);
         if (merge) {
             headers.append(headerLine);
             mergedMapping.append(U2Region(sequenceStart, seqLen));
         } else {
             QString objName = TextUtils::variate(headerLine, "_", names);
             names.insert(objName);
             DNASequence seq( headerLine, sequence );
             seq.quality = DNAQuality(qualityScores);
             seq.info.insert(DNAInfo::ID, headerLine);
             DocumentFormatUtils::addSequenceObject(objects, objName, seq);
         }

         if (mode == DocumentLoadMode_SingleObject) {
             break;
         }
     }

     assert(headers.size() == mergedMapping.size());

     if (!ti.hasErrors() && !ti.cancelFlag && merge && !headers.isEmpty()) {
         DocumentFormatUtils::addMergedSequenceObject(objects, docUrl, headers, sequence, mergedMapping);
     }

     if (merge && headers.size() > 1) {
         writeLockReason = DocumentFormat::MERGED_SEQ_LOCK;
     }
}

Document* FastqFormat::loadDocument( IOAdapter* io, TaskStateInfo& ti, const QVariantMap& _fs, DocumentLoadMode mode) {
    if( NULL == io || !io->isOpen() ) {
        ti.setError(L10N::badArgument("IO adapter"));
        return NULL;
    }
    QVariantMap fs = _fs;
    QList<GObject*> objects;

    int gapSize = qBound(-1, DocumentFormatUtils::getIntSettings(fs, MERGE_MULTI_DOC_GAP_SIZE_SETTINGS, -1), 1000*1000);
    int predictedSize = qMax(100*1000,
        DocumentFormatUtils::getIntSettings(fs, MERGE_MULTI_DOC_SEQUENCE_SIZE_SETTINGS, gapSize==-1 ? 0 : io->left()));

    QString lockReason;
    load( io, io->getURL(), objects, ti, gapSize, predictedSize, lockReason, mode);

    if (ti.hasErrors() || ti.cancelFlag) {
        qDeleteAll(objects);
        return NULL;
    }

    DocumentFormatUtils::updateFormatSettings(objects, fs);
    Document* doc = new Document( this, io->getFactory(), io->getURL(), objects, fs, lockReason );
    return doc;
}

#define LINE_LEN 70

void FastqFormat::storeDocument( Document* d, TaskStateInfo& ts, IOAdapter* io )
{
    
    foreach (GObject* obj, d->getObjects()) {
    
    DNASequenceObject* seqObj = qobject_cast< DNASequenceObject* >( obj);

    if ( NULL == seqObj ) {
        ts.setError(L10N::badArgument("NULL sequence" ));
        continue;
    }

    try {

        //write header;
        QByteArray block;

        QString hdr = seqObj->getGObjectName();
        block.append('@').append(hdr).append( '\n' );
        if (io->writeBlock( block ) != block.length()) {
            throw 0;
        }
        // write sequence
        const char* seq = seqObj->getSequence().constData();

        int len = seqObj->getSequence().length();
        for (int i = 0; i < len; i += LINE_LEN ) {
            int chunkSize = qMin( LINE_LEN, len - i );
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
        if (seqObj->getDNASequence().hasQualityScores()) {
            quality = seqObj->getQuality().qualCodes.constData();
            len = seqObj->getQuality().qualCodes.length();
        } else {
            // record the highest possible quality
            buf.fill('I',len);
            quality = buf.constData();
        } 

        for (int i = 0; i < len; i += LINE_LEN ) {
            int chunkSize = qMin( LINE_LEN, len - i );
            if (io->writeBlock( quality + i, chunkSize ) != chunkSize
                || !io->writeBlock( "\n", 1 )) {
                    throw 0;
            }
        }

    } catch (int) {
        GUrl url = seqObj->getDocument() ? seqObj->getDocument()->getURL() : GUrl();
        ts.setError(L10N::errorWritingFile(url));
    }
    }
}
}//namespace
