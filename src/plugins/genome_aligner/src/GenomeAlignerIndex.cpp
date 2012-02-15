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

#include <U2Core/Timer.h>
#include <U2Core/Counter.h>
#include <U2Algorithm/BinaryFindOpenCL.h>
#include <QtCore/QFile>
#include <QtEndian>
#include "GenomeAlignerFindTask.h"
#include "GenomeAlignerTask.h"
#include "GenomeAlignerIndex.h"
#include "SuffixSearchCUDA.h"

namespace U2 {

const QString GenomeAlignerIndex::HEADER_EXTENSION("idx");
const QString GenomeAlignerIndex::SARRAY_EXTENSION("sarr");
const QString GenomeAlignerIndex::REF_INDEX_EXTENSION("ref");
const QString GenomeAlignerIndex::HEADER("#UGENE suffix array index\n");
const QString GenomeAlignerIndex::PARAMETERS("#file \"%1\", sequence's length = %2, w = %3\n");
const int GenomeAlignerIndex::charsInMask = MAX_BIT_MASK_LENGTH;
const int GenomeAlignerIndex::overlapSize = 10000;
const QString COMMENT1("#sequence length, window size, bitChar's length, seqPartSize, count of sequences\n");
const QString COMMENT2("#reference name\n");
const QString COMMENT3("#offsets of sequences in the reference\n");
const QString COMMENT4("#seqStart, seqLength, saStart, saLength\n");

GenomeAlignerIndex::GenomeAlignerIndex()
: memIdx(NULL), memBM(NULL), objLens(NULL)
{
    bitTable = bt.getBitMaskCharBits(DNAAlphabet_NUCL);
    bitCharLen = bt.getBitMaskCharBitsNum(DNAAlphabet_NUCL);
    seqLength = 0;
    w = 0;
    partsInMemCache = 0;
    objCount = 0;
    currentPart = -1;
    build = true;
    seqPartSize = -1;
}

GenomeAlignerIndex::~GenomeAlignerIndex() {
    delete[] memIdx;
    delete[] memBM;
    delete[] objLens;
}

void GenomeAlignerIndex::serialize(const QString &refFileName) {
    QString indexFileName = baseFileName + QString(".") + HEADER_EXTENSION;
    QFile file(indexFileName);
    if (!file.open(QIODevice::WriteOnly)) {
        return;
    }

    QByteArray data;
    data = HEADER.toAscii();
    data += PARAMETERS.arg(refFileName).arg(seqLength).arg(w).toAscii();
    data += COMMENT1 + COMMENT2 + COMMENT3 + COMMENT4;
    data += QByteArray::number(seqLength, 10) + ", ";
    data += QByteArray::number(w, 10) + ", ";
    data += QByteArray::number(bitCharLen, 10) + ", ";
    data += QByteArray::number(seqPartSize, 10) + ", ";
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

inline quint32 getNextInt(QByteArray &data, bool &eol, bool &intErr) {
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
    return result.toUInt();
}

bool GenomeAlignerIndex::deserialize(QByteArray &error) {
    QString indexFileName = baseFileName + QString(".") + HEADER_EXTENSION;
    QFile file(indexFileName);
    if (!file.open(QIODevice::ReadOnly)) {
        error = "Can't open file-index.";
        return false;
    }
    QByteArray data;
    bool eol = false;
    bool intErr = false;

    do {
        data = file.readLine().trimmed();
    } while (data.length() > 0 && '#' == data[0]);

    if (0 == data.length()) {
        error = "Empty parameters' line in the file-index.";
        file.close();
        return false;
    }
    seqLength = getNextInt(data, eol, intErr);
    w = getNextInt(data, eol, intErr);
    int bitCharLen = getNextInt(data, eol, intErr);
    seqPartSize = getNextInt(data, eol, intErr);
    if (eol) {
        error = "Too little amount of parameters in the file-index.";
        file.close();
        return false;
    }
    objCount = getNextInt(data, eol, intErr);
    if (intErr) {
        error = "Bad integer for some parameter in the file-index.";
        file.close();
        return false;
    }
    if (objCount <= 0) {
        error = "Count of sequences must be >0.";
        file.close();
        return false;
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
                error = "Too little amount of parameters in the file-index.";
                file.close();
                return false;
            }
        }
    }
    if (intErr) {
        error = "Bad integer for some parameter in the file-index.";
        file.close();
        return false;
    }

    if (w <= 0) {
        error = "Negative index's parameters.";
        file.close();
        return false;
    }

    if (this->bitCharLen != bitCharLen) {
        error = "This index was built for a sequence with another sequence type.";
        file.close();
        return false;
    } else {
        this->bitCharLen = bitCharLen;
    }

    file.close();
    return true;
}

