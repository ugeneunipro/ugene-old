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

#include "GUIInitialChecks.h"
#include "QtUtils.h"
#include "ProjectUtils.h"
#include <U2Core/ProjectModel.h>
#include <U2Gui/ObjectViewModel.h>

#include <U2Core/U2SafePoints.h>

namespace U2 {

void GUIInitialCheck::execute(U2OpStatus& os) {
	QString activeWindowName = AppContext::getActiveWindowName();
	CHECK_SET_ERR(activeWindowName.isEmpty(), "Active window name is not empty");

	QMainWindow *mainWindow = AppContext::getMainWindow()->getQMainWindow();
	CHECK_SET_ERR(mainWindow->isActiveWindow(), "MainWindow is not active");
}

void GUILoadedCheck::execute(U2OpStatus &os) {
	CHECK_SET_ERR(AppContext::getProjectView() == NULL && AppContext::getProject() == NULL, "There is a project");
}

} // namespace
