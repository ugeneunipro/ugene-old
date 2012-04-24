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

#ifndef GTSEQUENCEVIEWUTILS_H
#define GTSEQUENCEVIEWUTILS_H

#include <QString>

namespace U2 {

class U2OpStatus;

class GTSequenceViewUtils {
public:
    static void getSequenceAsString(U2OpStatus &os, QString &sequence);
    static QString getBeginOfSequenceAsString(U2OpStatus &os, int length);
    static QString getEndOfSequenceAsString(U2OpStatus &os, int length);
    static int getLengthOfSequence(U2OpStatus &os);
    static void checkSequence(U2OpStatus &os, const QString &expectedSequence);
  	static void openSequenceView(U2OpStatus &os, const QString &sequenceName);
};

} // namespace U2

#endif // GTSEQUENCEVIEWUTILS_H
