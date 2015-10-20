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

#ifndef _U2_GT_UTILS_ASSEMBLY_BROWSER_H_
#define _U2_GT_UTILS_ASSEMBLY_BROWSER_H_

#include <QtGlobal>
#include <QModelIndex>
namespace U2 {

class AssemblyBrowserUi;
class AssemblyModel;
class U2OpStatus;

class GTUtilsAssemblyBrowser {
public:
    enum Area {
        Consensus,
        Overview,
        Reads
    };

    static AssemblyBrowserUi *getView(U2OpStatus &os, const QString& viewTitle);

    static void addRefFromProject(U2OpStatus &os, QString docName, QModelIndex parent = QModelIndex());

    static bool hasReference(U2OpStatus &os, const QString &viewTitle);
    static bool hasReference(U2OpStatus &os, QWidget *view = NULL);
    static bool hasReference(U2OpStatus &os, AssemblyBrowserUi *assemblyBrowser);

    static qint64 getLength(U2OpStatus &os);
    static qint64 getReadsCount(U2OpStatus &os);

    static bool isWelcomeScreenVisible(U2OpStatus &os);

    static void zoomIn(U2OpStatus& os);
    static void zoomToMax(U2OpStatus& os);
    static void zoomToMin(U2OpStatus& os);

    static void goToPosition(U2OpStatus &os, qint64 position);

    static void callContextMenu(U2OpStatus &os, Area area = Consensus);
    static void callExportCoverageDialog(U2OpStatus &os, Area area = Consensus);
};

}   // namespace U2

#endif // _U2_GT_UTILS_ASSEMBLY_BROWSER_H_
