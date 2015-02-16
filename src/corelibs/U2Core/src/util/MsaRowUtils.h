/**
 * UGENE - Integrated Bioinformatics Tools.
 * Copyright (C) 2008-2015 UniPro <ugene@unipro.ru>
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

#ifndef _U2_MSAROWUTILS_H_
#define _U2_MSAROWUTILS_H_

#include <U2Core/global.h>
#include <U2Core/U2Msa.h>

namespace U2 {

class U2CORE_EXPORT MsaRowUtils {
public:
    static int getRowLength(const QByteArray &seq, const QList<U2MsaGap> &gaps);
    static int getGapsLength(const QList<U2MsaGap> &gaps);
    static char charAt(const QByteArray &seq, const QList<U2MsaGap> &gaps, int pos);
    static qint64 getRowLengthWithoutTrailing(const QByteArray &seq, const QList<U2MsaGap> &gaps);
    /**
     * The method maps `pos` in MSA coordinates to a character position in 'seq', i.e. gaps aren't taken into account.
     * If false == 'allowGapInPos' and the gap symbol is located in 'pos' then the method returns -1.
     * Otherwise if true == 'allowGapInPos' and the gap symbol is located in 'pos' then the method returns
     * the position of a non-gap character left-most to the 'pos'.
     */
    static int getUngappedPosition(const QByteArray &seq, const QList<U2MsaGap> &gaps, int pos, bool allowGapInPos = false);
    static int getCoreStart(const QList<U2MsaGap>& gaps);
};

} // U2

#endif // _U2_MSAROWUTILS_H_
