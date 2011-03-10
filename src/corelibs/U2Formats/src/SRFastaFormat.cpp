#include "SRFastaFormat.h"

#include "DocumentFormatUtils.h"

#include <U2Core/Task.h>
#include <U2Core/IOAdapter.h>
#include <U2Core/DNAAlphabet.h>
#include <U2Core/L10n.h>

#include <U2Core/MAlignmentObject.h>
#include <U2Core/GObjectTypes.h>
#include <U2Core/TextUtils.h>

namespace U2 {

const QByteArray SRFastaFormat::formatTag(";SHORT READS FASTA");
const QByteArray SRFastaFormat::offsetTag("offset");

SRFastaFormat::SRFastaFormat(QObject* p) : DocumentFormat(p, DocumentFormatFlags_SW, QStringList()<<"srfa"<<"srfasta") {
    formatName = tr("ShortReadsFasta");
    supportedObjectTypes+=GObjectTypes::MULTIPLE_ALIGNMENT;
}

FormatDetectionResult SRFastaFormat::checkRawData(const QByteArray& rawData, const GUrl&) const {
    const char* data = rawData.constData();
    int size = rawData.size();
    
    if (!rawData.startsWith(formatTag)) {
        return FormatDetection_NotMatched;
    }

    bool containsBinary = TextUtils::contains(TextUtils::BINARY, data, size);
    return containsBinary ? FormatDetection_NotMatched : FormatDetection_Matched;
}

#define READ_BUFF_SIZE  4096
static void load(IOAdapter* io, QList<GObject*>& objects, TaskStateInfo& ti) {
    QByteArray readBuff(READ_BUFF_SIZE+1, 0);
    char* buff = readBuff.data();
    qint64 len = 0;
    char fastaHeaderStartChar = '>';
    QBitArray fastaHeaderStart = TextUtils::createBitMap(fastaHeaderStartChar);

    QByteArray sequence;
    MAlignment ma( io->getURL().baseFileName());
    bool lineOk = true;

    // skip format tag 
    io->readLine(buff, READ_BUFF_SIZE);
    
    while (!ti.cancelFlag) {
        //read header
        len = io->readUntil(buff, READ_BUFF_SIZE, TextUtils::LINE_BREAKS, IOAdapter::Term_Include, &lineOk);
        if (len == 0) { //end stream
            break;
        }
        if (!lineOk) {
            ti.setError(SRFastaFormat::tr("Line is too long"));
        }
        QString headerLine = QString(QByteArray::fromRawData(buff+1, len-1)).trimmed();
        if (buff[0]!= fastaHeaderStartChar) {
            ti.setError(SRFastaFormat::tr("First line is not a FASTA header"));
        }

        //read sequence
        sequence.clear();
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

        // Add sequence to alignment
        QStringList headerItems = headerLine.split('|');
        QString name;
        int offset = 0;
        if (headerItems.count() > 2) {
            int k = headerItems.indexOf(SRFastaFormat::offsetTag);
            bool ok = ( k == -1 ? false : true ) ;
            if ( (headerItems.count() > k + 1 ) && (ok == true) ) {
                offset = headerItems.at(k + 1).toInt(&ok);
                name = headerItems.last();
            } 
            if (!ok) {
                ti.setError(SRFastaFormat::tr("Incorrect sequence header: unable to parse sequence offset"));
            }
           
        } else {
            ti.setError(SRFastaFormat::tr("Incorrect sequence header: offset is not present"));
            name = headerLine;
        }
       
        MAlignmentRow row(name, sequence, offset);
        ma.addRow(row);
    }
    
    if (ma.getNumRows() == 0 || ma.getLength() == 0) {
        ti.setError("Multiple alignment is empty!");
    }
    
    DocumentFormatUtils::assignAlphabet(ma);
    if (ma.getAlphabet() == NULL) {
        ti.setError( SRFastaFormat::tr("Alphabet is unknown"));
        return;
    }

     if (!ti.hasErrors() && !ti.cancelFlag) {
         MAlignmentObject* obj = new MAlignmentObject(ma);
         objects.append(obj);
     }

}


Document* SRFastaFormat::loadDocument( IOAdapter* io, TaskStateInfo& ti, const QVariantMap& _fs, DocumentLoadMode mode) {
    Q_UNUSED(mode);
    if( NULL == io || !io->isOpen() ) {
        ti.setError(L10N::badArgument("IO adapter"));
        return NULL;
    }
    QVariantMap fs = _fs;
    QList<GObject*> objects;

    QString lockReason;
    load(io, objects, ti);
    
    if (objects.isEmpty() ) {
        return NULL;
    }

    Document* doc = new Document(this, io->getFactory(), io->getURL(), objects, fs, lockReason);
    return doc;
}

#define LINE_LEN 70
static void saveMAlginmentObj( IOAdapter* io, GObject* obj, TaskStateInfo& tsi ) {
    MAlignmentObject* maObj = qobject_cast< MAlignmentObject* > (obj);

    if ( NULL == maObj ) {
        tsi.setError(L10N::badArgument("NULL multiple alignment object" ));
        return;
    }
    
    const MAlignment& ma = maObj->getMAlignment();

    try {
    
        //writing format tag
        QByteArray tag(SRFastaFormat::formatTag);
        tag.append('\n');
        if (io->writeBlock(tag) != tag.length()) {
            throw 0;
        }

        foreach (const MAlignmentRow& row, ma.getRows()) {

            //writing header;
            QByteArray block;
            QString name = row.getName();
            QString offset = QString("%1").arg(row.getCoreStart());

            block.append('>').append(SRFastaFormat::offsetTag).append('|').append(offset).append('|').append(name).append('\n');
            if (io->writeBlock( block ) != block.length()) {
                throw 0;
            }
            
            //writing sequence
            const char* seq = row.getCore().constData();
            int len = row.getCoreLength();
            for (int i = 0; i < len; i += LINE_LEN ) {
                int chunkSize = qMin( LINE_LEN, len - i );
                if (io->writeBlock( seq + i, chunkSize ) != chunkSize
                    || !io->writeBlock( "\n", 1 )) {
                        throw 0;
                }
            }
        }

    } catch (int) {
        GUrl url = maObj->getDocument() ? maObj->getDocument()->getURL() : GUrl();
        tsi.setError(L10N::errorWritingFile(url));
    }
}

void SRFastaFormat::storeDocument( Document* doc, TaskStateInfo& ts, IOAdapter* io ) {
    QList<GObject*> objs = doc->getObjects();
    foreach( GObject* o, objs ) {
        saveMAlginmentObj( io, o, ts );
    }
}

}//namespace
