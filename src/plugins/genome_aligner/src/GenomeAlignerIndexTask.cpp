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

#include "GenomeAlignerIndexTask.h"

#include <U2Core/DNASequenceObject.h>
#include <U2Core/DNASequence.h>
#include <U2Core/AppResources.h>
#include <U2Core/AppContext.h>
#include <U2Core/AppSettings.h>
#include <QtEndian>
#include <QtGlobal>
#include "U2Formats/StreamSequenceReader.h"
#include "GenomeAlignerIndex.h"
#include <U2Algorithm/OpenCLGpuRegistry.h>

namespace U2 {

double GenomeAlignerIndexTask::MEMORY_DIVISION = 2.0/3.0;

GenomeAlignerIndexTask::GenomeAlignerIndexTask(const GenomeAlignerIndexSettings &settings)
: Task("Building genome aligner's index", TaskFlag_None), seq(NULL), sArray(NULL), bitMask(NULL),
buffer(NULL), objLens(NULL),  unknownChar('N')
{
    baseFileName = settings.indexFileName;
    newRefFile = new QFile(baseFileName + QString(".") + GenomeAlignerIndex::REF_INDEX_EXTENSION);
    w = 31; // to avoid +- overflow

    bitTable = bt.getBitMaskCharBits(DNAAlphabet_NUCL);
    bitCharLen = bt.getBitMaskCharBitsNum(DNAAlphabet_NUCL);

    index = new GenomeAlignerIndex();
    index->baseFileName = baseFileName;
    this->settings = settings;

    int maxMemorySize = AppContext::getAppSettings()->getAppResourcePool()->getMaxMemorySizeInMB();
    int reserveSize = 500; //500 Mb for reserve
    //sizeof(quint64) + sizeof(quint32) + sizeof(char) == 13
    PART_SIZE = (maxMemorySize-reserveSize)*1024*1024/(13);
}

GenomeAlignerIndexTask::~GenomeAlignerIndexTask() {
    foreach(QFile *file, tempFiles) {
        file->close();
        QFile::remove(file->fileName());
        delete file;
        file = NULL;
    }
    if (NULL != newRefFile) {
        newRefFile->close();
        delete newRefFile;
    }

    delete[] sArray;
    delete[] bitMask;
    delete[] buffer;
}

void GenomeAlignerIndexTask::run() {
    if (settings.deserializeFromFile) {
        index->deserialize(stateInfo);
        if (isCanceled() || hasErrors()) {
            return;
        }
    } else {
        reformatSequence();
        if (isCanceled() || hasErrors()) {
            return;
        }
        seqLength = objLens[objCount-1];
        indexLength = seqLength - (w - 1)*objCount;
        int parts = indexLength/PART_SIZE + 1;

        bitFilter = ((quint64)1<<(bitCharLen * w))-1;

        index->seqLength = seqLength;
        index->w = w;
        index->indexLength = indexLength;

        for (int i=0; i<parts; i++) {
            QString tmpFileName = baseFileName + QString(".%1.tmp").arg(i);
            QFile *tempFile = new QFile(tmpFileName);
            tempFiles.append(tempFile);
        }

        int partLen = 0;
        if (1 == parts) {
            partLen = indexLength;
        } else {
            partLen = PART_SIZE;
        }
        sArray = new quint32[partLen];
        bitMask = new quint64[partLen];
        buffer = new char[BUFF_SIZE];

        quint32 idx = 0;
        quint32 arrLen = 0;
        int curObj = 0;
        int i = 0;
        if (!newRefFile->open(QIODevice::ReadOnly)) {
            stateInfo.setError("Can't open ref index file");
            return;
        }
        foreach(QFile *file, tempFiles) {
            if (isCanceled() || hasErrors()) {
                return;
            }
            buildPart(&idx, &curObj, &arrLen);
            file->open(QIODevice::WriteOnly);
            writePart(file, arrLen);
            file->close();
            file->open(QIODevice::ReadOnly);

            stateInfo.progress = (0.75*i)/tempFiles.size();
            i++;
        }

        mergeSort();

        index->serialize(GUrl(settings.refFileName).fileName(), stateInfo);
        if (hasErrors()) {
            return;
        }
    }
    if (!index->openIndexFile()) {
        stateInfo.setError("Can't open sArray index file");
        return;
    }
    delete[] sArray;
    delete[] bitMask;
    sArray = NULL;
    bitMask = NULL;

    calculateMemForAligning();

    index->createMemCache();
}

void GenomeAlignerIndexTask::calculateMemForAligning() {
    qint64 memSize = 1024*1024*(AppContext::getAppSettings()->getAppResourcePool()->getMaxMemorySizeInMB() - 200);
    int elemSize = 12; //int + long long
    int pMem = (((qint64)index->indexLength)*elemSize)/(qint64)(MEMORY_DIVISION*memSize) + 1;
    int pGPU = 0;
    assert(pMem > 0);

    if (settings.openCL) {
        OpenCLGpuModel *gpu = AppContext::getOpenCLGpuRegistry()->getAnyEnabledGpu();
        if (NULL == gpu) {
            setError("Can't find enabled GPU");
            return;
        }
        qint64 gpuSize= gpu->getGlobalMemorySizeBytes();
        elemSize = 8; //long long
        pGPU = (index->indexLength*elemSize)/(qint64)(MEMORY_DIVISION*gpuSize) + 1;
        assert(pGPU > 0);

        index->partsInMemCache = qMax(pMem, pGPU);

        gpuFreeSize = gpuSize - elemSize*(index->indexLength/index->partsInMemCache);
    } else {
        index->partsInMemCache = qMax(pMem, pGPU);
    }
    elemSize = 12;
    memFreeSize = memSize - elemSize*(index->indexLength/index->partsInMemCache);
}

void GenomeAlignerIndexTask::buildPart(quint32 *idx, int *curObj, quint32 *arrLen) {
    QByteArray refPart;
    quint32 first = initPart(idx, curObj, arrLen, refPart);
    quint32 *arunner = sArray;
    quint64 *mrunner = bitMask;
    quint64 bitValue = 0;
    quint32 expectedNext = 0;
    quint32 wCharsInMask1 = w - 1;

    for (quint64 *end = mrunner + *arrLen; mrunner < end; arunner++, mrunner++) {
        const char* s = seq + *arunner;
        if (*arunner == expectedNext && expectedNext != 0) { //pop first bit, push wCharsInMask1 char to the mask
            bitValue = ((bitValue << bitCharLen) | bitTable[uchar(*(s + wCharsInMask1))]) & bitFilter;
#ifdef _DEBUG
            // double check that optimization doesn't break anything
            quint64 bitValue2 = index->getBitValue(s, w);
            assert(bitValue == bitValue2);
#endif
        } else {
            //recompute the mask if we have some symbols skipped
            bitValue = index->getBitValue(s, w);
        }
        expectedNext = (s + 1) - seq;
        *mrunner = bitValue;
    }

    if (isCanceled()) {
        seq = NULL;
        return;
    }

    sort(bitMask, 0, *arrLen);
    for (quint32 i=0; i<*arrLen; i++) {
        sArray[i] += first; //+first for correct sArray value
    }
    seq = NULL;
}

quint32 GenomeAlignerIndexTask::initPart(quint32 *idx, int *curObj, quint32 *arrLen, QByteArray &refPart) {
    *arrLen = 0;

    if (*idx > (objLens[*curObj]-w) && *idx < objLens[*curObj]) {
        *idx = objLens[*curObj];
        (*curObj)++;
    }
    //calculate first and last char of sequence for this part
    quint32 f = *idx;
    quint32 l = f;
    quint32 c = 0;

    for (int i=*curObj; i<objCount; i++) {
        c += (objLens[i] - w) - l + 1;
        if (c > PART_SIZE) {
            quint32 d = c - PART_SIZE;
            l = objLens[i] - w - d;
            break;
        }
        if (objCount - 1 == i) { //last iteration
            l = objLens[i] - w;
        } else {
            l = objLens[i];
        }
    }
    //now we khow how many symbols are needed to read:
    quint32 charsToRead = l - f + w;
    newRefFile->seek(f);
    refPart = newRefFile->read(charsToRead); //read a part of the sequence for sorting arrays
    seq = refPart.data();
    quint32 seqPartLen = refPart.length();

    quint32 *arunner = sArray;
    *arrLen = 0;
    bool goodSuff = false;
    quint32 seqIdx = 0;
    quint32 tmpIdx;
    quint32 first = *idx;
    for (quint32 i=0; i<PART_SIZE; i++) { //initializing sArray and arrLen
        if (*idx > (objLens[*curObj]-w) && *idx < objLens[*curObj]) {
            seqIdx += objLens[*curObj] - *idx;
            *idx = objLens[*curObj];
            (*curObj)++;
            goodSuff = false;
        }
        if (*idx >= seqLength || seqIdx >= seqPartLen) {
            break;
        }

        if (goodSuff) {
            if (seq[seqIdx + w-1]==unknownChar) {
                goodSuff = false;
                seqIdx += w;
                *idx += w;
                i--;
                continue;
            }
        }
        if (!goodSuff) {
            int goodChars = 0;
            tmpIdx = *idx;
            quint32 j=seqIdx;
            for (; j<seqPartLen && goodChars<w; j++) {
                if (tmpIdx == objLens[*curObj]) {
                    j += objLens[*curObj] - tmpIdx;
                    tmpIdx = objLens[*curObj];
                    (*curObj)++;
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

            *idx = tmpIdx - goodChars;
            seqIdx = j - w;
            if (goodChars!=w) {
                break;
            }
            goodSuff = true;
        }

        *arunner = *idx - first; //-first for positioning at a part of the sequence
        (*arrLen)++;
        (*idx)++;
        arunner++;
        seqIdx++;
    }

    return first;
}

//Stable sort of sequences
void GenomeAlignerIndexTask::sort(quint64 *x, int off, int len) {
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
    quint64 *v = x + m;

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

qint64 GenomeAlignerIndexTask::compare(const quint64 *x1, const quint64 *x2) const {
    return *x1-*x2;
}

void GenomeAlignerIndexTask::swap(quint64 *x1, quint64 *x2) const {
    assert(x1 - bitMask >= 0 && x1 - bitMask < (qint64)PART_SIZE);
    assert(x2 - bitMask >= 0 && x2 - bitMask < (qint64)PART_SIZE);

    quint32 *a1 = sArray+(x1-bitMask);
    quint32 *a2 = sArray+(x2-bitMask);
    qSwap(*x1, *x2);
    qSwap(*a1, *a2);
}

quint32 GenomeAlignerIndexTask::med3(quint64 *x, quint32 a, quint32 b, quint32 c) {
    qint64 bc = compare(x+b, x+c);
    qint64 ac = compare(x+a, x+c);
    return compare(x+a, x+b) < 0 ?
        (bc < 0 ? b : ac < 0 ? c : a) :
        (bc > 0 ? b : ac > 0 ? c : a);
}

void GenomeAlignerIndexTask::vecswap(quint64 *x1, quint64 *x2, quint32 n) {
    for (quint32 i=0; i<n; i++) {
        swap(x1+i, x2+i);
    }
}

void GenomeAlignerIndexTask::writePart(QFile *file, quint32 arrLen) {
    int bufIdx = 0;
    int size32 = sizeof(quint32);
    int size64 = sizeof(quint64);

    for (quint32 i=0; i<arrLen; i++) {
        if (BUFF_SIZE == bufIdx) {
            file->write(buffer, BUFF_SIZE);
            bufIdx = 0;
        }
        qToBigEndian(sArray[i], (uchar*)(&buffer[bufIdx]));
        bufIdx += size32;
        qToBigEndian(bitMask[i], (uchar*)(&buffer[bufIdx]));
        bufIdx += size64;
    }
    file->write(buffer, bufIdx);
}

void GenomeAlignerIndexTask::mergeSort() {
    QFile indexFile(baseFileName+QString(".")+GenomeAlignerIndex::SARRAY_EXTENSION);
    indexFile.open(QIODevice::WriteOnly);
    QFile **files = tempFiles.data();
    int size = tempFiles.size();
    char **buffers = new char*[size];
    int *idxs = new int[size];
    int *sizes = new int[size];

    for (int i=0; i<size; i++) {
        buffers[i] = new char[BUFF_SIZE];
        idxs[i] = 0;
        sizes[i] = files[i]->read(buffers[i], BUFF_SIZE);
    }

    int idx = 0;
    quint32 a;
    quint64 b;
    int size32 = sizeof(quint32);
    int size64 = sizeof(quint64);
    quint32 aMin = 0;
    quint64 bMin = 0;
    int idxMin = 0;
    bool firstIter = true;
    uchar *tmp = NULL;
    int bufIdx = 0;

    do {
        firstIter = true;
        for (int i=0; i<tempFiles.size(); i++) {
            if (0 == sizes[i]) {
                continue;
            }
            if (idxs[i] == sizes[i]) {
                idxs[i] = 0;
                sizes[i] = files[i]->read(buffers[i], BUFF_SIZE);
                if (0 == sizes[i]) {
                    continue;
                }
            }

            idx = idxs[i];
            tmp = (uchar*)buffers[i];
            a = qFromBigEndian<quint32>(&tmp[idx]);
            idx += size32;
            b = qFromBigEndian<quint64>(&tmp[idx]);

            if (firstIter || compare(&b, &bMin) < 0) {
                aMin = a;
                bMin = b;
                idxMin = i;
                firstIter = false;
            }
        }
        if (!firstIter) {
            idxs[idxMin] += size32 + size64;

            if (BUFF_SIZE == bufIdx) {
                indexFile.write(buffer, BUFF_SIZE);
                bufIdx = 0;
            }
            qToBigEndian(aMin, (uchar*)(&buffer[bufIdx]));
            bufIdx += size32;
            qToBigEndian(bMin, (uchar*)(&buffer[bufIdx]));
            bufIdx += size64;
        }
    } while (!firstIter);

    indexFile.write(buffer, bufIdx);
    indexFile.close();

    for (int i=0; i<size; i++) {
        delete[] buffers[i];
    }
    delete[] buffers;
    delete[] idxs;
    delete[] sizes;
}

void GenomeAlignerIndexTask::reformatSequence() {
    StreamSequenceReader seqReader;
    QList<GUrl> urls;
    urls.append(GUrl(settings.refFileName));
    bool init = seqReader.init(urls);
    if (!init) {
        setError(tr("Can not init short reads loader. %1").arg(seqReader.getErrorMessage()));
        return;
    }

    objCount = 0;
    QList<quint32> seqLens;

    newRefFile->open(QIODevice::WriteOnly);
    bool firstSeq = true;
    while (seqReader.hasNext()) {
        objCount++;
        const DNASequenceObject *obj = seqReader.getNextSequenceObject();
        if (NULL == obj) {
            setError("Reference object type must be a sequence, but not a multiple alignment");
            return;
        }
        if (DNAAlphabet_NUCL != obj->getAlphabet()->getType()) {
            setError("Unsupported file format: alphabet type is not NUCL");
            return;
        }
        const DNASequence &seq = obj->getDNASequence();
        seqLens.append(seq.length());
        newRefFile->write(seq.constData());
        if (firstSeq) {
            index->seqObjName = seq.getName() + QString("_and_others");
            firstSeq = false;
        }
    }
    newRefFile->close();

    if (0 == objCount) {
        setError(QString("Unsupported file format or empty reference in %1").arg(settings.refFileName));
        return;
    }
    index->objLens = new quint32[objCount];
    index->objCount = objCount;
    objLens = index->objLens;
    int i = 0;
    quint32 prev = 0;
    foreach (quint32 len, seqLens) {
        objLens[i] = prev + len;
        prev = objLens[i];
        i++;
    }
}

} //U2
