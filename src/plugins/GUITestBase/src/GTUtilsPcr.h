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

#ifndef _GTUTILS_PCR_H_
#define _GTUTILS_PCR_H_

#include <U2Core/U2OpStatus.h>
#include <U2Core/U2Type.h>


class QTableView;

namespace U2 {

class GTUtilsPcr {
public:
    static void setPrimer(U2OpStatus &os, U2Strand::Direction direction, const QByteArray &primer);
    static void setMismatches(U2OpStatus &os, U2Strand::Direction direction, int mismatches);
    static void setPerfectMatch(U2OpStatus &os, int number);
    static void setMaxProductSize(U2OpStatus &os, int number);
    static QWidget * browseButton(U2OpStatus &os, U2Strand::Direction direction);
    static int productsCount(U2OpStatus &os);
    static QString getResultRegion(U2OpStatus &os, int number);
    static QPoint getResultPoint(U2OpStatus &os, int number);
    static QPoint getDetailsPoint(U2OpStatus &os);
    static QWidget * primerBox(U2OpStatus &os, U2Strand::Direction direction);
    static void clearPcrDir(U2OpStatus &os);

private:
    static QTableView * table(U2OpStatus &os);
};

} // U2

#endif // _GTUTILS_PCR_H_
