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

#include "GUISaveFileTests.h"
#include "QtUtils.h"
#include "ProjectUtils.h"
#include "AppUtils.h"
#include "ToolTipUtils.h"
#include <U2Core/ProjectModel.h>
#include <U2Gui/ObjectViewModel.h>

namespace U2 {

void ProjectSaveAs::execute(U2OpStatus &os) {

 	ProjectUtils::openFile(os, testDir+"_common_data/scenarios/project/proj1.uprj");
	ProjectUtils::checkDocumentExists(os, "1CF7.PDB");
	AppUtils::checkUGENETitle(os, "proj1 UGENE");

	ProjectUtils::saveProjectAs(os, "proj2", testDir+"_common_data/scenarios/sandbox", "proj2");
	ProjectUtils::closeProject(os);

	ProjectUtils::openFile(os, testDir+"_common_data/scenarios/sandbox/proj2.uprj");
	ProjectUtils::checkDocumentExists(os, "1CF7.PDB");
	AppUtils::checkUGENETitle(os, "proj2 UGENE");

	ToolTipUtils::checkProjectTreeToolTip(os, "samples/PDB/1CF7.PDB", 0);
}

} // namespace
