/**
 * UGENE - Integrated Bioinformatics Tools.
 * Copyright (C) 2008-2013 UniPro <ugene@unipro.ru>
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

#ifndef GTUTILSMSAEDITORSEQUENCEAREA_H
#define GTUTILSMSAEDITORSEQUENCEAREA_H

#include "api/GTGlobals.h"
#include <U2View/MSAEditorSequenceArea.h>

namespace U2 {

class GTUtilsMSAEditorSequenceArea {
public:
    static void checkSelectedRect(U2OpStatus &os, const QRect &expectedRect);
    static void checkSorted(U2OpStatus &os, bool sortedState = true);

    static void checkConsensus(U2OpStatus &os, QString cons);

    // MSAEditorNameList
    static QStringList getNameList(U2OpStatus &os);

    static int getLeftOffset(U2OpStatus &os);
    static int getRightOffset(U2OpStatus &os);

    static int getLength(U2OpStatus &os);

    static void moveTo(U2OpStatus &os, const QPoint &p);

    // selects area in MSA coordinats, if some p coordinate less than 0, it becomes max valid coordinate
    static void selectArea(U2OpStatus &os, QPoint p1 = QPoint(0, 0), QPoint p2 = QPoint(-1, -1));
    static void click(U2OpStatus &os, QPoint p = QPoint(0, 0));

    static bool offsetsVisible(U2OpStatus &os);
};

} // namespace
#endif // GTUTILSMSAEDITORSEQUENCEAREA_H
