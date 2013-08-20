/**
 * UGENE - Integrated Bioinformatics Tools.
 * Copyright (C) 2008-2013 UniPro <ugene@unipro.ru>
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

#include "ConductGOSupport.h"

#include <python/PythonSupport.h>
#include <R/RSupport.h>

#include <U2Core/AppContext.h>

namespace U2 {

ConductGOSupport::ConductGOSupport(const QString &name)
: ExternalTool(name)
{
    initialize();
}

void ConductGOSupport::initialize() {
    if (AppContext::getMainWindow()) {
        icon = QIcon(":external_tool_support/images/cmdline.png");
        grayIcon = QIcon(":external_tool_support/images/cmdline_gray.png");
        warnIcon = QIcon(":external_tool_support/images/cmdline_warn.png");
    }

    toolKitName = "Cistrome";
    description = ConductGOSupport::tr("<i>Conduct GO</i> - For a list of input genes, this tool uses R/BioC packages (GO, GOstats) to identify over represented GO terms.");

    executableFileName = "go_analysis.py";

    toolRunnerProgramm = ET_PYTHON;
    dependencies << ET_PYTHON
                 << ET_R
                 << ET_R_GOSTATS
                 << ET_R_GO_DB
                 << ET_R_HGU133A_DB
                 << ET_R_HGU133B_DB
                 << ET_R_HGU1333PLUS2_DB
                 << ET_R_HGU95AV2_DB
                 << ET_R_MOUSE430A2_DB
                 << ET_R_CELEGANS_DB
                 << ET_R_DROSOPHILA2_DB
                 << ET_R_ORG_HS_EG_DB
                 << ET_R_ORG_MM_EG_DB
                 << ET_R_ORG_CE_EG_DB
                 << ET_R_ORG_DM_EG_DB;

    validMessage = "Conduct GO";
    validationArguments << "--version";

    versionRegExp=QRegExp(executableFileName + "\\d+\\.\\d+");

    muted = true;
}

} // U2