bool GenomeAlignerIndex::openIndexFiles() {
    indexPart.refFile = new QFile(baseFileName+QString(".")+GenomeAlignerIndex::REF_INDEX_EXTENSION);
    return indexPart.refFile->open(QIODevice::ReadOnly);
}

BMType GenomeAlignerIndex::getBitValue(const char *seq, int length) const {
    BMType bitValue = 0;
    for (int i = 0; i < length; i++) {
        bitValue = (bitValue << bitCharLen) | bitTable[uchar(*(seq+i))];
    }

    bitValue <<= 2*charsInMask - bitCharLen*length;

    return bitValue;
}

bool GenomeAlignerIndex::loadPart(int part) {
    currentPart = part;
    if (build) {
        GTIMER(c, v, "GenomeAlignerIndex::build");
        SAType arrLen = 0;
        sArray = indexPart.sArray;
        bitMask = indexPart.bitMask;
        buildPart(indexPart.seqStarts[part], indexPart.seqLengths[part], arrLen);
        indexPart.saLengths[part] = arrLen;
        indexPart.currentPart = part;
        sort(bitMask, 0, arrLen);
        GTIMER(c2, v2, "GenomeAlignerIndex::writePart");
        indexPart.writePart(part, arrLen);
        v2.stop();
        sArray = NULL;
        bitMask = NULL;

        if (part == indexPart.partCount - 1) {
            build = false;
            serialize(baseFileName + "." + REF_INDEX_EXTENSION);
        }
        return true;
    } else {
        GTIMER(c, v, "GenomeAlignerIndex::load");
        return indexPart.load(part);
    }
}

