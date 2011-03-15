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

#include <U2Core/U2AssemblyUtils.h>

#include <U2Core/TextUtils.h>
#include <U2Core/U2OpStatus.h>

#include <QtCore/QRegExp>

namespace U2 {

U2CigarOp U2AssemblyUtils::char2Cigar(char c, QString& err) {
    char cu = TextUtils::UPPER_CASE_MAP[c];
    switch (c) {
        case 'D':
            return U2CigarOp_D; // deleted
        case 'I':
            return U2CigarOp_I; // inserted
        case 'H':
            return U2CigarOp_H; // hard-clipped
        case 'M':
            return U2CigarOp_M; // matched
        case 'N':
            return U2CigarOp_N; // skipped
        case 'P':
            return U2CigarOp_P; // padded
        case 'S':
            return U2CigarOp_S; // soft-clipped
    }
    err = tr("Invalid CIGAR op: '%1'!").arg(c);
    return U2CigarOp_Invalid;
}

char U2AssemblyUtils::cigar2Char(U2CigarOp op) {
    char c;
    switch(op) {
            case U2CigarOp_D: c = 'D'; break;
            case U2CigarOp_I: c = 'I'; break;
            case U2CigarOp_H: c = 'H'; break;
            case U2CigarOp_M: c = 'M'; break;
            case U2CigarOp_N: c = 'N'; break;
            case U2CigarOp_P: c = 'P'; break;
            case U2CigarOp_S: c = 'S'; break;
            default: assert(0); c = '?';
    }
    return c;
}

QList<U2CigarToken> U2AssemblyUtils::parseCigar(const QByteArray& cigarString, QString& err) {
    QList<U2CigarToken> result;
    int pos = 0, len = cigarString.length();
    const char* cigar = cigarString.constData();
    int count = 0;
    // count numbers
    while (pos < len ) {
        char c = cigar[pos++];
        if (c >= '0' && c <= '9') {
            int n = c - '0';
            count = count * 10 + n;
            continue;
        } 
        U2CigarOp op = char2Cigar(c, err);
        if (!err.isEmpty()) {
            break;
        }
        result.append(U2CigarToken(op, count));
        count = 0;
    }
    return result;
}


QByteArray U2AssemblyUtils::cigar2String(const QList<U2CigarToken>& cigar) {
    QByteArray res;
    foreach(const U2CigarToken& t, cigar) {
        if (t.op != U2CigarOp_Invalid) {
            res = res + QByteArray::number(t.count) + cigar2Char(t.op);
        }
    }
    return res;
}

qint64 U2AssemblyUtils::getCigarExtraLength(const QList<U2CigarToken>& cigar) {
    qint64 res = 0;
    foreach(const U2CigarToken& t, cigar) {
        //TODO: recheck and test
        //TODO: add handling of other operations 
        switch(t.op) {
            case U2CigarOp_I: res-=t.count; break;
            case U2CigarOp_D: 
            case U2CigarOp_N: 
                res+=t.count; 
                break;
        }
    }
    return res;
}

static QByteArray prepareCigarChars() {
    return QByteArray("0123456789DIHMNPS");
}

QByteArray U2AssemblyUtils::getCigarAlphabetChars() {
    static QByteArray res = prepareCigarChars();
    return res;
}

//////////////////////////////////////////////////////////////////////////
// reads iterator impl

U2AssemblyReadsIteratorImpl::U2AssemblyReadsIteratorImpl(const QList<U2AssemblyRead>& _reads) : pos (0), reads(_reads){

}

/** returns true if there are more reads to iterate*/
bool U2AssemblyReadsIteratorImpl::hasNext() {
    return pos + 1 < reads.size();     
}

/** returns next read or error */
U2AssemblyRead U2AssemblyReadsIteratorImpl::next(U2OpStatus& os) {
    if (!hasNext()) {
        os.setError(U2AssemblyUtils::tr("No elements left!"));
        return U2AssemblyRead();
    }
    pos++;
    return reads.at(pos);
}



} //namespace
