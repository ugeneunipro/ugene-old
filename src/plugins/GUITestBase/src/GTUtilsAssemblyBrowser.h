/**
 * UGENE - Integrated Bioinformatics Tools.
 * Copyright (C) 2008-2016 UniPro <ugene@unipro.ru>
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
#include <GTGlobals.h>

namespace U2 {

class AssemblyBrowserUi;
class AssemblyModel;

class GTUtilsAssemblyBrowser {
public:
    enum Area {
        Consensus,
        Overview,
        Reads
    };

    static AssemblyBrowserUi *getView(HI::GUITestOpStatus &os, const QString& viewTitle);

    static void addRefFromProject(HI::GUITestOpStatus &os, QString docName, QModelIndex parent = QModelIndex());

    static bool hasReference(HI::GUITestOpStatus &os, const QString &viewTitle);
    static bool hasReference(HI::GUITestOpStatus &os, QWidget *view = NULL);
    static bool hasReference(HI::GUITestOpStatus &os, AssemblyBrowserUi *assemblyBrowser);

    static qint64 getLength(HI::GUITestOpStatus &os);
    static qint64 getReadsCount(HI::GUITestOpStatus &os);

    static bool isWelcomeScreenVisible(HI::GUITestOpStatus &os);

    static void zoomIn(HI::GUITestOpStatus& os);
    static void zoomToMax(HI::GUITestOpStatus& os);
    static void zoomToMin(HI::GUITestOpStatus& os);

    static void goToPosition(HI::GUITestOpStatus &os, qint64 position);

    static void callContextMenu(HI::GUITestOpStatus &os, Area area = Consensus);
    static void callExportCoverageDialog(HI::GUITestOpStatus &os, Area area = Consensus);
};

}   // namespace U2

#endif // _U2_GT_UTILS_ASSEMBLY_BROWSER_H_
