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

#ifndef _U2_GT_UTILS_ASSEMBLY_BROWSER_H_
#define _U2_GT_UTILS_ASSEMBLY_BROWSER_H_

#include <QtGlobal>

namespace U2 {

class AssemblyBrowserUi;
class AssemblyModel;
class U2OpStatus;

class GTUtilsAssemblyBrowser {
public:
    static AssemblyBrowserUi *getView(U2OpStatus &os, const QString& viewTitle);

    static bool hasReference(U2OpStatus &os, const QString &viewTitle);
    static bool hasReference(U2OpStatus &os, QWidget *view);
    static bool hasReference(U2OpStatus &os, AssemblyBrowserUi *assemblyBrowser);

    static qint64 getLength(U2OpStatus &os, const QString& viewTitle);
    static qint64 getReadsCount(U2OpStatus &os, const QString& viewTitle);

    static void zoomToMax(U2OpStatus& os);

    static void goToPosition(U2OpStatus &os, qint64 position);
};

}   // namespace U2

#endif // _U2_GT_UTILS_ASSEMBLY_BROWSER_H_
