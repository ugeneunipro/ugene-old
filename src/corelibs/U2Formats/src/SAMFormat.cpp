#include "SAMFormat.h"
#include "DocumentFormatUtils.h"

#include <U2Core/Task.h>
#include <U2Core/IOAdapter.h>
#include <U2Core/DNAAlphabet.h>
#include <U2Core/L10n.h>
#include <U2Core/DNAQuality.h>

#include <U2Core/DNASequenceObject.h>
#include <U2Core/AnnotationTableObject.h>
#include <U2Core/GObjectTypes.h>
#include <U2Core/MAlignmentObject.h>
#include <U2Core/TextUtils.h>
#include <U2Core/DNATranslation.h>
#include <U2Core/AppContext.h>

#include <QtCore/QRegExp>

namespace U2 {

const QByteArray SAMFormat::VERSION = "1.0";
const QByteArray SAMFormat::SAM_SECTION_START = "@";
const QByteArray SAMFormat::SECTION_HEADER = "@HD"; 
const QByteArray SAMFormat::SECTION_SEQUENCE = "@SQ"; 
const QByteArray SAMFormat::SECTION_READ_GROUP = "@RG";
const QByteArray SAMFormat::SECTION_PROGRAM = "@PG";
const QByteArray SAMFormat::SECTION_COMMENT = "@CO";

const QByteArray SAMFormat::TAG_VERSION = "VN";
const QByteArray SAMFormat::TAG_SORT_ORDER = "SO";
const QByteArray SAMFormat::TAG_GROUP_ORDER = "GO";
 
const QByteArray SAMFormat::TAG_SEQUENCE_NAME = "SN";
const QByteArray SAMFormat::TAG_SEQUENCE_LENGTH = "LN";
const QByteArray SAMFormat::TAG_GENOME_ASSEMBLY_ID = "AS";
const QByteArray SAMFormat::TAG_SEQUENCE_MD5_SUM = "M5";
const QByteArray SAMFormat::TAG_SEQUENCE_URI = "UR";
const QByteArray SAMFormat::TAG_SEQUENCE_SPECIES = "SP";

const SAMFormat::Field SAMFormat::samFields[] = { //alignment section fields excluding optional tags
    Field("QNAME", "[ !-?A-~]+"),
    Field("FLAG", "[0-9]+"),
    Field("RNAME", "\\*|[!-()+-<>-~][ !-~]*"),
    Field("POS", "[0-9]+"),
    Field("MAPQ", "[0-9]+"),
    Field("CIGAR", "([0-9]+[MIDNSHP])+|\\*"),
    Field("RNEXT", "\\*|=|[!-()+-<>-~][!-~]*"),
    Field("PNEXT", "[0-9]+"),
    Field("TLEN", "-?[0-9]+"),
    Field("SEQ", "\\*|[A-Za-z=.]+"),
    Field("QUAL", "[!-~]+|\\*")
};

bool SAMFormat::validateField(int num, QByteArray &field, TaskStateInfo *ti) {
    if(!samFields[num].getPattern().exactMatch(field)) {
        if(ti != NULL) {
            ti->setError(SAMFormat::tr("Field \"%1\" not matched pattern \"%2\", expected pattern \"%3\"").arg(samFields[num].name).arg(QString(field)).arg(samFields[num].getPattern().pattern()));
        }
        return false;
    }
    return true;
}

SAMFormat::SAMFormat( QObject* p ): DocumentFormat(p, DocumentFormatFlags_SW, QStringList()<< "sam")
{
    formatName = tr("SAM");
    supportedObjectTypes+=GObjectTypes::MULTIPLE_ALIGNMENT;
}

FormatDetectionResult SAMFormat::checkRawData( const QByteArray& rawData, const GUrl&) const {
    QRegExp rx("^@[A-Za-z][A-Za-z](\\t[A-Za-z][A-Za-z]:[ -~]+)");
    //try to find SAM header
    if(rx.indexIn(rawData) != 0) {
        // if no header try to parse first alignment line
        QList<QByteArray> fieldValues = rawData.split(SPACE);
        int readFieldsCount = fieldValues.count();
        for(int i=0; i < qMin(11, readFieldsCount); i++) {
            if(!validateField(i, fieldValues[i])) {
                return FormatDetection_NotMatched;
            }
        }
        return FormatDetection_HighSimilarity;
    }
    return FormatDetection_VeryHighSimilarity;
}

Document* SAMFormat::loadDocument( IOAdapter* io, TaskStateInfo& ti, const QVariantMap& _fs, DocumentLoadMode mode /*= DocumentLoadMode_Whole*/ )
{
    Q_UNUSED(mode);
    if( NULL == io || !io->isOpen() ) {
        ti.setError(L10N::badArgument("IO adapter"));
        return NULL;
    }
    QList<GObject*> objects;
    QVariantMap fs = _fs;

    QString lockReason;

    QMap<QString, MAlignment> maMap; //file may contain multiple MA objects
    MAlignment defaultMA("Alignment " + io->getURL().baseFileName());

    QByteArray readBuffer(READ_BUFF_SIZE, '\0');
    char* buff  = readBuffer.data();
    bool lineOk = false;

    QByteArray fields[11];

    int len = 0;
    while(!ti.cancelFlag && (len = io->readLine(buff, READ_BUFF_SIZE, &lineOk)) > 0)
    {
        QByteArray line = QByteArray::fromRawData( buff, len );

        if(line.startsWith(SAM_SECTION_START)) { //Parse sections

            QList<QByteArray> tags;

            if(getSectionTags(line, SECTION_SEQUENCE, tags)) { //Parse sequence section
                foreach(QByteArray tag, tags) {
                    if(tag.startsWith(TAG_SEQUENCE_NAME)) { // Set alignment name
                        QString maName = QByteArray::fromRawData(tag.constData() + 3, tag.length() - 3);
                        MAlignment ma;
                        ma.setName(maName);
                        maMap[maName] = ma;
                    }
                }
            } else if(getSectionTags(line, SECTION_HEADER, tags)) { //Parse header section
                foreach(QByteArray tag, tags) {
                    if(tag.startsWith(TAG_VERSION)) { //Check file format version
                        QByteArray versionStr = QByteArray::fromRawData(tag.constData() + 3, tag.length() - 3);
                        QList<QByteArray> version = versionStr.split('.');
                        if(version[0].toInt() != 1 && version[1].toInt() > 3) {
                            ti.setError(SAMFormat::tr("Unsupported file version \"%1\"").arg(QString(versionStr)));
                            return NULL;
                        }
                    }
                }
            }
            // Skip other sections

            continue;
        }

        QList<QByteArray> fieldValues = line.split(SPACE);

        int readFieldsCount = fieldValues.count();

        //if(readFieldsCount < 11) readFieldsCount--;
        QBitArray terminators = TextUtils::WHITES | TextUtils::LINE_BREAKS;
        char lastTerminator = lineOk ? '\n' : 0;

        while(readFieldsCount < 11 && (len = io->readUntil(buff, READ_BUFF_SIZE, terminators, IOAdapter::Term_Include, &lineOk)) > 0) {
            QByteArray addline = QByteArray::fromRawData( buff, len - 1 ).simplified();
            fieldValues[readFieldsCount - 1].append(addline);
            lastTerminator = buff[len-1];
            if(lineOk)
                break;
            else {
                fieldValues[readFieldsCount - 1].append(lastTerminator);
            }
        }
        {
            bool merge = readFieldsCount < 11 ? false : true;
            /*if(readFieldsCount < 11)*/ {
                while(!TextUtils::LINE_BREAKS.at(lastTerminator) && (len = io->readUntil(buff, READ_BUFF_SIZE, terminators, IOAdapter::Term_Include, &lineOk)) > 0) {
                    if(!lineOk) {
                        len++;
                    }
                    QByteArray addline = QByteArray::fromRawData( buff, len - 1).simplified();
                    if(merge) {
                        fieldValues[readFieldsCount - 1].append(addline);
                    } else {
                        fieldValues.append(addline);
                        readFieldsCount++;
                    }
                    lastTerminator = buff[len - 1];
                    merge = !lineOk;
                }
            }

            // skiping optional tags
            if(!TextUtils::LINE_BREAKS.at(lastTerminator))
                while((len = io->readLine(buff, READ_BUFF_SIZE, &lineOk)) > 0 && !lineOk);
        }

        if(readFieldsCount < 11) {
            ti.setError(SAMFormat::tr("Unexpected end of file"));
            return NULL;
        }

        for(int i=0; i < qMin(11, readFieldsCount); i++) {
            fields[i] = fieldValues[i];
             if(!validateField(i, fields[i], &ti)) {
                 return NULL;
            }
        }

        QString rname = fields[2];

        if(rname != "*" && !maMap.contains(rname)) {
            //ioLog.info(SAMFormat::tr("Reference sequence \"%1\" not present in @SQ header").arg(rname));
            rname = "*";
        }

        MAlignmentRow row;

        short flag = fields[1].toShort();
        bool isReversed = flag & 0x0010;

        row.setName(fields[0]);
        if(fields[9] == "*") {
            row.setSequence("", 0);
        } else {
            if(isReversed) {
                QByteArray &seq = fields[9];
                DNAAlphabet *al = AppContext::getDNAAlphabetRegistry()->findAlphabet(seq);
                if(al == NULL) {
                    ti.setError(SAMFormat::tr("Can't find alphabet for sequence \"%1\"").arg(QString(seq)));
                    return NULL;
                }
                DNATranslation* tr = AppContext::getDNATranslationRegistry()->lookupComplementTranslation(al);
                if(tr == NULL) {
                    ti.setError(SAMFormat::tr("Can't translation for alphabet \"%1\"").arg(al->getName()));
                    return NULL;
                }
                TextUtils::translate(tr->getOne2OneMapper(), seq.data(), seq.size());
                TextUtils::reverse(seq.data(), seq.size());
                row.setSequence(seq, fields[3].toInt()-1);
            } else {
                row.setSequence(fields[9], fields[3].toInt()-1);
            }   
        }

        if(fields[10] != "*") {
            if(isReversed) {
                QByteArray &seq = fields[10];
                TextUtils::reverse(seq.data(), seq.size());
                row.setQuality(DNAQuality(seq));
            }
            else
                row.setQuality(DNAQuality(fields[10]));
        }

        if(rname == "*") {
            defaultMA.addRow(row);
        } else {
            maMap[rname].addRow(row);
        }

        ti.progress = io->getProgress();
    }

    foreach(MAlignment ma, maMap.values()) {
        DocumentFormatUtils::assignAlphabet(ma);
        if (ma.getAlphabet() == NULL) {
            ti.setError( SAMFormat::tr("Alphabet is unknown"));
            return NULL;
        }

        objects.append(new MAlignmentObject(ma));
    }

    if(defaultMA.getRows().count() != 0) {
        DocumentFormatUtils::assignAlphabet(defaultMA);
        if (defaultMA.getAlphabet() == NULL) {
            ti.setError( SAMFormat::tr("Alphabet is unknown"));
            return NULL;
        }

        objects.append(new MAlignmentObject(defaultMA));
    }

    if (ti.hasErrors() || ti.cancelFlag) {
        qDeleteAll(objects);
        return NULL;
    }

    DocumentFormatUtils::updateFormatSettings(objects, fs);
    Document* doc = new Document(this, io->getFactory(), io->getURL(), objects, fs, lockReason);
    return doc;
}


void SAMFormat::storeDocument( Document* d, TaskStateInfo& ts, IOAdapter* io )
{
    //TODO: sorting options?
    if( NULL == d ) {
        ts.setError(L10N::badArgument("doc"));
        return;
    }
    if( NULL == io || !io->isOpen() ) {
        ts.setError(L10N::badArgument("IO adapter"));
        return;
    }

    QList<const MAlignmentObject*> maList;
    foreach(GObject *obj, d->findGObjectByType(GObjectTypes::MULTIPLE_ALIGNMENT)) {
        const MAlignmentObject* maObj = qobject_cast<const MAlignmentObject*>(obj);
        assert(maObj != NULL);
        maList.append(maObj);
    }

    QByteArray tab = "\t";
    QByteArray block;

    //Writing header
    block.append(SECTION_HEADER).append("\t").append("VN:").append(VERSION).append("\n");
    if (io->writeBlock( block ) != block.length()) {
        throw 0;
    }

    //Writing sequence section
    foreach(const MAlignmentObject* maObj, maList) {
        const MAlignment &ma = maObj->getMAlignment();
        block.clear();
        block.append(SECTION_SEQUENCE).append(tab).append(TAG_SEQUENCE_NAME).append(":").append(ma.getName().replace(QRegExp("\\s|\\t"), "_"))
            .append(tab).append(TAG_SEQUENCE_LENGTH).append(":").append(QByteArray::number(ma.getLength())).append("\n");
        if (io->writeBlock( block ) != block.length()) {
            throw 0;
        }
    }

    //Writing alignment section
    foreach(const MAlignmentObject* maObj, maList) {
        const MAlignment &ma = maObj->getMAlignment();
        QByteArray rname(ma.getName().replace(QRegExp("\\s|\\t"), "_").toAscii());
        foreach(MAlignmentRow row, ma.getRows()) {
            block.clear();
            QByteArray qname = QString(row.getName()).replace(QRegExp("\\s|\\t"), "_").toAscii();
            QByteArray flag("0"); // can contains strand, mapped/unmapped, etc.
            QByteArray pos = QByteArray::number(row.getCoreStart()+1);
            QByteArray mapq("255"); //255 indicating the mapping quality is not available
            QByteArray cigar("*");
            QByteArray mrnm("*");
            QByteArray mpos("0");
            QByteArray isize("0");
            QByteArray seq(row.getCore());
            QByteArray qual(row.getCoreQuality().qualCodes);
            if(qual.isEmpty()) qual.fill('I', row.getCoreLength()); //I - 50 Phred quality score (99.999%)

            block = qname + tab + flag + tab+ rname + tab + pos + tab + mapq + tab + cigar + tab + mrnm
                + tab + mpos + tab + isize + tab + seq + tab + qual + "\n";
            if (io->writeBlock( block ) != block.length()) {
                throw 0;
            }
        }
    }
}

bool SAMFormat::getSectionTags( QByteArray &line, const QByteArray &sectionName, QList<QByteArray> &tags )
{
    if(!line.startsWith(sectionName)) return false;
    QByteArray tagsLine = QByteArray::fromRawData(line.constData() + 3, line.length() - 3);
    tags = tagsLine.split(SPACE);
    return true;
}

bool SAMFormat::storeAlignedRead( int offset, const DNASequence& read, IOAdapter* io, const QString& refName, int refLength, bool first )
{
   
    static const QByteArray TAB = "\t";
    
    if( NULL == io || !io->isOpen() ) {
        //ts.setError(L10N::badArgument("IO adapter"));
        return false;
    }

    QByteArray block;
    QByteArray rname = QString(refName).replace(QRegExp("\\s|\\t"), "_").toAscii();

    
    if (first) {
        block.append(SECTION_HEADER).append(TAB).append("VN:").append(VERSION).append("\n");   
        block.append(SECTION_SEQUENCE).append(TAB).append(TAG_SEQUENCE_NAME).append(":");
        block.append(rname).append(TAB);
        block.append(TAG_SEQUENCE_LENGTH).append(":").append(QByteArray::number(refLength)).append("\n");
        if (io->writeBlock( block ) != block.length()) {
            return false;
        }
        block.clear();
    }
    
    QByteArray qname = QString(read.getName()).replace(QRegExp("\\s|\\t"), "_").toAscii();
    if (qname.isEmpty()) {
        qname = "contig";
    }
    QByteArray flag("0"); // can contains strand, mapped/unmapped, etc.
    QByteArray pos = QByteArray::number(offset+1);
    QByteArray mapq("255"); //255 indicating the mapping quality is not available
    QByteArray cigar("*");
    QByteArray mrnm("*");
    QByteArray mpos("0");
    QByteArray isize("0");
    QByteArray seq(read.seq);
    QByteArray qual(read.quality.qualCodes);
    if(qual.isEmpty()) {
        qual.fill('I', read.length()); //I - 50 Phred quality score (99.999%)
    }

    block = qname + TAB + flag + TAB + rname + TAB + pos + TAB + mapq + TAB + cigar + TAB + mrnm
        + TAB + mpos + TAB + isize + TAB + seq + TAB + qual + "\n";
    if (io->writeBlock( block ) != block.length()) {
        return false;
    }

    return true;


    
}

}// namespace
