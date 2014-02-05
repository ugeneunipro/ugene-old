/**
 * UGENE - Integrated Bioinformatics Tools.
 * Copyright (C) 2008-2014 UniPro <ugene@unipro.ru>
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

#include "AceFormat.h"
#include <U2Formats/DocumentFormatUtils.h>
#include <U2Core/IOAdapter.h>
#include <U2Core/L10n.h>
#include <U2Core/GObjectTypes.h>
#include <U2Core/MAlignmentImporter.h>
#include <U2Core/MAlignmentObject.h>
#include <U2Core/TextUtils.h>
#include <U2Core/MSAUtils.h>
#include <U2Core/GObjectRelationRoles.h>
#include <U2Core/U2OpStatus.h>
#include <U2Core/U2SafePoints.h>
#include <U2Core/U2AlphabetUtils.h>

namespace U2 {

const QString ACEFormat::CO = "CO";
const QString ACEFormat::RD = "RD";
const QString ACEFormat::QA = "QA";
const QString ACEFormat::AS = "AS";
const QString ACEFormat::AF = "AF";
const QString ACEFormat::BQ = "BQ";

ACEFormat::ACEFormat(QObject* p) : DocumentFormat(p, DocumentFormatFlags(0), QStringList("ace")) {
    formatName = tr("ACE");
    formatDescription = tr("ACE is a format used for storing information about genomic confgurations");
    supportedObjectTypes+=GObjectTypes::ASSEMBLY;
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

static inline void skipBreaks(U2::IOAdapter *io, U2OpStatus &ti, char* buff, qint64* len){
    bool lineOk = true;
    *len = io->readUntil(buff, READ_BUFF_SIZE, TextUtils::LINE_BREAKS, IOAdapter::Term_Include, &lineOk);
    if (*len == 0) { //end if stream
        ti.setError(ACEFormat::tr("Unexpected end of file"));
        return;
    }
    if (!lineOk) {
        ti.setError(ACEFormat::tr("Line is too long"));
        return;
    }
}
static inline void parseConsensus(U2::IOAdapter *io, U2OpStatus &ti, char* buff, QString& consName, QSet<QString> &names, QString& headerLine, QByteArray& consensus){
    char aceBStartChar = 'B';
    QBitArray aceBStart = TextUtils::createBitMap(aceBStartChar);
    qint64 len = 0;
    bool ok = true;
    QString line;
    consName = getName(headerLine);
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
        len = io->readUntil(buff, READ_BUFF_SIZE, aceBStart, IOAdapter::Term_Exclude, &ok);
        if (len <= 0) {
            ti.setError(ACEFormat::tr("No consensus"));
            return ;
        }
        len = TextUtils::remove(buff, len, TextUtils::WHITES);
        buff[len] = 0;
        consensus.append(buff);
        ti.setProgress(io->getProgress());
    } while (!ti.isCoR() && !ok);
    len = io->readUntil(buff, READ_BUFF_SIZE, TextUtils::LINE_BREAKS, IOAdapter::Term_Include, &ok);
    line = QString(QByteArray::fromRawData(buff, len)).trimmed();
    if(!line.startsWith("BQ")){
        ti.setError(ACEFormat::tr("BQ keyword hasn't been found"));
        return ;
    }
    consensus=consensus.toUpper();
    if(!checkSeq(consensus)){
        ti.setError(ACEFormat::tr("Bad consensus data"));
        return ;
    }
    consensus.replace('*',MAlignment_GapChar);
}

static inline void parseAFTag(U2::IOAdapter *io, U2OpStatus &ti, char* buff, int count, QMap< QString, int> &posMap, QMap< QString, bool> &complMap, QSet<QString> &names){
    int count1 = count;
    QString readLine;
    QString name;
    qint64 len = 0;
    int readPos = 0;
    int complStrand = 0;
    int paddedStart = 0;
    while (!ti.isCoR() && count1>0) {
        do{
            skipBreaks(io, ti, buff, &len);
            if(ti.hasError()){
                return;
            }
            readLine = QString(QByteArray::fromRawData(buff, len)).trimmed();
        }while (!readLine.startsWith("AF"));

        name = getName(readLine);
        if(!readLine.startsWith("AF") || "" == name){
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
        ti.setProgress(io->getProgress());
    } 
}

static inline void parseRDandQATag(U2::IOAdapter *io, U2OpStatus &ti, char* buff, QSet<QString> &names, QString& name, QByteArray& sequence){
    QString line;
    qint64 len = 0;
    bool ok = true;
    char aceQStartChar = 'Q';
    QBitArray aceQStart = TextUtils::createBitMap(aceQStartChar);
    do{
        skipBreaks(io, ti, buff, &len);
        if(ti.hasError()){
            return;
        }
        line = QString(QByteArray::fromRawData(buff, len)).trimmed();
    }while (!line.startsWith("RD"));

    name = getName(line);
    if(!line.startsWith("RD") || "" == name){
        ti.setError(ACEFormat::tr("There is no read note"));
        return ;
    }

    sequence.clear();
    do {
        len = io->readUntil(buff, READ_BUFF_SIZE, aceQStart, IOAdapter::Term_Exclude, &ok);
        if (len <= 0) {
            ti.setError(ACEFormat::tr("No sequence"));
            return ;
        }
        len = TextUtils::remove(buff, len, TextUtils::WHITES);
        buff[len] = 0;
        sequence.append(buff);
        ti.setProgress(io->getProgress());
    } while (!ti.isCoR() && !ok);
    len = io->readUntil(buff, READ_BUFF_SIZE, TextUtils::LINE_BREAKS, IOAdapter::Term_Include, &ok);
    line = QString(QByteArray::fromRawData(buff, len)).trimmed();
    if(!line.startsWith("QA")){
        ti.setError(ACEFormat::tr("QA keyword hasn't been found"));
        return ;
    }
    int clearRangeStart = 0;
    int clearRangeEnd = 0;

    clearRangeStart = readsCount(line);
    if(-1==clearRangeStart){
        ti.setError(ACEFormat::tr("QA error no clear range"));
        return ;
    }

    clearRangeEnd = clearRange(line);
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
}

/**
 * Offsets in an ACE file are specified relatively to the reference sequence,
 * so "pos" can be negative.
 */
