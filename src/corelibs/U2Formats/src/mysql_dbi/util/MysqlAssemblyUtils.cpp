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

#include <SamtoolsAdapter.h>

#include <U2Core/U2AssemblyUtils.h>
#include <U2Core/U2DbiUtils.h>
#include <U2Core/U2OpStatus.h>
#include <U2Core/U2SafePoints.h>
#include <U2Core/U2SqlHelpers.h>

#include "MysqlAssemblyUtils.h"
#include "MysqlHelpers.h"

namespace U2 {

namespace {

QByteArray getQuality(const U2AssemblyRead &read) {
    if (read->readSequence.length() == read->quality.length()) {
        return read->quality;
    }

    return QByteArray(read->readSequence.length(), char(0xFF));
}

}   // unnamed namespace

QByteArray MysqlAssemblyUtils::packData(MysqlAssemblyDataMethod method, const U2AssemblyRead &read, U2OpStatus& os) {
    const QByteArray &name = read->name;
    const QByteArray &seq = read->readSequence;
    QByteArray cigarText = U2AssemblyUtils::cigar2String(read->cigar);
    QByteArray qualityString = getQuality(read);
    const QByteArray &rnext = read->rnext;
    QByteArray pnext = QByteArray::number(read->pnext);
    QByteArray aux = SamtoolsAdapter::aux2string(read->aux);

    SAFE_POINT_EXT(method == MysqlAssemblyDataMethod_NSCQ, os.setError(QString("Unsupported packing method: %1").arg(method)), "");

    int nBytes = 1 + name.length() + 1  + seq.length() + 1 + cigarText.length() + 1 + qualityString.length();
    nBytes += 1 + rnext.length() + 1 + pnext.length();
    if (!aux.isEmpty()) {
        if (!aux.isEmpty()) {
            nBytes += 1 + aux.length();
        }
    }

    QByteArray res(nBytes, Qt::Uninitialized);
    char* data = res.data();
    int pos = 0;

    // packing type
    data[pos] = '0';
    pos++;

    // name
    memcpy(data + pos, name.constData(), name.length());
    pos += name.length();
    data[pos] = '\n';
    pos++;

    // sequence
    memcpy(data + pos, seq.constData(), seq.length());
    pos += seq.length();
    data[pos] = '\n';
    pos++;

    // cigar
    memcpy(data + pos, cigarText.constData(), cigarText.length());
    pos += cigarText.length();
    data[pos] = '\n';
    pos++;

    // quality
    memcpy(data + pos, qualityString.constData(), qualityString.length());
    pos += qualityString.length();
    data[pos] = '\n';
    pos++;

    // rnext
    qMemCopy(data + pos, rnext.constData(), rnext.length());
    pos += rnext.length();
    data[pos] = '\n';
    pos++;

    // pnext
    qMemCopy(data + pos, pnext.constData(), pnext.length());
    if (!aux.isEmpty()) {
        pos += pnext.length();
        data[pos] = '\n';
        pos++;

        // aux
        qMemCopy(data + pos, aux.constData(), aux.length());
    }

    return res;
}

void MysqlAssemblyUtils::unpackData(const QByteArray& packedData, U2AssemblyRead &read, U2OpStatus& os) {
    QByteArray &name = read->name;
    QByteArray &sequence = read->readSequence;
    QByteArray &qualityString = read->quality;

    if (packedData.isEmpty()) {
        os.setError(U2DbiL10n::tr("Packed data is empty"));
        return;
    }
    const char* data = packedData.constData();

    // packing type
    if (data[0] != '0') {
        os.setError(U2DbiL10n::tr("Packing method prefix is not supported: %1").arg(data));
        return;
    }

    // name
    int nameStart = 1;
    int nameEnd = packedData.indexOf('\n', nameStart);
    if (nameEnd == -1) {
        os.setError(U2DbiL10n::tr("Data is corrupted, no name end marker found: %1").arg(data));
        return;
    }
    name.append(QByteArray(data + nameStart, nameEnd - nameStart));

    // sequence
    int sequenceStart = nameEnd + 1;
    int sequenceEnd = packedData.indexOf('\n', sequenceStart);
    if (sequenceEnd == -1) {
        os.setError(U2DbiL10n::tr("Data is corrupted, no sequence end marker found: %1").arg(data));
        return;
    }
    sequence.append(data + sequenceStart, sequenceEnd - sequenceStart);

    // cigar
    int cigarStart = sequenceEnd + 1;
    int cigarEnd = packedData.indexOf('\n', cigarStart);
    if (cigarEnd == -1) {
        os.setError(U2DbiL10n::tr("Data is corrupted, no CIGAR end marker found: %1").arg(data));
        return;
    }
    QByteArray cigarText(data + cigarStart, cigarEnd - cigarStart);

    // quality
    int qualityStart = cigarEnd + 1;
    int qualityEnd = qualityStart + sequence.length();
    if (qualityEnd > packedData.length()) {
        assert(packedData.length() == qualityStart);
        qualityEnd = packedData.length();
    }
    qualityString.append(data + qualityStart, qualityEnd - qualityStart);

    if (qualityEnd != packedData.length()) {
        // rnext
        int rnextStart = qualityEnd + 1;
        int rnextEnd = packedData.indexOf('\n', rnextStart);
        if (rnextEnd == -1) {
            os.setError(U2DbiL10n::tr("Data is corrupted, no rnext end marker found: %1").arg(data));
            return;
        }
        read->rnext = QByteArray(data + rnextStart, rnextEnd - rnextStart);

        // pnext
        int pnextStart = rnextEnd + 1;
        int pnextEnd = packedData.indexOf('\n', pnextStart);
        if (pnextEnd == -1) {
            pnextEnd = packedData.length();
        }
        QByteArray pnext(data + pnextStart, pnextEnd - pnextStart);
        bool ok = false;
        read->pnext = pnext.toLongLong(&ok);
        if (!ok) {
            os.setError(U2DbiL10n::tr("Can not convert pnext to a number: %1").arg(pnext.data()));
            return;
        }

        // aux
        int auxStart = pnextEnd + 1;
        int auxEnd = packedData.length();
        read->aux = SamtoolsAdapter::string2aux(QByteArray(data + auxStart, auxEnd - auxStart));
    }

    // parse cigar
    QString err;
    read->cigar = U2AssemblyUtils::parseCigar(cigarText, err);
    if (!err.isEmpty()) {
        os.setError(err);
    }
}

void MysqlAssemblyUtils::calculateCoverage(U2SqlQuery& q, const U2Region& r, U2AssemblyCoverageStat& c, U2OpStatus& os) {
    int csize = c.coverage.size();
    SAFE_POINT(csize > 0, "illegal coverage vector size!", );

    U2Range<int>* cdata = c.coverage.data();
    double basesPerRange = double(r.length) / csize;
    while (q.step() && !os.isCoR()) {
        qint64 startPos = q.getInt64(0);
        qint64 len = q.getInt64(1);
        U2Region readRegion(startPos, len);
        U2Region readCroppedRegion = readRegion.intersect(r);

        if (readCroppedRegion.isEmpty()) {
            continue;
        }

        int firstCoverageIdx = (int)((readCroppedRegion.startPos - r.startPos) / basesPerRange);
        int lastCoverageIdx = (int)((readCroppedRegion.startPos + readCroppedRegion.length - 1 - r.startPos ) / basesPerRange);
        for (int i = firstCoverageIdx; i <= lastCoverageIdx && i < csize; i++) {
            cdata[i].minValue++;
            cdata[i].maxValue++;
        }
    }
}

void MysqlAssemblyUtils::addToCoverage(U2AssemblyCoverageImportInfo& ii, const U2AssemblyRead& read) {
    if (!ii.computeCoverage) {
        return;
    }

    int csize = ii.coverage.coverage.size();

    int startPos = (int)(read->leftmostPos / ii.coverageBasesPerPoint);
    int endPos = (int)((read->leftmostPos + read->effectiveLen - 1) / ii.coverageBasesPerPoint);
    if(endPos > csize - 1) {
        coreLog.trace(QString("addToCoverage: endPos > csize - 1: %1 > %2").arg(endPos).arg(csize-1));
        endPos = csize - 1;
    }
    U2Range<int>* coverageData = ii.coverage.coverage.data();
    for (int i = startPos; i <= endPos && i < csize; i++) {
        coverageData[i].minValue++;
        coverageData[i].maxValue++;
    }
}

}   // namespace U2
