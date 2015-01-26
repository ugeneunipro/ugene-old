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

#ifndef _GTUTILS_PRIMER_LIBRARY_H_
#define _GTUTILS_PRIMER_LIBRARY_H_

#include <U2Core/U2OpStatus.h>

class QAbstractButton;
class QTableView;

namespace U2 {

class GTUtilsPrimerLibrary {
public:
    enum Button {Add, Edit, Remove, Import, Export, Close};
    static QWidget * openLibrary(U2OpStatus &os);
    static void clickButton(U2OpStatus &os, Button button);
    static QAbstractButton * getButton(U2OpStatus &os, Button button);
    static int librarySize(U2OpStatus &os);
    static QString getPrimerSequence(U2OpStatus &os, int number);
    static QString getPrimerSequence(U2OpStatus &os, const QString &name);
    static QPoint getPrimerPoint(U2OpStatus &os, int number);
    static void clickPrimer(U2OpStatus &os, int number);
    static void clearLibrary(U2OpStatus &os);
    static void addPrimer(U2OpStatus &os, const QString &name, const QString &data);
    static void selectPrimers(U2OpStatus &os, const QList<int> &numbers);
    static void selectAll(U2OpStatus &os);

private:
    static QTableView * table(U2OpStatus &os);
};

} // U2

#endif // _GTUTILS_PRIMER_LIBRARY_H_
