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

#include "AssemblyConsensusUtils.h"

namespace U2 {

namespace {
    inline char index2char(int index) {
        return "ACGT"[index];
    }

    inline int char2index(char c) {
        return QString("ACGT").indexOf(c, 0, Qt::CaseInsensitive);
    }

    inline bool isValidIndex(int index) {
        return index >= 0 && index < U2AssemblyBasesFrequenciesInfo::LETTERS_COUNT;
    }
}

U2AssemblyBasesFrequenciesInfo::U2AssemblyBasesFrequenciesInfo()
    : mostFrequentIndex(-1), totalBases(0)
{
    memset(baseFrequencies, 0, sizeof(baseFrequencies));
}

void U2AssemblyBasesFrequenciesInfo::addToCharFrequency(char c) {
    int index = char2index(c);
    if(isValidIndex(index)) {
        ++baseFrequencies[index];
        ++totalBases;

        if(!isValidIndex(mostFrequentIndex) || baseFrequencies[index] > baseFrequencies[mostFrequentIndex]) {
            mostFrequentIndex = index;
        }
    }
}

qint64 U2AssemblyBasesFrequenciesInfo::getCharFrequency(char c) {
    int index = index2char(c);
    return isValidIndex(index) ? baseFrequencies[index] : 0;
}

char U2AssemblyBasesFrequenciesInfo::getMostFrequentLetter() {
    return isValidIndex(mostFrequentIndex) ? index2char(mostFrequentIndex) : 'N';
}

QByteArray AssemblyBasesFrequenciesStat::getConsensusFragment() {
    int size = frequencyInfos.size();
    QByteArray res(size, ' ');
    for (int i = 0; i < size; ++i) {
        if(frequencyInfos[i].totalBases > 0) {
            res[i] = frequencyInfos[i].getMostFrequentLetter();
        }
    }
    return res;
}

} //namespace
