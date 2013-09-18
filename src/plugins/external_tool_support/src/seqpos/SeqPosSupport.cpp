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

#include "SeqPosSupport.h"

#include <python/PythonSupport.h>
#include <R/RSupport.h>

#include <U2Core/AppContext.h>
#include <U2Core/DataPathRegistry.h>
#include <U2Core/Settings.h>

namespace U2 {

SeqPosSupport::SeqPosSupport(const QString &name)
: ExternalTool(name)
{
    initialize();
}

void SeqPosSupport::initialize() {
    if (AppContext::getMainWindow()) {
        icon = QIcon(":external_tool_support/images/cmdline.png");
        grayIcon = QIcon(":external_tool_support/images/cmdline_gray.png");
        warnIcon = QIcon(":external_tool_support/images/cmdline_warn.png");
    }

    toolKitName = "Cistrome";
    description = SeqPosSupport::tr("<i>SeqPos</i> - Finds motifs enriched in a set of regions.");

    executableFileName = "MDSeqPos.py";

    toolRunnerProgramm = ET_PYTHON;
    dependencies << ET_PYTHON
                 << ET_R
                 << ET_R_SEQLOGO;

    validMessage = "mdseqpos";
    validationArguments << "--version";

    versionRegExp=QRegExp("Version \\d+\\.\\d+");

    U2DataPathRegistry* dpr = AppContext::getDataPathRegistry();
    if (dpr) {
        QString assemblyPath = AppContext::getSettings()->getValue(CISTROME_DATA_DIR).toString() + "/" + ASSEMBLY_DIR_NAME;
        U2DataPath* dp = new U2DataPath(ASSEMBLY_DIR, assemblyPath);
        if (!dp->isValid()) {
            delete dp;
            dp = new U2DataPath(ASSEMBLY_DIR, QString(PATH_PREFIX_DATA) + QString(":") + "cistrome/" + ASSEMBLY_DIR_NAME, true);
        }

        if (!dpr->registerEntry(dp)) {
            delete dp;
        }
    }

    errorDescriptions.insert("CRITICAL: numpy 1.3 or greater must be installed", SeqPosSupport::tr("Please, install numpy 1.3 or greater for your Python to run SeqPos"));
    errorDescriptions.insert("CRITICAL: DJANGO 1.1.1 or greater must be installed", SeqPosSupport::tr("Please, install DJANGO 1.1.1 or greater for your Python to run SeqPos"));

    muted = true;
}

} // U2
