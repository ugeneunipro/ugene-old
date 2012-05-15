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

#ifndef GTUTILSMSAEDITORSEQUENCEAREA_H
#define GTUTILSMSAEDITORSEQUENCEAREA_H

#include "api/GTGlobals.h"

namespace U2 {

class GTUtilsMSAEditorSequenceArea {
public:
    static void checkSelectedRect(U2OpStatus &os, const QRect &expectedRect); // see example in GUITest_common_scenarios_msa_editor::test_0004
    static int getLeftOffset(U2OpStatus &os);
    static int getRightOffset(U2OpStatus &os);

    static int getLength(U2OpStatus &os);
    static void putCursor(U2OpStatus &os, int x, int y);
    static void selectArea(U2OpStatus &os, const QPoint &topLeft, const QPoint &bottomRigth);

    static bool offsetsVisible(U2OpStatus &os);
};

} // namespace
#endif // GTUTILSMSAEDITORSEQUENCEAREA_H