BinarySearchResult GenomeAlignerIndex::bitMaskBinarySearch(BMType bitValue, BMType bitFilter) {
    int low = 0;
    int high = indexPart.getLoadedPartSize() - 1;
    BMType *a = indexPart.bitMask;
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

BinarySearchResult *GenomeAlignerIndex::bitMaskBinarySearchOpenCL(const BMType *bitValues, int size, BMType bitFilter) {
    taskLog.details(QString("Binary search on GPU of %1 Mb search-values in %2 Mb base values")
        .arg((8*size)/(1024*1024)).arg((8*indexPart.getLoadedPartSize())/(1024*1024)));
    BinaryFindOpenCL bf((NumberType*)indexPart.bitMask, indexPart.getLoadedPartSize(), (NumberType*)bitValues, size, bitFilter);

    NumberType *ans = bf.launch();

    return (BinarySearchResult*)ans;
}

BinarySearchResult * GenomeAlignerIndex::findBitValuesUsingCUDA( BMType *bitValues, int size, BMType bitFilter )
{
    Q_UNUSED(bitValues);Q_UNUSED(size);Q_UNUSED(bitFilter);
    BinarySearchResult* result = NULL;
#ifdef GA_BUILD_WITH_CUDA 
    taskLog.details(QString("Binary search using CUDA on GPU of %1 Mb search-values in %2 Mb base values")
        .arg((8*size)/(1024*1024)).arg((8*indexPart.getLoadedPartSize())/(1024*1024)));

    // estimate memory size?
    SuffixSearchCUDA ss;
    
    result = (BinarySearchResult*)ss.runSearch( indexPart.bitMask, 
        indexPart.getLoadedPartSize(), bitValues, size, bitFilter);
         
#endif // GA_BUILD_WITH_CUDA
    
    return result;

}

bool GenomeAlignerIndex::isValidPos(SAType offset, int startPos, int length, SAType &fisrtSymbol, SearchQuery *qu, SAType &loadedSeqStart) {
    assert(offset<objLens[objCount-1]);
    if ((qint64)offset - loadedSeqStart < startPos) {
        return false;
    }
    fisrtSymbol = offset-startPos;
    if (qu->contains(fisrtSymbol)) {
        return false;
    }

    //binary search in reference objects
    int low = 0;
    int high = objCount;
    int mid = 0;
    qint64 rc = 0;
    SAType minBorder = 0;
    while (low <= high) {
        mid = (low + high) / 2;
        rc = objLens[mid] - (qint64)offset;
        minBorder = mid>0?objLens[mid-1]:0;
        if (((qint64)offset >= minBorder) && (rc > 0)) {
            break;
        } else if (rc <= 0) {
            low = mid;
        } else {
            high = mid;
        }
    }

    if (fisrtSymbol < minBorder) {
        return false;
    }
    if (offset + (length - startPos - 1) >= objLens[mid]) {
        return false;
    }

    return true;
}

bool GenomeAlignerIndex::compare(const char *sourceSeq, const char *querySeq, int startPos, int w, int &c, int CMAX, int length) {
    // forward collect
    for (int i=startPos+w; i<length && c <= CMAX; i++) {
        c += (querySeq[i] == sourceSeq[i])?0:1;
    }
    if (c>CMAX) {
        return false;
    }
    // backward collect
    for (int i=startPos-1; i>=0 && c <= CMAX; i--) {
        c += (querySeq[i] == sourceSeq[i])?0:1;
    }
    if (c <= CMAX) {
        return true;
    }
    return false;
}

//this method contains big copy-paste but it works very fast because of it.
void GenomeAlignerIndex::alignShortRead(SearchQuery *qu, BMType bitValue, int startPos, BinarySearchResult firstResult, AlignContext *settings, BMType bitFilter, int w)
{
    if (firstResult < 0) {
        return;
    }

    SAType fisrtSymbol = 0;
    //SAType offset = 0;
    const QByteArray &querySeq = qu->constSequence();
    char *refBuff = NULL;

    int CMAX = settings->nMismatches;
    if (!settings->absMismatches) {
        CMAX = (querySeq.length() * settings->ptMismatches) / MAX_PERCENTAGE;
    }

    int bestC = CMAX + 1;
    SAType bestResult = 0;
    bool found = false;
    if (settings->bestMode) {
        if (qu->haveResult()) {
            bestC = qu->firstMCount();
        }
        if (NULL != qu->getRevCompl() && qu->getRevCompl()->haveResult()) {
            int cRev = qu->getRevCompl()->firstMCount();
            if (cRev < bestC) {
                bestC = cRev;
            }
        }
    }
    CMAX = bestC - 1;
    SAType loadedPartSize = indexPart.getLoadedPartSize();
    SAType loadedSeqStart = indexPart.getLoadedSeqStart();
    quint64 rightOverlapStart = (quint64)loadedSeqStart + (quint64)indexPart.seqLengths[indexPart.currentPart];
    if (indexPart.currentPart != indexPart.partCount - 1) {
        rightOverlapStart -= 2*overlapSize;
    }
    for (SAType k=firstResult; (k<loadedPartSize) && (bitValue&bitFilter)==(indexPart.bitMask[k]&bitFilter); k++) {
        if (!isValidPos(indexPart.sArray[k] + loadedSeqStart, startPos, querySeq.length(),
            fisrtSymbol, qu, loadedSeqStart)) {
            continue;
        }

        refBuff = &(indexPart.seq[fisrtSymbol - loadedSeqStart]);

        int c = 0;
        if (compare(refBuff, querySeq.constData(), startPos, w, c, CMAX, querySeq.length())) {
            if (settings->bestMode) {
                found = true;
                bestC = c;
                bestResult = fisrtSymbol;
                CMAX = bestC - 1;

                if (0 == c) {
                    break;
                } else {
                    continue;
                }
            }
            if (fisrtSymbol >= rightOverlapStart) {
                qu->addOveplapResult(fisrtSymbol);
            } else {
                qu->addResult(fisrtSymbol, c);
            }
        }
    }
    if (settings->bestMode && found) {
        qu->clear();
        qu->addResult(bestResult, bestC);
    }
}

/*build index*/
void GenomeAlignerIndex::buildPart(SAType start, SAType length, SAType &arrLen) {
    initSArray(start, length, arrLen);
    const char *seq = indexPart.seq;
    SAType *arunner = sArray;
    BMType *mrunner = bitMask;
    BMType bitValue = 0;
    SAType expectedNext = 0;
    quint32 wCharsInMask1 = w - 1;

    for (BMType *end = mrunner + arrLen; mrunner < end; arunner++, mrunner++) {
        const char* s = seq + *arunner;
        if (*arunner == expectedNext && expectedNext != 0) { //pop first bit, push wCharsInMask1 char to the mask
            bitValue = ((bitValue << bitCharLen) | bitTable[uchar(*(s + wCharsInMask1))]) & bitFilter;
#ifdef _DEBUG
            // double check that optimization doesn't break anything
            BMType bitValue2 = getBitValue(s, w);
            assert(bitValue == bitValue2);
#endif
        } else {
            //recompute the mask if we have some symbols skipped
            bitValue = getBitValue(s, w);
        }
        expectedNext = (s + 1) - seq;
        *mrunner = bitValue;
    }
}

void GenomeAlignerIndex::initSArray(SAType start, SAType length, SAType &arrLen) {
    indexPart.refFile->seek(start);
    qint64 l = indexPart.refFile->read(indexPart.seq, length);
    if (length != l) {
        //setError("Index .ref file is corrupted.");
        return;
    }
    const char *seq = indexPart.seq;

    SAType *arunner = sArray;
    SAType idx = start;
    SAType seqIdx = 0;
    SAType tmpIdx = 0;

    int curObj = 0;
    for (; curObj<objCount; curObj++) {
        if (idx < objLens[curObj]) {
            break;
        }
    }
    assert(curObj < objCount);

    bool goodSuff = false;
    arrLen = 0;
    SAType last = start + length - w;
    for (quint32 i=0; idx <= last; i++) { //initializing sArray and arrLen
        if (idx > (objLens[curObj]-w) && idx < objLens[curObj]) {
            seqIdx += objLens[curObj] - idx;
            idx = objLens[curObj];
            curObj++;
            goodSuff = false;
        }
        if (idx >= seqLength || seqIdx >= length) {
            break;
        }

        if (goodSuff) {
            if (seq[seqIdx + w-1]==unknownChar) {
                goodSuff = false;
                seqIdx += w;
                idx += w;
                i--;
                continue;
            }
        }
        if (!goodSuff) {
            int goodChars = 0;
            tmpIdx = idx;
            SAType j = seqIdx;
            for (; j<length && goodChars<w; j++) {
                if (tmpIdx == objLens[curObj]) {
                    j += objLens[curObj] - tmpIdx;
                    tmpIdx = objLens[curObj];
                    curObj++;
                    goodChars = 0;
                    j--;
                    continue;
                }
                if (seq[j]!=unknownChar) {
                    goodChars++;
                } else {
                    goodChars = 0;
                }
                tmpIdx++;
            }

            idx = tmpIdx - goodChars;
            seqIdx = j - w;
            if (goodChars != w) {
                break;
            }
            goodSuff = true;
        }

        *arunner = idx - start; //-start for positioning at a part of the sequence
        arrLen++;
        idx++;
        arunner++;
        seqIdx++;
    }
}

//Stable sort of sequences
void GenomeAlignerIndex::sort(BMType *x, int off, int len) {
    // Insertion sort on smallest arrays
    if (len < 7) {
        for (int i=off; i<len+off; i++){
            for (int j=i; j > off && compare(x+j-1,x+j)>0; j--) {
                swap(x+j, x+j-1);
            }
        }
        return;
    }

    // Choose a partition element, v
    quint32 m = off + len / 2;       // Small arrays, middle element
    if (len > 7) {
        quint32 l = off;
        quint32 n = off + len - 1;
        if (len > 40) {        // Big arrays, pseudo median of 9
            quint32 s = len / 8;
            l = med3(x, l,     l+s, l+2*s);
            m = med3(x, m-s,   m,   m+s);
            n = med3(x, n-2*s, n-s, n);
        }
        m = med3(x, l, m, n); // Mid-size, med of 3
    }
    BMType *v = x + m;

    // Establish Invariant: v* (<v)* (>v)* v*
    int a = off, b = a, c = off + len - 1, d = c;
    while(true) {
        qint64 cr;
        while (b <= c && (cr = compare(v, x+b)) >=0 ) {
            if (cr == 0) {
                (x+b==v) && (v=x+a);//save middle pos value
                swap(x+a++,x+b);
            }
            b++;
        }
        while (c >= b && (cr = compare(x+c, v)) >=0 ) {
            if (cr == 0) {
                (x+c==v) && (v=x+d);//save middle pos value
                swap(x+c, x+d--);
            }
            c--;
        }
        if (b > c) {
            break;
        }
        swap(x+b++, x+c--);
    }

    // Swap partition elements back to middle
    int s, n = off + len;
    s = qMin(a-off, b-a  ); vecswap(x+off, x+b-s, s);
    s = qMin(d-c,   n-d-1); vecswap(x+b,   x+n-s, s);

    // Recursively sort non-partition-elements
    if ((s = b-a) > 1) {
        sort(x, off, s);
    }
    if ((s = d-c) > 1) {
        sort(x, n-s, s);
    }
}

qint64 GenomeAlignerIndex::compare(const BMType *x1, const BMType *x2) const {
    return *x1-*x2;
}

void GenomeAlignerIndex::swap(BMType *x1, BMType *x2) const {
    assert(x1 - bitMask >= 0 && x1 - bitMask < (qint64)indexPart.saLengths[indexPart.currentPart]);
    assert(x2 - bitMask >= 0 && x2 - bitMask < (qint64)indexPart.saLengths[indexPart.currentPart]);

    SAType *a1 = sArray+(x1-bitMask);
    SAType *a2 = sArray+(x2-bitMask);
    qSwap(*x1, *x2);
    qSwap(*a1, *a2);
}

quint32 GenomeAlignerIndex::med3(BMType *x, quint32 a, quint32 b, quint32 c) {
    qint64 bc = compare(x+b, x+c);
    qint64 ac = compare(x+a, x+c);
    return compare(x+a, x+b) < 0 ?
        (bc < 0 ? b : ac < 0 ? c : a) :
        (bc > 0 ? b : ac > 0 ? c : a);
}

void GenomeAlignerIndex::vecswap(BMType *x1, BMType *x2, quint32 n) {
    for (quint32 i=0; i<n; i++) {
        swap(x1+i, x2+i);
    }
}


} //U2
