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

#include "ConservationPlotSupport.h"
#include "python/PythonSupport.h"
#include "R/RSupport.h"
#include "utils/ExternalToolUtils.h"

#include <U2Core/AppContext.h>
#include <U2Core/Settings.h>

namespace U2 {

ConservationPlotSupport::ConservationPlotSupport(const QString &name)
: ExternalTool(name)
{
    initialize();
}

void ConservationPlotSupport::initialize() {
    if (AppContext::getMainWindow()) {
        icon = QIcon(":external_tool_support/images/cmdline.png");
        grayIcon = QIcon(":external_tool_support/images/cmdline_gray.png");
        warnIcon = QIcon(":external_tool_support/images/cmdline_warn.png");
    }

    toolKitName = "Cistrome";
    description = ConservationPlotSupport::tr("<i>ConservationPlot</i> - Draw conservation plot for many bed files.");

    executableFileName = "conservation_plot.py";

    toolRunnerProgramm = ET_PYTHON;
    dependencies << ET_PYTHON
                 << ET_R;

    validMessage = "conservation_plot.py ";
    validationArguments << "--version";

    versionRegExp=QRegExp(executableFileName + " (\\d+\\.\\d+)");

    ExternalToolUtils::addDefaultCistromeDirToSettings();
    ExternalToolUtils::addCistromeDataPath(CONSERVATION_DATA_NAME, CONSERVATION_DIR_NAME, true);

    muted = true;
}

} // U2
