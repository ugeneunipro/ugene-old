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

#include "ToolTipUtils.h"
#include "QtUtils.h"
#include "ProjectUtils.h"

#include <U2Core/U2SafePoints.h>

#include <U2Core/DocumentSelection.h>
#include <U2Core/GObjectSelection.h>
#include <U2Gui/ProjectView.h>
#include <U2Core/ProjectModel.h>

namespace U2 {

void ToolTipUtils::checkExistingToolTip(U2OpStatus &os, const QString& tooltip) {

	QString t = getToolTip();
	CHECK_SET_ERR(t.contains(tooltip), "Tooltip is <" + t + ">, doesn't contain <" + tooltip + ">");
}

void ToolTipUtils::checkProjectTreeToolTip(U2OpStatus &os, const QString& tooltip, int num) {

	QPoint p = ProjectUtils::getTreeViewItemPosition(os, num);
	QtUtils::moveTo(os, "documentTreeWidget", p);
	QtUtils::sleep(1000);

	QString t = getToolTip();
	CHECK_SET_ERR(t.contains(tooltip), "Tooltip is <" + t + ">, doesn't contain <" + tooltip + ">");
}

QString ToolTipUtils::getToolTip() {

	return QToolTip::text();
}

}
