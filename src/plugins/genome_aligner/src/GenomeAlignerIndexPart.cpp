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

#include <U2Core/Timer.h>
#include <U2Algorithm/BitsTable.h>
#include <QtEndian>

#include "GenomeAlignerIndexPart.h"

namespace U2 {

IndexPart::IndexPart() {
    sArray = NULL;
    bitMask = NULL;
    seq = NULL;
    partCount = 0;
    currentPart = -1;
    seqStarts = NULL;
    seqLengths = NULL;
    saLengths = NULL;

    refFile = NULL;
    partFiles = NULL;
}

IndexPart::~IndexPart() {
    delete[] sArray;
    delete[] bitMask;
    delete[] seq;
    delete[] seqStarts;
    delete[] seqLengths;
    delete[] saLengths;

    if (NULL != refFile) {
        refFile->close();
    }
    delete refFile;

    for (int i=0; i<partCount; i++) {
        if (NULL != partFiles[i]) {
            partFiles[i]->close();
        }
        delete partFiles[i];
    }
    delete[] partFiles;
}

SAType IndexPart::getLoadedPartSize() {
    assert(currentPart>=0);
    return saLengths[currentPart];
}

bool isLittleEndian() {
    char little[] = {1, 0, 0, 0};
    int *n = (int*)little;

    if (1 == *n) {
        return true;
    }
    return false;
}

void IndexPart::load(int part) {
    assert(part < partCount);
    if (part == currentPart) {
        return;
    }
    currentPart = part;
    int elemSize = sizeof(SAType) + sizeof(BMType);
    qint64 size = 0;
    saLengths[part] = partFiles[part]->size()/elemSize;
    if (!partFiles[part]->isOpen()) {
        partFiles[part]->open(QIODevice::ReadOnly);
    }
    partFiles[part]->seek(0);

    char *buff = (char*)sArray;
    size = partFiles[part]->read(buff, saLengths[currentPart]*sizeof(SAType));
    assert(size == saLengths[currentPart]*sizeof(SAType));

    buff = (char*)bitMask;
    size = partFiles[part]->read(buff, saLengths[currentPart]*sizeof(BMType));
    assert(size == saLengths[currentPart]*sizeof(BMType));

    refFile->seek(seqStarts[currentPart]);
    size = refFile->read(seq, seqLengths[currentPart]);
    assert(size == seqLengths[currentPart]);

    if (!isLittleEndian()) {
        for (quint32 i=0; i<saLengths[currentPart]; i++) {
            sArray[i] = qFromLittleEndian<quint32>((uchar*)(sArray + i));
            bitMask[i] = qFromLittleEndian<quint64>((uchar*)(bitMask + i));
        }
    }
    //createBitmask(0, saLengths[currentPart]);
}

void IndexPart::createBitmask(int start, int last) {
    BMType bitValue = 0;
    char *s = NULL;
    static BitsTable bt;
    static const quint32 *bitTable = bt.getBitMaskCharBits(DNAAlphabet_NUCL);
    for (int i=start; i<last; i++) {
        bitValue = 0;
        s = seq + sArray[i];
        for (int j = 0; j < MAX_BIT_MASK_LENGTH; j++) {
            bitValue = (bitValue << 2) | bitTable[uchar(*(s+j))];
        }
        bitMask[i] = bitValue;
    }
}

SAType IndexPart::getMaxLength() {
    SAType maxLength = 0;
    for (int i=0; i<partCount; i++) {
        if (maxLength < seqLengths[i]) {
            maxLength = seqLengths[i];
        }
    }

    return maxLength;
}

SAType IndexPart::getLoadedSeqStart() {
    assert(currentPart>=0);
    return seqStarts[currentPart];
}

void IndexPart::build(int) {

}

void IndexPart::writePart(int part, quint32 arrLen) {
    partFiles[part]->open(QIODevice::ReadWrite);
    currentPart = part;

    if (!isLittleEndian()) {
        for (quint32 i=0; i<arrLen; i++) {
            qToLittleEndian(sArray[i], (uchar*)(sArray + i));
            qToLittleEndian(bitMask[i], (uchar*)(bitMask + i));
        }
    }

    partFiles[part]->write((char*)sArray, arrLen * sizeof(SAType));
    partFiles[part]->write((char*)bitMask, arrLen * sizeof(BMType));
}

} //U2
