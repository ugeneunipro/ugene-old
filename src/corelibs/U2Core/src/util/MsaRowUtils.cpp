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

#include <U2Core/U2SafePoints.h>

#include "MsaRowUtils.h"

namespace U2 {

/** Gap character */
#define MAlignment_GapChar '-'

int MsaRowUtils::getRowLength(const QByteArray &seq, const QList<U2MsaGap> &gaps) {
    return seq.length() + getGapsLength(gaps);
}

int MsaRowUtils::getGapsLength(const QList<U2MsaGap> &gaps) {
    int length = 0;
    foreach (U2MsaGap elt, gaps) {
        length += elt.gap;
    }
    return length;
}

char MsaRowUtils::charAt(const QByteArray &seq, const QList<U2MsaGap> &gaps, int pos) {
    if (pos < 0 || pos >= getRowLength(seq, gaps)) {
        return MAlignment_GapChar;
    }

    int gapsLength = 0;
    foreach (const U2MsaGap &gap, gaps) {
        // Current gap is somewhere further in the row
        if (gap.offset > pos) {
            break;
        }
        // Inside the gap
        else if ((pos >= gap.offset) && (pos < gap.offset + gap.gap)) {
            return MAlignment_GapChar;
        }
        // Go further in the row, calculating the current gaps length
        else {
            gapsLength += gap.gap;
        }
    }

    if (pos >= gapsLength + seq.length()) {
        return MAlignment_GapChar;
    }

    int index = pos - gapsLength;
    bool indexIsInBounds = (index < seq.length()) && (index >= 0);

    SAFE_POINT(indexIsInBounds,
        QString("Internal error detected in MAlignmentRow::charAt,"
        " row length is '%1', gapsLength is '%2'!").arg(getRowLength(seq, gaps)).arg(index), MAlignment_GapChar);
    return seq[index];
}

qint64 MsaRowUtils::getRowLengthWithoutTrailing(const QByteArray &seq, const QList<U2MsaGap> &gaps) {
    int rowLength = getRowLength(seq, gaps);
    int rowLengthWithoutTrailingGap = rowLength;
    if (!gaps.isEmpty()) {
        if (MAlignment_GapChar == charAt(seq, gaps, rowLength - 1)) {
            U2MsaGap lastGap = gaps.last();
            rowLengthWithoutTrailingGap -= lastGap.gap;
        }
    }
    return rowLengthWithoutTrailingGap;
}

int MsaRowUtils::getUngappedPosition(const QByteArray &seq, const QList<U2MsaGap> &gaps, int pos) {
    if (MAlignment_GapChar == charAt(seq, gaps, pos)) {
        return -1;
    }

    int gapsLength = 0;
    foreach (const U2MsaGap &gap, gaps) {
        if (gap.offset < pos) {
            gapsLength += gap.gap;
        } else {
            break;
        }
    }
    return (pos - gapsLength);
}

} // U2
