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

#include "GenomeAlignerIndex.h"

#include <QtCore/QFile>
#include <U2Algorithm/BinaryFindOpenCL.h>
#include <QtEndian>
#include "GenomeAlignerFindTask.h"
#include "GenomeAlignerTask.h"
#include <U2Core/Timer.h>
#include <U2Core/Counter.h>

namespace U2 {

const QString GenomeAlignerIndex::HEADER_EXTENSION("idx");
const QString GenomeAlignerIndex::SARRAY_EXTENSION("sarr");
const QString GenomeAlignerIndex::REF_INDEX_EXTENSION("ref");
const QString GenomeAlignerIndex::HEADER("#UGENE suffix array index\n");
const QString GenomeAlignerIndex::PARAMETERS("#file \"%1\", sequence's length = %2, w = %3\n");
const int GenomeAlignerIndex::charsInMask = 31;
const QString COMMENT1("#sequence length, window size, index size, bitChar's length, count of sequences\n");
const QString COMMENT2("#reference name\n");
const QString COMMENT3("#offsets of sequences in the reference\n");

GenomeAlignerIndex::GenomeAlignerIndex()
: seq(NULL), sArray(NULL), bitMask(NULL), memIdx(NULL),
memBM(NULL), indexFile(NULL), refFile(NULL), objLens(NULL)
{
    bitTable = bt.getBitMaskCharBits(DNAAlphabet_NUCL);
    bitCharLen = bt.getBitMaskCharBitsNum(DNAAlphabet_NUCL);
    seqLength = 0;
    w = 0;
    indexLength = 0;
    loadedPartSize = 0;
    partsInMemCache = 0;
    objCount = 0;
    currentPart = -1;
}

GenomeAlignerIndex::~GenomeAlignerIndex() {
    if (NULL != indexFile) {
        indexFile->close();
        delete indexFile;
        indexFile = NULL;
    }
    if (NULL != refFile) {
        refFile->close();
        delete refFile;
        refFile = NULL;
    }
    delete[] sArray;
    delete[] bitMask;
    delete[] memIdx;
    delete[] memBM;
    delete[] seq;
    delete[] objLens;
}

void GenomeAlignerIndex::serialize(const QString &refFileName, TaskStateInfo &ti) {
    QString indexFileName = baseFileName + QString(".") + HEADER_EXTENSION;
    QFile file(indexFileName);
    if (!file.open(QIODevice::WriteOnly)) {
        ti.setError(QString("Can't open %1 for writing").arg(indexFileName));
        return;
    }

    QByteArray data;
    data = HEADER.toAscii();
    data += PARAMETERS.arg(refFileName).arg(seqLength).arg(w).toAscii();
    data += COMMENT1 + COMMENT2 + COMMENT3;
    data += QByteArray::number(seqLength, 10) + ", ";
    data += QByteArray::number(w, 10) + ", ";
    data += QByteArray::number(indexLength, 10) + ", ";
    data += QByteArray::number(bitCharLen, 10) + ", ";
    data += QByteArray::number(objCount, 10) + "\n";
    data += seqObjName + "\n";
    for (int i=0; i<objCount; i++) {
        data += QByteArray::number(objLens[i], 10);
        if (objCount-1 == i) {
            data += "\n";
        } else {
            data += ", ";
        }
    }
    file.write(data);

    file.close();
}

inline int getNextInt(QByteArray &data, bool &eol, bool &intErr) {
    int commaIdx = data.indexOf(',');
    if (-1 == commaIdx) {
        commaIdx = data.length();
        eol = true;
    }

    QByteArray result = data.left(commaIdx).trimmed();
    data = data.mid(commaIdx+1).trimmed();

    char c = 0;
    for (int i = 0; i < result.length(); i++) {
        c = result[i];
        if (c <'0' || c>'9') {
            intErr = true;
            return -1;
        }
    }
    return result.toInt();
}

void GenomeAlignerIndex::deserialize(TaskStateInfo &ti) {
    QString indexFileName = baseFileName + QString(".") + HEADER_EXTENSION;
    QFile file(indexFileName);
    if (!file.open(QIODevice::ReadOnly)) {
        ti.setError("Can't open file-index");
        return;
    }
    QByteArray data;
    bool eol = false;
    bool intErr = false;

    do {
        data = file.readLine().trimmed();
    } while (data.length() > 0 && '#' == data[0]);

    if (0 == data.length()) {
        ti.setError("Empty parameters' line in the file-index");
        return;
    }
    seqLength = getNextInt(data, eol, intErr);
    w = getNextInt(data, eol, intErr);
    indexLength = getNextInt(data, eol, intErr);
    int bitCharLen = getNextInt(data, eol, intErr);
    if (eol) {
        ti.setError("Too little amount of parameters in the file-index");
        return;
    }
    objCount = getNextInt(data, eol, intErr);
    if (intErr) {
        ti.setError("Bad integer for some parameter in the file-index");
        return;
    }
    if (objCount <= 0) {
        ti.setError("Count of sequences must be >0");
        return;
    }

    seqObjName = file.readLine().trimmed();
    data = file.readLine().trimmed();
    objLens = new quint32[objCount];
    eol = false;
    intErr = false;
    for (int i=0; i<objCount; i++) {
        objLens[i] = getNextInt(data, eol, intErr);
        if (i<objCount-1) {
            if (eol) {
                ti.setError("Too little amount of parameters in the file-index");
                return;
            }
        }
    }
    if (intErr) {
        ti.setError("Bad integer for some parameter in the file-index");
        return;
    }

    if (w <= 0 || indexLength <= 0) {
        ti.setError("Negative index's parameters");
        return;
    }

    if (this->bitCharLen != bitCharLen) {
        ti.setError("This index was built for a sequence with another sequence type");
        return;
    } else {
        this->bitCharLen = bitCharLen;
    }
}

bool GenomeAlignerIndex::openIndexFile() {
    indexFile = new QFile(baseFileName+QString(".")+GenomeAlignerIndex::SARRAY_EXTENSION);

    return indexFile->open(QIODevice::ReadOnly);
}

QFile *GenomeAlignerIndex::openRefFile() {
    QFile *refFile = new QFile(baseFileName+QString(".")+GenomeAlignerIndex::REF_INDEX_EXTENSION);

    if (refFile->open(QIODevice::ReadOnly)) {
        return refFile;
    } else {
        delete refFile;
        return NULL;
    }
}


void GenomeAlignerIndex::createMemCache() {
    memIdx = new quint32[2*partsInMemCache];
    memBM = new quint64[2*partsInMemCache];
    int elemSize = sizeof(quint32) + sizeof(quint64);
    quint32 partSize =  indexLength/partsInMemCache;
    QByteArray b(2*elemSize, '\0');
    char *buff = b.data();

    assert(NULL != indexFile && indexFile->isOpen());
    indexFile->seek(0);

    indexFile->read(buff, elemSize);
    quint32 idx = 0;
    memIdx[0] = idx;
    memBM[0] = qFromBigEndian<quint64>((uchar*)(buff+sizeof(quint32)));
    bool foundBorder = false;
    int maxPartSize = 0;
    int curPartSize = 0;
    quint64 bitFilter = ((quint64)0 - 1)<<(62 - GenomeAlignerTask::MIN_BIT_MASK_LENGTH);
    for (int i=0; i<partsInMemCache; i++) {
        foundBorder = false;
        if (partsInMemCache-1 == i) {
            curPartSize = (indexLength-1) - idx + 1;
            idx = indexLength - 1;
        } else {
            curPartSize = partSize;
            idx += partSize-1;
        }
        indexFile->seek(idx*elemSize);
        indexFile->read(buff, elemSize);
        memIdx[2*i+1] = idx;
        memBM[2*i+1] = qFromBigEndian<quint64>((uchar*)(buff + sizeof(quint32)));
        idx++;
        if (partsInMemCache-1 == i) {
            if (curPartSize > maxPartSize) {
                maxPartSize = curPartSize;
            }
            continue;
        }
        while (!foundBorder && idx < indexLength) {
            indexFile->read(buff, elemSize);
            memIdx[2*i+2] = idx;
            memBM[2*i+2] = qFromBigEndian<quint64>((uchar*)(buff + sizeof(quint32)));
            if ((memBM[2*i+1]&bitFilter) != (memBM[2*i+2]&bitFilter)) {
                foundBorder = true;
            } else {
                curPartSize++;
            }
        }
        if (idx == indexLength) {
            partsInMemCache = i+1;
        }
        if (curPartSize > maxPartSize) {
            maxPartSize = curPartSize;
        }
    }

    sArray = new quint32[maxPartSize];
    bitMask = new quint64[maxPartSize];
}

quint64 GenomeAlignerIndex::getBitValue(const char *seq, int length) const {
    quint64 bitValue = 0;
    for (int i = 0; i < length; i++) {
        bitValue = (bitValue << bitCharLen) | bitTable[uchar(*(seq+i))];
    }

    bitValue <<= 2*charsInMask - bitCharLen*length;

    return bitValue;
}

int GenomeAlignerIndex::getPrefixSize() const {
    return w;
}

int GenomeAlignerIndex::findInCache(quint64 bitValue, quint64 bitFilter) const {
    int low = 0;
    int high = 2*partsInMemCache-1;
    quint64 *a = memBM;
    int preAnswer = -1;
    while (low <= high) {
        int mid = (low + high) >> 1;
        qint64 rc = (a[mid] & bitFilter) - (bitValue&bitFilter);
        if (rc < 0) {
            if (1 < high - mid) {
                low = mid + 1;
            } else {
                if (1 == high%2 && (a[high]&bitFilter) >= (bitValue&bitFilter)) {
                    preAnswer = high >> 1;
                } else if (0 == high%2 && (a[high]&bitFilter) <= (bitValue&bitFilter)) {
                    preAnswer = high >> 1;
                }
                break;
            }
        } else if (rc > 0) {
            if (1 < mid - low) {
                high = mid - 1;
            } else {
                if (0 == low%2 && (a[low]&bitFilter) <= (bitValue&bitFilter)) {
                    preAnswer = low >> 1;
                } else if (1 == low%2 && (a[low]&bitFilter) >= (bitValue&bitFilter)) {
                    preAnswer = low >> 1;
                }
                break;
            }
        } else {
            preAnswer = mid >> 1;
            break;
        }
    }

    return preAnswer;
}

void GenomeAlignerIndex::loadPart(int part) {
    assert(part < partsInMemCache);

    quint32 startIdx = memIdx[2*part];
    loadedPartSize = memIdx[2*part+1] - startIdx + 1;
    int elemSize = sizeof(quint32) + sizeof(quint64);

    int BUFF_SIZE = 1024*1024*elemSize;
    QByteArray b(BUFF_SIZE, '\0');
    char *buff = b.data();
    indexFile->seek(startIdx*elemSize);
    int size = indexFile->read(buff, BUFF_SIZE);
    int idx = 0;
    assert(size>0);

    for (quint32 i=0; i<loadedPartSize; i++, idx+=elemSize) {
        if (idx >= size) {
            size = indexFile->read(buff, BUFF_SIZE);
            idx = 0;
            assert(size>0);
        }
        sArray[i] = qFromBigEndian<quint32>((uchar*)(buff+idx));
        bitMask[i] = qFromBigEndian<quint64>((uchar*)(buff+idx + sizeof(quint32)));
    }
    currentPart = part;
}

int GenomeAlignerIndex::findBit(quint64 bitValue, quint64 bitFilter) {
    int low = 0;
    int high = loadedPartSize - 1;
    quint64 *a = bitMask;
    qint64 rc = 0;
    while (low <= high) {
        int mid = (low + high) / 2;
        rc = (a[mid]&bitFilter) - (bitValue&bitFilter);
        if (rc < 0) {
            low = mid + 1;
        } else if (rc > 0) {
            high = mid - 1;
        } else {
            for(high=mid-1;high >= 0 && (a[high]&bitFilter)==(bitValue&bitFilter); high--){};
            high++;
            break;
        }
    }
    if (0==rc) {
        return high;
    }
    return -1;
}

ResType *GenomeAlignerIndex::findBitOpenCL(quint64 *bitValues, int size, quint64 bitFilter) {
    taskLog.details(QString("Binary search on GPU of %1 Mb search-values in %2 Mb base values")
        .arg((8*size)/(1024*1024)).arg((8*loadedPartSize)/(1024*1024)));
    BinaryFindOpenCL bf((NumberType*)bitMask, loadedPartSize, (NumberType*)bitValues, size, bitFilter);

    NumberType *ans = bf.launch();

    return (ResType*)ans;
}

void GenomeAlignerIndex::findInPart(QFile *refFile, const QByteArray &querySeq, int startPos, ResType firstResult,
                                    quint64 bitValue, QList<quint32> &results, SearchContext *settings) {
    assert(NULL != refFile && refFile->isOpen());
    if (firstResult < 0) {
        return;
    }

    quint32 offset = 0;
    quint32 minBorder = 0;
    quint32 fisrtSymbol = 0;
    quint64 tmpBM = 0;
    quint64 tmpBV = 0;
    //quint64 m = 0;
    //quint64 v = 0;
    quint64 xorBM = 0;
    int j = 0;
    int CMAX = settings->nMismatches;
    int restBits = qMax(0, 2*(charsInMask - settings->w));
    //int matchLen = restBits/(CMAX + 1);
    if (!settings->absMismatches) {
        CMAX = (querySeq.length() * settings->ptMismatches) / MAX_PERCENTAGE;
    }
    for (int k=firstResult; (bitValue&settings->bitFilter)==(bitMask[k]&settings->bitFilter); k++) {
        offset = sArray[k];
        assert(offset>=0 && offset<objLens[objCount-1]);
        fisrtSymbol = offset-startPos;
        if (results.contains(fisrtSymbol)) {
            continue;
        }
        for (j=0; j<objCount; j++) {
            if (offset < objLens[j]) {
                break;
            }
        }
        minBorder = j>0?objLens[j-1]:0;
        if (fisrtSymbol < minBorder) {
            continue;
        }
        if (offset + (querySeq.length() - startPos - 1) >= objLens[j]) {
            continue;
        }

        int c = 0;
        //full bitMask optimization
        int bits = qMin(2*(querySeq.length() - startPos - settings->w), restBits);
        if (CMAX > 0) {
            /*if (bits >= matchLen) {
                tmpBM = bitMask[k]<<(2+2*settings->w);
                tmpBV = bitValue<<(2+2*settings->w);
                bool match = false;

                for (int i=0; i+matchLen<=bits; i+=matchLen) {
                    m = tmpBM >> (64-matchLen);
                    v = tmpBV >> (64-matchLen);
                    if (m == v) {
                        match = true;
                        break;
                    }
                    tmpBM <<= matchLen;
                    tmpBV <<= matchLen;
                }
                if (!match) {
                    continue;
                }
                bits = 0;
            } else {
                bits = 0;
            }*/
            tmpBM = bitMask[k]<<(2+2*settings->w);
            tmpBV = bitValue<<(2+2*settings->w);
            xorBM = (tmpBV ^ tmpBM);
            xorBM >>= 2*settings->w + 2 + (restBits - bits);

            for (int i=0; i<bits && c<=CMAX; i+=2) {
                if (1 == (xorBM&1)) {
                    xorBM >>=1;
                    c++;
                } else {
                    xorBM >>=1;
                    if (1 == (xorBM&1)) {
                        c++;
                    }
                }
                xorBM >>= 1;
            }
            if (c>CMAX) {
                continue;
            }
        } else {
            bits = 0;
        }

        //unchecked optimization of disk reads count
        /*GTIMER(c3, t3, "optimization");
        quint64 window = getBitValue(querySeq.constData()+startPos+settings->w, 30);
        int res = findBit(window, settings->bitFilter);
        bool found = false;
        for (int j=res; (window&settings->bitFilter)==(bitMask[j]&settings->bitFilter); j++) {
            if (sArray[j] == offset+settings->w) {
                found = true;
                break;
            }
        }
        if (!found) {
            continue;
        }
        t3.stop();*/


        refFile->seek(fisrtSymbol);
        QByteArray buf(querySeq.length(), '\0');
        char *refBuff = buf.data();
        refFile->read(refBuff, querySeq.length());
        // forward collect
        for (int i=startPos+settings->w + bits/2; i<querySeq.length() && c <= CMAX; i++) {
            c += (querySeq.at(i) == refBuff[i])?0:1;
        }
        if (c>CMAX) {
            continue;
        }
        // backward collect
        for (int i=startPos-1; i>=0 && c <= CMAX; i--) {
            c += (querySeq.at(i) == refBuff[i])?0:1;
        }
        if ( (c <= CMAX) && (!results.contains(fisrtSymbol))) {
            results.append(fisrtSymbol);
            if (settings->bestMode) {
                break;
            }
        }
    }
}

} //U2
