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

#ifndef _U2_DNA_CHROMATOGRAM_H_
#define _U2_DNA_CHROMATOGRAM_H_

#include <QtCore/QVector>
#include <U2Core/global.h>

namespace U2 {

class U2CORE_EXPORT DNAChromatogram {
public:
    DNAChromatogram() : traceLength(0), seqLength(0), hasQV(false) {}
    int traceLength;
    int seqLength;
    QVector<ushort> baseCalls;
    QVector<ushort> A;
    QVector<ushort> C;
    QVector<ushort> G;
    QVector<ushort> T;
    QVector<char> prob_A;
    QVector<char> prob_C;
    QVector<char> prob_G;
    QVector<char> prob_T;
    bool hasQV;
};

} //namespace

#endif
