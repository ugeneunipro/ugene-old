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

#include "GUIExportProjectTests.h"
#include "QtUtils.h"
#include "ProjectUtils.h"
#include "GUIDialogUtils.h"
#include "AppUtils.h"
#include <U2Core/ProjectModel.h>
#include <U2Gui/ObjectViewModel.h>

namespace U2 {

void ExportProject::execute(U2OpStatus &os) {

    ProjectUtils::openFile(os, testDir + "_common_data/scenarios/project/1.gb");

    GUIDialogUtils::openExportProjectDialog(os);
    GUIDialogUtils::checkExportProjectDialog(os, "project.uprj");
}



void NoExportMenuItem::execute( U2OpStatus &os ){
    AppUtils::checkUGENETitle(os, "UGENE");
    QAction *result = QtUtils::getMenuAction(os, ACTION_PROJECTSUPPORT__EXPORT_PROJECT, MWMENU_FILE);
    CHECK_SET_ERR(result == NULL, "Export menu item present in menu without any project created");
}

} // namespace