static inline int getSmallestOffset(const QMap<QString, int>& posMap) {
    int smallestOffset = 0;
    foreach (int value, posMap) {
        smallestOffset = qMin(smallestOffset, value - 1);
    }

    return smallestOffset;
}

void ACEFormat::load(IOAdapter *io, const U2DbiRef& dbiRef, QList<GObject*> &objects, U2OpStatus &os) {
    FAIL(tr("Not implemented"), );

    QByteArray readBuff(READ_BUFF_SIZE+1, 0);
    char* buff = readBuff.data();
    qint64 len = 0;
    int count = 0;

    QByteArray sequence;
    QSet<QString> names;
    QMap< QString, bool> complMap;

     //skip leading whites if present
    bool lineOk = true;
    skipBreaks(io, os, buff, &len);
    if(os.hasError()){
        return;
    }
    QString headerLine = QString(QByteArray::fromRawData(buff, len)).trimmed();

    if (!headerLine.startsWith(AS)) {
        os.setError(ACEFormat::tr("First line is not an ace header"));
        return ;
    }
    int contigC = contigCount(headerLine);
    if(-1==contigC){
         os.setError(ACEFormat::tr("No contig count tag in the header line"));
        return ;
    }
    for(int i =0; i < contigC; i++){
        if(i==0){
            QBitArray nonWhites = ~TextUtils::WHITES;
            io->readUntil(buff, READ_BUFF_SIZE, nonWhites, IOAdapter::Term_Exclude, &lineOk);
            //read header
            skipBreaks(io, os, buff, &len);
            if(os.hasError()){
                return;
            }
            headerLine = QString(QByteArray::fromRawData(buff, len)).trimmed();
            if (!headerLine.startsWith(CO)) {
                os.setError(ACEFormat::tr("Must be CO keyword"));
                return ;
            }
        }else{
            do{
                skipBreaks(io, os, buff, &len);
                if(os.hasError()){
                    return;
                }
                headerLine = QString(QByteArray::fromRawData(buff, len)).trimmed();
            }while (!headerLine.startsWith(CO));
        }
        count = readsCount(headerLine);
        if(-1 == count){
            os.setError(ACEFormat::tr("There is no note about reads count"));
            return ;
        }
        //consensus
        QString name;
        QByteArray consensus;
        QString consName;

        parseConsensus(io, os, buff, consName, names, headerLine, consensus);
        if (os.hasError()){
            return;
        }

        MAlignment al(consName);
        al.addRow(consName, consensus, os);
        CHECK_OP(os, );

        //AF
        QMap< QString, int> posMap;
        parseAFTag(io, os, buff, count, posMap, complMap, names);
        CHECK_OP(os, );

        int smallestOffset = getSmallestOffset(posMap);
        if (smallestOffset < 0) {
            al.insertGaps(0, 0, qAbs(smallestOffset), os);
            CHECK_OP(os, );
        }

        //RD and QA
        while (!os.isCoR() && count>0) {
            parseRDandQATag(io, os, buff, names, name, sequence);
            CHECK_OP(os, );
            
            bool isComplement = complMap.take(name);
            int pos = posMap.value(name) - 1;
            if (smallestOffset < 0) {
                pos += qAbs(smallestOffset);
            }
            QString rowName(name);
            if(true == isComplement){
                rowName.append("(rev-compl)");
            }

            QByteArray offsetGaps;
            offsetGaps.fill(MAlignment_GapChar, pos);
            sequence.prepend(offsetGaps);
            al.addRow(rowName, sequence, os);
            CHECK_OP(os, );

            count--;
            os.setProgress(io->getProgress());
        }
        U2AlphabetUtils::assignAlphabet(al);
        CHECK_EXT(al.getAlphabet() != NULL, ACEFormat::tr("Alphabet unknown"), );

        U2EntityRef msaRef = MAlignmentImporter::createAlignment(dbiRef, al, os);
        CHECK_OP(os, );

        MAlignmentObject* obj = new MAlignmentObject(al.getName(), msaRef);
        objects.append(obj);
    }
}

FormatCheckResult ACEFormat::checkRawData(const QByteArray& rawData, const GUrl&) const {
    static const char* formatTag = "AS";

    if (!rawData.startsWith(formatTag)) {
        return FormatDetection_NotMatched;
    }
    return FormatDetection_AverageSimilarity;
}

Document* ACEFormat::loadDocument(IOAdapter* io, const U2DbiRef& dbiRef, const QVariantMap& fs, U2OpStatus& os) {
    QList <GObject*> objs;
    load(io, dbiRef, objs, os);

    CHECK_OP_EXT(os, qDeleteAll(objs), NULL);
    
    if(objs.isEmpty()){
        os.setError(ACEFormat::tr("File doesn't contain any msa objects"));
        return NULL;
    }
    Document *doc = new Document(this, io->getFactory(), io->getURL(), dbiRef, objs, fs);

    return doc;
}

} //namespace

