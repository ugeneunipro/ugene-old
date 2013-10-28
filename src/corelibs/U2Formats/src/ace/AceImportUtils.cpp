/**
 * UGENE - Integrated Bioinformatics Tools.
 * Copyright (C) 2008-2013 UniPro <ugene@unipro.ru>
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

#include <U2Core/TextUtils.h>
#include <U2Core/U2AbstractDbi.h>
#include <U2Core/U2SafePoints.h>

#include "ace/AceImportUtils.h"

namespace U2 {

///////////////////////////////////
//// Assembly
///////////////////////////////////

Assembly::Assembly() {
}

const Assembly::Sequence & Assembly::getReference() {
    return reference;
}

void Assembly::setReference(const Sequence &_reference) {
    reference = _reference;
    if (name.isEmpty()) {
        name = reference.name;
        if (name.endsWith("_ref")) {
            name.chop(QString("_ref").length());
        }
    }
}

QList<U2AssemblyRead> Assembly::getReads() const {
    return convertReads();
}

void Assembly::addRead(const Sequence &read) {
    reads << read;
}

void Assembly::setReads(const QList<Sequence> &_reads) {
    reads = _reads;
}

int Assembly::getReadsCount() const {
    return reads.count();
}

const QByteArray &Assembly::getName() const {
    return name;
}

void Assembly::setName(const QByteArray &_name) {
    name = _name;
}

bool Assembly::isValid() const {
    bool ok = reference.isValid();
    foreach(Sequence read, reads) {
        ok &= read.isValid();
    }
    return ok;
}

QList<U2AssemblyRead> Assembly::convertReads() const {
    QList<U2AssemblyRead> res;
    foreach (Assembly::Sequence localRead, reads) {
        U2AssemblyRead r(new U2AssemblyReadData);
        r->name = localRead.name;
        r->leftmostPos = localRead.offset;
        r->effectiveLen = localRead.data.length();
        r->readSequence = localRead.data;
        if (localRead.isComplemented) {
            r->flags = Reverse;
        }
        r->cigar = QList<U2CigarToken>() << U2CigarToken(U2CigarOp_M, localRead.data.length());
        res << r;
    }
    return res;
}

///////////////////////////////////
//// AceReader
///////////////////////////////////
const int AceReader::READ_BUFF_SIZE = 4096;
const int AceReader::CONTIG_COUNT_POS = 1;
const int AceReader::READS_COUNT_POS = 3;
const int AceReader::READS_POS = 3;
const int AceReader::COMPLEMENT_POS = 2;
const int AceReader::PADDED_START_POS = 3;
const int AceReader::FIRST_QA_POS = 3;
const int AceReader::LAST_QA_POS = 4;

const QByteArray AceReader::AS = "AS";
const QByteArray AceReader::CO = "CO";
const QByteArray AceReader::BQ = "BQ";
const QByteArray AceReader::AF = "AF";
const QByteArray AceReader::RD = "RD";
const QByteArray AceReader::QA = "QA";
const QByteArray AceReader::COMPLEMENT = "C";
const QByteArray AceReader::UNCOMPLEMENT = "U";

AceReader::AceReader(IOAdapter& _io, U2OpStatus &_os) :
    io(&_io),
    os(&_os),
    currentContig(0) {
    QByteArray readBuff(READ_BUFF_SIZE + 1, 0);
    char* buff = readBuff.data();
    qint64 len = 0;

    skipBreaks(io, buff, &len);
    CHECK_OP((*os), );
    QByteArray headerLine = (QByteArray::fromRawData(buff, len)).trimmed();
    CHECK_EXT(headerLine.startsWith(AS), os->setError(tr("First line is not an ace header")), );

    contigsCount = getContigCount(headerLine);
    CHECK_OP((*os), );
    CHECK_EXT(contigsCount > 0, os->setError((tr("There is no assemblies in input file"))), );
}

Assembly AceReader::getAssembly() {
    Assembly result;
    Assembly::Sequence reference;

    QByteArray readBuff(READ_BUFF_SIZE + 1, 0);
    char* buff = readBuff.data();
    qint64 len = 0;
    int readsCount = 0;
    QByteArray headerLine;

    QSet<QByteArray> names;
    QMap<QByteArray, bool> complMap;

    CHECK_EXT(currentContig < contigsCount, os->setError(tr("There are not enough assemblies")), result);

    do {
        skipBreaks(io, buff, &len);
        CHECK_OP((*os), result);
        headerLine = (QByteArray::fromRawData(buff, len)).trimmed();
    } while (!headerLine.startsWith(CO));

    readsCount = getReadsCount(headerLine);
    CHECK_OP((*os), result);

    // consensus, is set as reference in assembly
    parseConsensus(io, buff, names, headerLine, reference);
    CHECK_OP((*os), result);

    // read AF tag
    QMap<QByteArray, int> posMap;
    parseAfTag(io, buff, readsCount, posMap, complMap, names);
    CHECK_OP((*os), result);

    int smallestOffset = getSmallestOffset(posMap);
    if (smallestOffset < 0) {
        QByteArray gaps(qAbs(smallestOffset), '*');
        reference.data.prepend(gaps);
    }
    result.setReference(reference);

    // read RD and QA tags
    while (readsCount > 0) {
        Assembly::Sequence read;

        parseRdAndQaTag(io, buff, names, read);
        CHECK_OP((*os), result);

        read.isComplemented = complMap.take(read.name);
        read.offset = posMap.value(read.name) - 1;
        if (smallestOffset < 0) {
            read.offset += qAbs(smallestOffset);
        }

        result.addRead(read);
        readsCount--;
    }

    currentContig++;

    return result;
}

bool AceReader::isFinish() {
    return currentContig >= contigsCount || io->isEof();
}

void AceReader::skipBreaks(IOAdapter *io, char *buff, qint64 *len) {
    bool lineOk = true;
    *len = io->readUntil(buff, READ_BUFF_SIZE, TextUtils::LINE_BREAKS, IOAdapter::Term_Include, &lineOk);
    CHECK_EXT(*len != 0, os->setError(tr("Unexpected end of file")), );
    CHECK_EXT(lineOk, os->setError(tr("Line is too long")), );
}

int AceReader::getContigCount(const QByteArray &cur_line) {
    QByteArray line = cur_line;
    int contigC = getSubString(line, CONTIG_COUNT_POS);
    CHECK_OP((*os), 0);
    CHECK_EXT(-1 != contigC, os->setError(tr("No contig count tag in the header line")), -1);
    return contigC;
}

int AceReader::getSubString(QByteArray &line, int pos) {
    int curIdx = 0;
    char space = ' ';

    line = line.simplified();

    for (int i = 0; i < pos; i++) {
        curIdx = line.indexOf(space);
        CHECK_EXT(-1 != curIdx, os->setError(tr("Not enough parameters in current line")), -1);
        line = line.mid(curIdx + 1);
    }

    curIdx = line.indexOf(space);
    if (-1 != curIdx) {
        line = line.mid(0, curIdx);
    }

    bool ok = false;
    int result = line.toInt(&ok);
    CHECK_EXT(ok, os->setError(tr("Parameter is not a digit")), -1);

    return result;
}

int AceReader::getReadsCount(const QByteArray &cur_line) {
    QByteArray line = cur_line;
    int readsCount = getSubString(line, READS_COUNT_POS);
    CHECK_OP_EXT((*os), os->setError(tr("There is no note about reads count")), 0);
    return readsCount;
}

void AceReader::parseConsensus(IOAdapter *io, char *buff, QSet<QByteArray> &names, QByteArray &headerLine, Assembly::Sequence &consensus) {
    char aceBStartChar = 'B';
    QBitArray aceBStart = TextUtils::createBitMap(aceBStartChar);
    qint64 len = 0;
    bool ok = true;
    QByteArray line;

    consensus.name = getName(headerLine);
    CHECK_EXT(!names.contains(consensus.name), os->setError(tr("A name is duplicated")), );

    names.insert(consensus.name);
    consensus.name += "_ref";

    do {
        len = io->readUntil(buff, READ_BUFF_SIZE, aceBStart, IOAdapter::Term_Exclude, &ok);
        CHECK_EXT(len > 0, os->setError(tr("No consensus")), );

        len = TextUtils::remove(buff, len, TextUtils::WHITES);
        buff[len] = 0;
        consensus.data.append(buff);
        os->setProgress(io->getProgress());
    } while (!ok);

    len = io->readUntil(buff, READ_BUFF_SIZE, TextUtils::LINE_BREAKS, IOAdapter::Term_Include, &ok);
    line = (QByteArray::fromRawData(buff, len)).trimmed();
    CHECK_EXT(line.startsWith(BQ), os->setError(tr("BQ keyword hasn't been found")), );

    consensus.data = consensus.data.toUpper();
    CHECK_EXT(checkSeq(consensus.data), os->setError(tr("Bad consensus data")), );
}

QByteArray AceReader::getName(const QByteArray &line) {
    int curIdx = 0;
    char space = ' ';

    QByteArray name = line.simplified();

    curIdx = name.indexOf(space);
    CHECK_EXT(-1 != curIdx, os->setError(tr("Can't find a sequence name in current line")), "");

    name = name.mid(curIdx + 1);

    curIdx = name.indexOf(space);
    CHECK_EXT(-1 != curIdx, os->setError(tr("Can't find a sequence name in current line")), "");

    name = name.mid(0, curIdx);
    CHECK_EXT(!name.isEmpty(), os->setError(tr("An empty sequence name")), "");
    return name;
}

bool AceReader::checkSeq(const QByteArray &seq) {
    for (int i = 0; i < seq.length(); i++){
        if (seq[i] != 'A' && seq[i] != 'C' && seq[i] != 'G' && seq[i] != 'T' && seq[i] != 'N' && seq[i] != '*' && seq[i] != 'X') {
            return false;
        }
    }
    return true;
}

void AceReader::parseAfTag(U2::IOAdapter *io, char *buff, int count, QMap<QByteArray, int> &posMap, QMap<QByteArray, bool> &complMap, QSet<QByteArray> &names) {
    int readsCount = count;
    QByteArray readLine;
    QByteArray name;
    qint64 len = 0;
    int readPos = 0;
    int complStrand = 0;

    while (readsCount > 0) {
        do {    // skip unused BQ part
            skipBreaks(io, buff, &len);
            CHECK_OP((*os), );
            readLine = (QByteArray::fromRawData(buff, len)).trimmed();
        } while (!readLine.startsWith(AF));
        CHECK_EXT(readLine.startsWith(AF), os->setError(tr("There is no AF note")), );

        name = getName(readLine);
        CHECK_OP((*os), );

        readPos = readsPos(readLine);
        CHECK_OP((*os), );
        complStrand = readsComplement(readLine);
        CHECK_OP((*os), );

        posMap.insert(name, readPos);

        bool cur_compl = (complStrand == 1);
        complMap.insert(name, cur_compl);

        CHECK_EXT(!names.contains(name), os->setError(tr("A name is duplicated")), );
        names.insert(name);

        readsCount--;
        os->setProgress(io->getProgress());
    }
}

int AceReader::readsPos(const QByteArray &cur_line) {
    QByteArray line = cur_line;
    char space = ' ';

    prepareLine(line, READS_POS);
    CHECK_EXT(!line.contains(space), os->setError(tr("Bad AF note")), 0);

    bool ok = true;
    int result = line.toInt(&ok);
    CHECK_EXT(ok, os->setError(tr("Bad AF note")), 0);

    return result;
}

int AceReader::prepareLine(QByteArray &line, int pos) {
    int curIdx = 0;
    char space = ' ';

    line = line.simplified();

    for (int i = 0; i < pos; i++) {
        curIdx = line.indexOf(space);
        if (-1 == curIdx) {
            return -1;
        }

        line = line.mid(curIdx + 1);
    }

    return curIdx;
}

int AceReader::readsComplement(const QByteArray &cur_line) {
    QByteArray line = cur_line;
    prepareLine(line, COMPLEMENT_POS);

    if (line.startsWith(UNCOMPLEMENT)) {
        return 0;
    } else if (line.startsWith(COMPLEMENT)) {
        return 1;
    } else {
        os->setError(tr("Bad AF note"));
        return -1;
    }
}

int AceReader::paddedStartCons(const QByteArray &cur_line) {
    QByteArray line = cur_line;
    getSubString(line, PADDED_START_POS);
    CHECK_OP((*os), 0);

    bool ok = true;
    int result = line.toInt(&ok);
    CHECK_EXT(ok, os->setError(tr("Bad AF note")), 0);

    return result;
}

int AceReader::getSmallestOffset(const QMap<QByteArray, int> &posMap) {
    int smallestOffset = 0;
    foreach (int value, posMap) {
        smallestOffset = qMin(smallestOffset, value - 1);
    }

    return smallestOffset;
}

void AceReader::parseRdAndQaTag(U2::IOAdapter *io, char *buff, QSet<QByteArray> &names, Assembly::Sequence &read) {
    QByteArray line;
    qint64 len = 0;
    bool ok = true;
    char aceQStartChar = 'Q';
    QBitArray aceQStart = TextUtils::createBitMap(aceQStartChar);

    do {    // skip unused BS part
        skipBreaks(io, buff, &len);
        CHECK_OP((*os), );
        line = (QByteArray::fromRawData(buff, len)).trimmed();
    } while (!line.startsWith(RD));
    CHECK_EXT(line.startsWith(RD), os->setError(tr("There is no read note")), );

    read.name = getName(line);
    CHECK_OP((*os), );

    do {
        len = io->readUntil(buff, READ_BUFF_SIZE, aceQStart, IOAdapter::Term_Exclude, &ok);
        CHECK_EXT(len > 0, os->setError(tr("No sequence")), );
        len = TextUtils::remove(buff, len, TextUtils::WHITES);
        buff[len] = 0;
        read.data.append(buff);
        os->setProgress(io->getProgress());
    } while (!ok);

    len = io->readUntil(buff, READ_BUFF_SIZE, TextUtils::LINE_BREAKS, IOAdapter::Term_Include, &ok);
    line = (QByteArray::fromRawData(buff, len)).trimmed();
    CHECK_EXT(line.startsWith(QA), os->setError(tr("QA keyword hasn't been found")), );

    int clearRangeStart = getClearRangeStart(line);
    CHECK_OP((*os), );
    int clearRangeEnd = getClearRangeEnd(line);
    CHECK_OP((*os), );

    CHECK_EXT(clearRangeStart <= clearRangeEnd && clearRangeEnd <= read.data.length(), os->setError(tr("QA error bad range")), );

    read.data = read.data.toUpper();
    CHECK_EXT(checkSeq(read.data), os->setError(tr("Bad sequence data")), );

    CHECK_EXT(names.contains(read.name), os->setError(tr("A name is not match with AF names")), );
    names.remove(read.name);
}

int AceReader::getClearRangeStart(const QByteArray &cur_line) {
    QByteArray line = cur_line;
    int result = getSubString(line, FIRST_QA_POS);
    CHECK_OP_EXT((*os), os->setError(tr("Can't find clear range start in current line")), 0);
    CHECK_EXT(result > 0, os->setError(tr("Clear range start is invalid")), 0);
    return result;
}

int AceReader::getClearRangeEnd(const QByteArray &cur_line) {
    QByteArray line = cur_line;
    int result = getSubString(line, LAST_QA_POS);
    CHECK_OP_EXT((*os), os->setError(tr("Can't find clear range end in current line")), 0);
    CHECK_EXT(result > 0, os->setError(tr("Clear range end is invalid")), 0);
    return result;
}

///////////////////////////////////
//// AceIterator
///////////////////////////////////

AceIterator::AceIterator(AceReader &_reader, U2OpStatus &_os) :
    reader(&_reader),
    os(&_os) {
}

bool AceIterator::hasNext() {
    return !reader->isFinish();
}

Assembly AceIterator::next() {
    CHECK_EXT(hasNext(), os->setError(QObject::tr("There is no next element")), Assembly());
    return reader->getAssembly();
}

}   // namespace U2
