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

#include <QScopedPointer>

#include <U2Core/U2SafePoints.h>
#include <U2Core/U2OpStatus.h>

#include "SamtoolsAdapter.h"

namespace U2 {

static quint8 cigarOp2samtools(U2CigarOp op, U2OpStatus &os) {
    switch(op) {
    case U2CigarOp_M: // alignment match
        return 0;
    case U2CigarOp_I: // inserted
        return 1;
    case U2CigarOp_D: // deleted
        return 2;
    case U2CigarOp_N: // skipped
        return 3;
    case U2CigarOp_S:  // soft-clipped
        return 4;
    case U2CigarOp_H: // hard-clipped
        return 5;
    case U2CigarOp_P: // padded
        return 6;
    case U2CigarOp_EQ:// sequence match
        return 7;
    case U2CigarOp_X: // sequence mismatch
        return 8;
    default:
        os.setError(SamtoolsAdapter::tr("Invalid cigar operation %1, cannot convert to samtools").arg(op));
        return 0;
    };
}

/**
    According to bam.h comments to bam1_cigar macro,
    In the CIGAR array, each element is a 32-bit integer. The
    lower 4 bits gives a CIGAR operation and the higher 28 bits keep the
    length of a CIGAR.
*/
QByteArray SamtoolsAdapter::cigar2samtools(QList<U2CigarToken> cigar, U2OpStatus &os) {
    QByteArray samtoolsCigar;
    foreach(const U2CigarToken &token, cigar) {
        qint8 op = cigarOp2samtools(token.op, os);
        qint32 value = (token.count << 4) | (op & 0xf);

        CHECK_OP(os, samtoolsCigar);
        samtoolsCigar.append((char*)(&value), sizeof(value));
    }
    return samtoolsCigar;
}

/**
    According to bam.h comments to bam1_seq macro,
    Each base is encoded in 4 bits: 1 for A, 2 for C, 4 for G,
    8 for T and 15 for N. Two bases are packed in one byte with the base
    at the higher 4 bits having smaller coordinate on the read.
*/
QByteArray SamtoolsAdapter::sequence2samtools(QByteArray sequence, U2OpStatus &os) {
    int packedLength = (sequence.length() + 1)/2;
    QByteArray samtoolsSequence(packedLength, 0);
    for(int i = 0; i < packedLength; ++i) {
        qint8 value = bam_nt16_table[sequence[2*i]] << 4;
        if(2*i + 1 < sequence.length()) {
            value |= bam_nt16_table[sequence[2*i+1]] & 0xf;
        }
        CHECK_OP(os, samtoolsSequence);
        samtoolsSequence[i] = value;
    }
    return samtoolsSequence;
}

static bool check_seq2samtools(const bam1_t &b, QByteArray seq, U2OpStatus &os) {
    if(b.core.l_qseq != seq.length()) {
        os.setError(QString("Internal SamtoolsAdapter seq check failed: expected length %1, got %2").arg(seq.length()).arg(b.core.l_qseq));
        return false;
    }
    for(int i = 0; i < b.core.l_qseq; ++i) {
        char expected = seq[i];
        char actual = bam_nt16_rev_table[bam1_seqi(bam1_seq(&b), i)];
        if(expected != actual) {
            os.setError(QString("Internal SamtoolsAdapter seq check failed: expected %1, got %2 at pos %3").arg(expected).arg(actual).arg(i));
            return false;
        }
    }
    return true;
}

static bool check_qual(const bam1_t &b, QByteArray qual, U2OpStatus &os) {
    if(b.core.l_qseq != qual.length()) {
        os.setError(QString("Internal SamtoolsAdapter qual check failed: expected length %1, got %2").arg(qual.length()).arg(b.core.l_qseq));
        return false;
    }
    uint8_t * bQual = bam1_qual(&b);
    int lastBQualOffset = &bQual[b.core.l_qseq-1] - b.data;
    if(lastBQualOffset > b.data_len) {
        os.setError(QString("Internal SamtoolsAdapter qual check failed: quality out of bounds: data_len is %1, but last qual offset is %2").arg(b.data_len).arg(lastBQualOffset));
        return false;
    }
    for(int i = 0; i < b.core.l_qseq; ++i) {
        uint8_t expected = qual[i];
        uint8_t actual = bQual[i];
        if(expected != actual) {
            os.setError(QString("Internal SamtoolsAdapter qual check failed: expected %1, got %2 at pos %3").arg(expected).arg(actual).arg(i));
            return false;
        }
    }
    return true;
}

typedef quint8 *data_ptr;
inline static void copyArray(data_ptr &dest, const QByteArray array) {
    memcpy(dest, array.constData(), array.length());
    dest += array.length();
}

static const int SAMTOOLS_QUALITY_OFFSET = 33;
static const char SAMTOOLS_QUALITY_OFF_CHAR = 0xff;
inline static void copyQuality(data_ptr &dest, const QByteArray &quality) {
    QByteArray samtoolsQuality = quality;
    // shift qulity for samtools
    if (quality.size() > 0 && SAMTOOLS_QUALITY_OFF_CHAR != quality.data()[0]) {
        for (int i=0; i<quality.size(); i++) {
            samtoolsQuality.data()[i] -= SAMTOOLS_QUALITY_OFFSET;
        }
    }
    copyArray(dest, samtoolsQuality);
}

inline static void copyChar(data_ptr &dest, quint8 c) {
    *dest = c;
    ++dest;
}

void SamtoolsAdapter::reads2samtools(U2DbiIterator<U2AssemblyRead> *reads, U2OpStatus &os, ReadsContainer & result) {
    while(reads->hasNext()) {
        U2AssemblyRead r = reads->next();
        bam1_t resRead;
        read2samtools(r, os, resRead);
        CHECK_OP(os, );
        result.append(resRead);
    }
}

void SamtoolsAdapter::read2samtools(const U2AssemblyRead &read, const ReadsContext &ctx, U2OpStatus &os, bam1_t &result) {
    read2samtools(read, os, result);
    CHECK_OP(os, );
    result.core.tid = ctx.getReadAssemblyNum();

    bool paired = false;
    U2AssemblyRead pRead;
    if (ReadFlagsUtils::isPairedRead(read->flags)) {
        pRead = ctx.getPairedRead(read, paired, os);
    }
    if (paired) {
        // TODO: mate assembly id could be not the same!
        result.core.mtid = ctx.getReadAssemblyNum();
        result.core.mpos = pRead->leftmostPos;
    }
}

void SamtoolsAdapter::read2samtools(const U2AssemblyRead &r, U2OpStatus &os, bam1_t &resRead) {
    bam1_core_t &core = resRead.core;

    memset(&resRead, 0, sizeof(resRead));

    core.tid = 0;
    core.pos = r->leftmostPos;

    core.bin = bam_reg2bin(r->leftmostPos, r->leftmostPos+r->effectiveLen);
    core.qual = r->mappingQuality;
    core.l_qname = r->name.length() + 1;

    core.flag = r->flags;
    core.n_cigar = r->cigar.length();

    core.l_qseq = r->readSequence.size();
    core.mtid = -1;
    core.mpos = -1;
    core.isize = 0;

    QByteArray quality = r->quality;
    if(quality.isEmpty()) {
        quality = QByteArray(core.l_qseq, 0xff);
    }

    QByteArray cigar = cigar2samtools(r->cigar, os);
    QByteArray seq = sequence2samtools(r->readSequence, os);
    int dataLen = r->name.length() + 1 + cigar.length() + seq.length() + quality.length();
    quint8 * data = new quint8[dataLen];
    quint8 * dest = data;
    copyArray(dest, r->name);
    copyChar(dest, '\0');
    copyArray(dest, cigar);
    copyArray(dest, seq);
    copyQuality(dest, quality);

    // No tags. TODO: is this good?
    resRead.l_aux = 0;
    resRead.data_len = resRead.m_data = dataLen;
    resRead.data = data;

    CHECK_OP(os,);

    // TODO: remove checks, make tests
    //CHECK_EXT(cigar2samtools(r->cigar, os).size() == 4*r->cigar.size(), os.setError("Internal SamtoolsAdapter cigar length check failed"),);
    //check_seq2samtools(resRead, r->readSequence, os);
    //CHECK_OP(os,);
    //check_qual(resRead, quality, os);
    //CHECK_OP(os,);
}

static bool startPosLessThan(const bam1_t &a, const bam1_t &b) {
    return a.core.pos < b.core.pos;
}

void ReadsContainer::sortByStartPos() {
    qSort(vector.begin(), vector.end(), startPosLessThan);
}

ReadsContainer::~ReadsContainer() {
    foreach(const bam1_t &b, vector) {
        delete b.data;
    }
}

/************************************************************************/
/* ReadsContext */
/************************************************************************/
ReadsContext::ReadsContext(U2AssemblyDbi *_assemblyDbi,
    const U2DataId &_assemblyId,
    const QMap<U2DataId, int> &_assemblyNumMap)
: assemblyDbi(_assemblyDbi), assemblyId(_assemblyId), assemblyNumMap(_assemblyNumMap)
{

}

U2AssemblyRead ReadsContext::getPairedRead(const U2AssemblyRead &read, bool &found, U2OpStatus &os) const {
    found = false;
    CHECK(ReadFlagsUtils::isPairedRead(read->flags), U2AssemblyRead());

    QScopedPointer< U2DbiIterator<U2AssemblyRead> > it(assemblyDbi->getReadsByName(assemblyId, read->name, os));
    CHECK_OP(os, U2AssemblyRead());

    QList<U2AssemblyRead> result;
    while (it->hasNext()) {
        U2AssemblyRead r = it->next();
        if(r->id != read->id) {
            found = true;
            return r;
        }
    }
    return U2AssemblyRead();
}

int ReadsContext::getReadAssemblyNum() const {
    return assemblyNumMap.value(assemblyId, 0);
}

int ReadsContext::getAssemblyNum(const U2DataId &assemblyId) const {
    return assemblyNumMap.value(assemblyId, 0);
}

} // namespace
