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
#include <U2Core/U2AssemblyUtils.h>

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

RawDataCheckResult SAMFormat::checkRawData( const QByteArray& rawData, const GUrl&) const {
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

static void prepareRead(const QByteArray& core, const QByteArray& quality, QByteArray& seq, QByteArray& qual, QByteArray& cigar) {
    bool gap = true;
    int lastCigarPos = 0;

    QList<U2CigarToken> cigarTokens;

    for(int i = 0, coreLen = core.length(); i < coreLen; i++) {
        char c = core[i];
        if(c == MAlignment_GapChar) {
            if(!gap) {
                if(i != lastCigarPos) {
                    cigarTokens.append(U2CigarToken(U2CigarOp_M, i - lastCigarPos));
                }
                gap = true;
                lastCigarPos = i;
            }
        } else {
            seq.push_back(c);
            if(!quality.isEmpty()) {
                qual.push_back(DNAQuality::encode(quality[i], DNAQualityType_Sanger));
            }
            if(gap) {
                if(i != lastCigarPos) {
                    cigarTokens.append(U2CigarToken(U2CigarOp_N, i - lastCigarPos));
                }
                gap = false;
                lastCigarPos = i;
            }
        }
    }
    if(lastCigarPos != core.length() - 1) {
        cigarTokens.append(U2CigarToken(gap ? U2CigarOp_N : U2CigarOp_M, core.length() - 1 - lastCigarPos));
    }
    if(cigarTokens.isEmpty()) {
        cigar = QByteArray("*");
    } else {
        foreach(U2CigarToken token, cigarTokens) {
            assert(token.count != 0);
            cigar.append(QString::number(token.count));
            switch(token.op) {
                case U2CigarOp_M: cigar.append("M"); break;
                case U2CigarOp_N: cigar.append("N"); break;
                default:;
            }
        }
    }
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

    if (ti.hasError() || ti.cancelFlag) {
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
            //const QByteArray &core = row.getCore();
            QByteArray qname = QString(row.getName()).replace(QRegExp("\\s|\\t"), "_").toAscii();
            QByteArray flag("0"); // can contains strand, mapped/unmapped, etc.
            QByteArray pos = QByteArray::number(row.getCoreStart()+1);
            QByteArray mapq("255"); //255 indicating the mapping quality is not available
            QByteArray mrnm("*");
            QByteArray mpos("0");
            QByteArray isize("0");
            QByteArray seq;
            QByteArray qual;
            QByteArray cigar;

            prepareRead(row.getCore(), row.getCoreQuality().qualCodes, seq, qual, cigar);
            if (row.hasQuality()) {
                row.getCoreQuality().qualCodes;
            } else {
                qual.reserve(seq.length());
                qual.fill('I', seq.length());
            }
            
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

bool SAMFormat::storeHeader(IOAdapter* io, const QVector<QByteArray> &names, const QVector<int> &lengths) {
    assert(names.size() > 0);
    assert(names.size() == lengths.size());
    static const QByteArray TAB = "\t";
    QByteArray block;
    block.append(SECTION_HEADER).append(TAB).append("VN:").append(VERSION).append("\n");   
    for (int i=0; i<names.size(); i++) {
        block.append(SECTION_SEQUENCE).append(TAB).append(TAG_SEQUENCE_NAME).append(":");
        block.append(names[i]).append(TAB);
        block.append(TAG_SEQUENCE_LENGTH).append(":").append(QByteArray::number(lengths[i])).append("\n");
    }
    if (io->writeBlock( block ) != block.length()) {
        return false;
    }
    block.clear();

    return true;
}

bool SAMFormat::storeAlignedRead(int offset, const DNASequence& read, IOAdapter* io, const QByteArray& refName, int refLength, bool first, bool useCigar, const QByteArray &cigar)
{
    static const QByteArray TAB = "\t";
    static const QByteArray flag("0"); // can contains strand, mapped/unmapped, etc.
    static const QByteArray mapq("255"); //255 indicating the mapping quality is not available
    static const QByteArray mrnm("*");
    static const QByteArray mpos("0");
    static const QByteArray isize("0");
    static const QString rowDataNotCigar = "%1" + TAB + flag + TAB + "%2" + TAB + "%3" + TAB + mapq + TAB + "%4M" + TAB + mrnm
        + TAB + mpos + TAB + isize + TAB + "%5" + TAB + "%6" + "\n";
    static const QString rowDataCigar = "%1" + TAB + flag + TAB + "%2" + TAB + "%3" + TAB + mapq + TAB + "%4" + TAB + mrnm
        + TAB + mpos + TAB + isize + TAB + "%5" + TAB + "%6" + "\n";
    
    if( NULL == io || !io->isOpen() ) {
        return false;
    }
    
    if (first) {
        QByteArray block;
        block.append(SECTION_HEADER).append(TAB).append("VN:").append(VERSION).append("\n");   
        block.append(SECTION_SEQUENCE).append(TAB).append(TAG_SEQUENCE_NAME).append(":");
        block.append(refName).append(TAB);
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

    QByteArray qual;
    if (read.hasQualityScores()) {
        qual = read.quality.qualCodes;
    } else {
        qual.fill('I', read.seq.length());
    }
    QString row;
    if (useCigar) {
        row = rowDataCigar.arg(qname.constData())
            .arg(refName.constData())
            .arg(offset+1)
            .arg(cigar.constData())
            .arg(read.seq.constData())
            .arg(qual.constData());
    } else {
        row = rowDataNotCigar.arg(qname.constData())
            .arg(refName.constData())
            .arg(offset+1)
            .arg(read.seq.length())
            .arg(read.seq.constData())
            .arg(qual.constData());
    }

    if (io->writeBlock(row.toAscii()) != row.length()) {
        return false;
    }

    return true;
}

}// namespace
