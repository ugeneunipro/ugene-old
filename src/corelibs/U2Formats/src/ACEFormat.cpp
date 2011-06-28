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

#include "ACEFormat.h"
#include <U2Formats/DocumentFormatUtils.h>
#include <U2Core/Task.h>
#include <U2Core/IOAdapter.h>
#include <U2Core/L10n.h>
#include <U2Core/GObjectTypes.h>
#include <U2Core/MAlignmentObject.h>
#include <U2Core/TextUtils.h>
#include <U2Core/MSAUtils.h>
#include <U2Core/GObjectRelationRoles.h>

namespace U2 {

    const QString ACEFormat::CO = "CO";
    const QString ACEFormat::RD = "RD";
    const QString ACEFormat::QA = "QA";
    const QString ACEFormat::AS = "AS";
    const QString ACEFormat::AF = "AF";
    const QString ACEFormat::BQ = "BQ";

ACEFormat::ACEFormat(QObject* p) : DocumentFormat(p, DocumentFormatFlags(0), QStringList("ace")) {
    formatName = tr("ACE");
    supportedObjectTypes+=GObjectTypes::MULTIPLE_ALIGNMENT;
}

static int modifyLine(QString &line, int pos){
    int curIdx = 0;
    char space = ' ';

    line=line.simplified();

    for (int i=0; i < pos; i++){
        curIdx = line.indexOf(space);
        if(-1== curIdx){
            return 0;
        }

        line = line.mid(curIdx+1);
    }
    curIdx = line.indexOf(space);
    if(-1== curIdx){
        return 0;
    }

    line = line.mid(0, curIdx);

    bool ok=false;
    int result = line.toInt(&ok);
    if(ok == false){
        return -1;
    }else{
        return result;
    }
}

static int prepareLine(QString &line, int pos){
    int curIdx = 0;
    char space = ' ';

    line=line.simplified();

    for (int i=0; i < pos; i++){
        curIdx = line.indexOf(space);
        if(-1== curIdx){
            return -1;
        }

        line = line.mid(curIdx+1);
    }

    return curIdx;
}

#define READS_COUNT_POS 3
static int readsCount(const QString& cur_line){
    QString line = cur_line;
    return modifyLine(line, READS_COUNT_POS);
}

#define CONTIG_COUNT_POS 1
static int contigCount(const QString& cur_line){
    QString line = cur_line;
    return modifyLine(line, CONTIG_COUNT_POS);
}

#define LAST_QA_POS 4
static int clearRange(const QString& cur_line){
    QString line = cur_line;
    modifyLine(line, LAST_QA_POS);

    bool ok = true;
    int result = line.toInt(&ok);
    if(!ok){
        return INT_MAX;
    }else{
        return result;
    }
}
#define PADDED_START_POS 3
static int paddedStartCons(const QString& cur_line){
    QString line = cur_line;
    modifyLine(line, PADDED_START_POS);

    bool ok = true;
    int result = line.toInt(&ok);
    if(!ok){
        return INT_MAX;
    }else{
        return result;
    }
}

#define READS_POS 3
static int readsPos(const QString& cur_line){
    QString line = cur_line;
    prepareLine(line, READS_POS);

    if(-1 != line.indexOf(' ')){
        return INT_MAX;
    }

    line = line.mid(0, line.length());

    bool ok = true;
    int result = line.toInt(&ok);
    if(!ok){
        return INT_MAX;
    }else{
        return result;
    }
}
#define COMPLEMENT_POS 2
static int readsComplement(const QString& cur_line){
    QString line = cur_line;
    prepareLine(line, COMPLEMENT_POS);

    if(line.startsWith("U")){
        return 0;
    }else if (line.startsWith("C")){
        return 1;
    }else{
        return -1;
    }
}

static QString getName(const QString &line){
    int curIdx = 0;
    char space = ' ';

    QString name = line.simplified();

    curIdx = name.indexOf(space);
    if(-1== curIdx){
        return "";
    }

    name = name.mid(curIdx+1);

    curIdx = name.indexOf(space);
    if(-1== curIdx){
        return "";
    }

    name = name.mid(0,curIdx);

    return name;
}

static bool checkSeq(const QByteArray &seq){
    for(int i =0; i <seq.length(); i++){
        if(seq[i]!='A' && seq[i]!='C' && seq[i]!='G' && seq[i]!='T' && seq[i]!='N' && seq[i]!='*' && seq[i]!='X')
            return false;
    }
    return true;
}

#define READ_BUFF_SIZE  4096
void ACEFormat::load(U2::IOAdapter *io, QList<GObject*> &objects, U2::TaskStateInfo &ti) {
    QByteArray readBuff(READ_BUFF_SIZE+1, 0);
    char* buff = readBuff.data();
    qint64 len = 0;
    int count = 0;

    char aceHeaderStartChar = 'C';
    QBitArray aceHeaderStart = TextUtils::createBitMap(aceHeaderStartChar);
    char aceReadStartChar = 'R';
    QBitArray aceReadStart = TextUtils::createBitMap(aceReadStartChar);
    char aceBStartChar = 'B';
    QBitArray aceBStart = TextUtils::createBitMap(aceBStartChar);
    char aceQStartChar = 'Q';
    QBitArray aceQStart = TextUtils::createBitMap(aceQStartChar);

    QByteArray sequence;
    QSet<QString> names;
    QMap< QString, int> posMap;
    QMap< QString, bool> complMap;

     //skip leading whites if present
    bool lineOk = true;
    len = io->readUntil(buff, READ_BUFF_SIZE, TextUtils::LINE_BREAKS, IOAdapter::Term_Include, &lineOk);
    if (len == 0) { //end if stream
        return ;
    }
    if (!lineOk) {
        ti.setError(ACEFormat::tr("Line is too long"));
        return ;
    }
    QString headerLine = QString(QByteArray::fromRawData(buff, len)).trimmed();
    if (!headerLine.startsWith(AS)) {
        ti.setError(ACEFormat::tr("First line is not an ace header"));
        return ;
    }
    int contigC = contigCount(headerLine);
    if(-1==contigC){
         ti.setError(ACEFormat::tr("No contig count tag in the header line"));
        return ;
    }
    for(int i =0; i < contigC; i++){
        if(i==0){
            QBitArray nonWhites = ~TextUtils::WHITES;
            io->readUntil(buff, READ_BUFF_SIZE, nonWhites, IOAdapter::Term_Exclude, &lineOk);
            //read header
            len = io->readUntil(buff, READ_BUFF_SIZE, TextUtils::LINE_BREAKS, IOAdapter::Term_Include, &lineOk);
            if (len == 0) { //end if stream
                return ;
            }
            if (!lineOk) {
                ti.setError(ACEFormat::tr("Line is too long"));
                return ;
            }
            headerLine = QString(QByteArray::fromRawData(buff, len)).trimmed();
            if (!headerLine.startsWith(CO)) {
                ti.setError(ACEFormat::tr("Must be CO keyword"));
                return ;
            }
        }else{
            do{
                len = io->readUntil(buff, READ_BUFF_SIZE, TextUtils::LINE_BREAKS, IOAdapter::Term_Include, &lineOk);
                if (len == 0) { //end if stream
                    ti.setError(ACEFormat::tr("Unexpected end of file"));
                    return ;
                }
                if (!lineOk) {
                    ti.setError(ACEFormat::tr("Line is too long"));
                    return ;
                }
                headerLine = QString(QByteArray::fromRawData(buff, len)).trimmed();
            }while (!headerLine.startsWith(CO));
        }
        count = readsCount(headerLine);
        if(-1 == count){
            ti.setError(ACEFormat::tr("There is no note about reads count"));
            return ;
        }
        QString readLine;
        QString name;
        QByteArray consensus;
        //consensus
        QString consName = getName(headerLine);
        if("" == consName){
                ti.setError(ACEFormat::tr("There is no AF note"));
                return ;
        }
        if (names.contains(consName)) {
                ti.setError(ACEFormat::tr("A name is duplicated"));
                return ;
        }
        names.insert(consName);
        consensus.clear();
            do {
                len = io->readUntil(buff, READ_BUFF_SIZE, aceBStart, IOAdapter::Term_Exclude, &lineOk);
                if (len <= 0) {
                    ti.setError(ACEFormat::tr("No consensus"));
                    return ;
                }
                len = TextUtils::remove(buff, len, TextUtils::WHITES);
                buff[len] = 0;
                consensus.append(buff);
                ti.progress = io->getProgress();
            } while (!ti.cancelFlag && !lineOk);
            len = io->readUntil(buff, READ_BUFF_SIZE, TextUtils::LINE_BREAKS, IOAdapter::Term_Include, &lineOk);
            readLine = QString(QByteArray::fromRawData(buff, len)).trimmed();
            if(!readLine.startsWith(BQ)){
                ti.setError(ACEFormat::tr("BQ keyword hasn't been found"));
                return ;
            }
            consensus=consensus.toUpper();
            if(!checkSeq(consensus)){
                ti.setError(ACEFormat::tr("Bad consensus data"));
                return ;
            }
            consensus.replace('*',MAlignment_GapChar);
            MAlignment al(consName);
            al.addRow(MAlignmentRow(consName, consensus));

        //AF
        int count1 = count;
        int readPos = 0;
        int complStrand = 0;
        int paddedStart = 0;
        while (!ti.cancelFlag && count1>0) {
            do{
                len = io->readUntil(buff, READ_BUFF_SIZE, TextUtils::LINE_BREAKS, IOAdapter::Term_Include, &lineOk);
                if (len == 0) { //end if stream
                    ti.setError(ACEFormat::tr("Unexpected end of file"));
                    return ;
                }
                if (!lineOk) {
                    ti.setError(ACEFormat::tr("Line is too long"));
                    return ;
                }
                readLine = QString(QByteArray::fromRawData(buff, len)).trimmed();
            }while (!readLine.startsWith(AF));

            name = getName(readLine);
            if(!readLine.startsWith(AF) || "" == name){
                ti.setError(ACEFormat::tr("There is no AF note"));
                return ;
            }

            readPos = readsPos(readLine);
            complStrand = readsComplement(readLine);
            if((INT_MAX == readPos) ||  (-1 == complStrand) ){
                ti.setError(ACEFormat::tr("Bad AF note"));
                return ;
            }

            paddedStart = paddedStartCons(readLine);
            if(INT_MAX == paddedStart){
                ti.setError(ACEFormat::tr("Bad AF note"));
                return ;
            }

            posMap.insert(name,paddedStart);

            if (names.contains(name)) {
                ti.setError(ACEFormat::tr("A name is duplicated"));
                return ;
            }

            bool cur_compl = (complStrand == 1);
            complMap.insert(name,cur_compl);

            names.insert(name);

            count1--;
            ti.progress = io->getProgress();
        }
        //RD

        while (!ti.cancelFlag && count>0) {
            do{
                len = io->readUntil(buff, READ_BUFF_SIZE, TextUtils::LINE_BREAKS, IOAdapter::Term_Include, &lineOk);
                if (len == 0) { //end if stream
                    ti.setError(ACEFormat::tr("Unexpected end of file"));
                    return ;
                }
                if (!lineOk) {
                    ti.setError(ACEFormat::tr("Line is too long"));
                    return ;
                }
                readLine = QString(QByteArray::fromRawData(buff, len)).trimmed();
            }while (!readLine.startsWith(RD));

            name = getName(readLine);
            if(!readLine.startsWith(RD) || "" == name){
                ti.setError(ACEFormat::tr("There is no read note"));
                return ;
            }

            sequence.clear();
            do {
                len = io->readUntil(buff, READ_BUFF_SIZE, aceQStart, IOAdapter::Term_Exclude, &lineOk);
                if (len <= 0) {
                    ti.setError(ACEFormat::tr("No sequence"));
                    return ;
                }
                len = TextUtils::remove(buff, len, TextUtils::WHITES);
                buff[len] = 0;
                sequence.append(buff);
                ti.progress = io->getProgress();
            } while (!ti.cancelFlag && !lineOk);
            len = io->readUntil(buff, READ_BUFF_SIZE, TextUtils::LINE_BREAKS, IOAdapter::Term_Include, &lineOk);
            readLine = QString(QByteArray::fromRawData(buff, len)).trimmed();
            if(!readLine.startsWith(QA)){
                ti.setError(ACEFormat::tr("QA keyword hasn't been found"));
                return ;
            }
            int clearRangeStart = 0;
            int clearRangeEnd = 0;

            clearRangeStart = readsCount(readLine);
            if(-1==clearRangeStart){
                ti.setError(ACEFormat::tr("QA error no clear range"));
                return ;
            }

            clearRangeEnd = clearRange(readLine);
            if(0==clearRangeEnd){
                ti.setError(ACEFormat::tr("QA error no clear range"));
                return ;
            }

            len = sequence.length();
            if(clearRangeStart > clearRangeEnd || clearRangeEnd > len){
                ti.setError(ACEFormat::tr("QA error bad range"));
                return ;
            }

            sequence=sequence.toUpper();
            if(!checkSeq(sequence)){
                ti.setError(ACEFormat::tr("Bad sequence data"));
                return ;
            }

            if (!names.contains(name)) {
                ti.setError(ACEFormat::tr("A name is not match with AF names"));
                return ;
            }else{
                names.remove(name);
            }

            sequence.replace('*',MAlignment_GapChar);
            sequence.replace('N',MAlignment_GapChar);
            sequence.replace('X',MAlignment_GapChar);

            bool isComplement = complMap.take(name);
            if(true == isComplement){
                char *s = sequence.data();
                TextUtils::reverse(s, sequence.length());
            }
            MAlignmentRow row;

            row.setName(name);
            row.setSequence(sequence, posMap.take(name)-1);

            al.addRow(row);

            count--;
            ti.progress = io->getProgress();
        }
        DocumentFormatUtils::assignAlphabet(al);
        if (al.getAlphabet() == NULL) {
             ti.setError( ACEFormat::tr("Alphabet unknown"));
             return;
        }
        MAlignmentObject* obj = new MAlignmentObject(al);
        objects.append(obj);
    }
}

RawDataCheckResult ACEFormat::checkRawData(const QByteArray& rawData, const GUrl&) const {
    static const char* formatTag = "AS";

    if (!rawData.startsWith(formatTag)) {
        return FormatDetection_NotMatched;
    }
    return FormatDetection_AverageSimilarity;
}

Document* ACEFormat::loadDocument(IOAdapter* io, TaskStateInfo& ti, const QVariantMap& fs, DocumentLoadMode) {
    QList <GObject*> objs;
    load(io, objs, ti);

    if (ti.hasError()) {
        qDeleteAll(objs);
        return NULL;
    }
    Document *doc = new Document(this, io->getFactory(), io->getURL(), objs, fs);

    return doc;
}

} //namespace

