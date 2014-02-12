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

#include <QtCore/QtEndian>

#include <U2Core/U2SafePoints.h>

#include "DatatypeSerializeUtils.h"

namespace U2 {

const QString DNAChromatogramSerializer::ID = "chroma_1.14";

namespace {
    template<class T>
    inline QByteArray packNum(const T &num) {
        T leNum = qToLittleEndian<T>(num);
        return QByteArray((char*)&leNum, sizeof(T));
    }

    template<class T>
    inline QByteArray packNumVector(const QVector<T> &vector) {
        QByteArray result;
        result += packNum<int>(vector.size());
        foreach (const T &num, vector) {
            result += packNum<T>(num);
        }
        return result;
    }

    inline QByteArray packCharVector(const QVector<char> &vector) {
        QByteArray result;
        result += packNum<int>(vector.size());
        foreach (const char &c, vector) {
            result += c;
        }
        return result;
    }

    inline QByteArray packBool(bool value) {
        char c = (value) ? 1 : 0;
        return QByteArray(1, c);
    }

    template<class T>
    inline T unpackNum(const uchar *data, int length, int &offset, U2OpStatus &os) {
        if (offset + int(sizeof(T)) > length) {
            os.setError("The data is too short");
            return T();
        }
        T result = qFromLittleEndian<T>(data + offset);
        offset += sizeof(T);
        return result;
    }

    template<class T>
    inline QVector<T> unpackNumVector(const uchar *data, int length, int &offset, U2OpStatus &os) {
        QVector<T> result;
        int size = unpackNum<int>(data, length, offset, os);
        CHECK_OP(os, result);
        for (int i=0; i<size; i++) {
            T num = unpackNum<T>(data, length, offset, os);
            CHECK_OP(os, result);
            result << num;
        }
        return result;
    }

    inline QVector<char> unpackCharVector(const uchar *data, int length, int &offset, U2OpStatus &os) {
        QVector<char> result;
        int size = unpackNum<int>(data, length, offset, os);
        CHECK_OP(os, result);
        for (int i=0; i<size; i++) {
            if (offset + 1 > length) {
                os.setError("The data is too short");
                return result;
            }
            result << data[offset];
            offset++;
        }
        return result;
    }

    inline bool unpackBool(const uchar *data, int length, int &offset, U2OpStatus &os) {
        if (offset + 1 > length) {
            os.setError("The data is too short");
            return false;
        }
        uchar c = data[offset];
        offset++;
        return (0 == c) ? false : true;
    }
}

QByteArray DNAChromatogramSerializer::serialize(const DNAChromatogram &chroma) {
    QByteArray result;
    result += packNum<int>(chroma.traceLength);
    result += packNum<int>(chroma.seqLength);
    result += packNumVector<ushort>(chroma.baseCalls);
    result += packNumVector<ushort>(chroma.A);
    result += packNumVector<ushort>(chroma.C);
    result += packNumVector<ushort>(chroma.G);
    result += packNumVector<ushort>(chroma.T);
    result += packCharVector(chroma.prob_A);
    result += packCharVector(chroma.prob_C);
    result += packCharVector(chroma.prob_G);
    result += packCharVector(chroma.prob_T);
    result += packBool(chroma.hasQV);
    return result;
}

DNAChromatogram DNAChromatogramSerializer::deserialize(const QByteArray &binary, U2OpStatus &os) {
    DNAChromatogram result;
    const uchar *data = (const uchar*)(binary.data());
    int offset = 0;
    int length = binary.length();

    result.traceLength = unpackNum<int>(data, length, offset, os);
    CHECK_OP(os, result);
    result.seqLength = unpackNum<int>(data, length, offset, os);
    CHECK_OP(os, result);
    result.baseCalls = unpackNumVector<ushort>(data, length, offset, os);
    CHECK_OP(os, result);
    result.A = unpackNumVector<ushort>(data, length, offset, os);
    CHECK_OP(os, result);
    result.C = unpackNumVector<ushort>(data, length, offset, os);
    CHECK_OP(os, result);
    result.G = unpackNumVector<ushort>(data, length, offset, os);
    CHECK_OP(os, result);
    result.T = unpackNumVector<ushort>(data, length, offset, os);
    CHECK_OP(os, result);
    result.prob_A = unpackCharVector(data, length, offset, os);
    CHECK_OP(os, result);
    result.prob_C = unpackCharVector(data, length, offset, os);
    CHECK_OP(os, result);
    result.prob_G = unpackCharVector(data, length, offset, os);
    CHECK_OP(os, result);
    result.prob_T = unpackCharVector(data, length, offset, os);
    CHECK_OP(os, result);
    result.hasQV = unpackBool(data, length, offset, os);
    return result;
}

} // U2
