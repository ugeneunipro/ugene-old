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

#include "CEASSupport.h"

#include <U2Core/AppContext.h>
#include <U2Core/DataPathRegistry.h>


namespace U2 {

const QString CEASSupport::TOOL_NAME("CEAS Tools");
const QString CEASSupport::REF_GENES_DATA_NAME("Gene annotation table");


CEASSupport::CEASSupport(const QString &name)
: ExternalTool(name)
{
    initialize();
}

void CEASSupport::initialize() {
    if (AppContext::getMainWindow()) {
        icon = QIcon(":external_tool_support/images/cmdline.png");
        grayIcon = QIcon(":external_tool_support/images/cmdline_gray.png");
        warnIcon = QIcon(":external_tool_support/images/cmdline_warn.png");
    }

    toolKitName = "Cistrome";
    description = CEASSupport::tr("<i>CEAS</i> - Cis-regulatory Element Annotation System -"
        " helps to characterize genome-wide protein-DNA interaction patterns from ChIP-chip"
        " and ChIP-Seq of both sharp and broad binding factors."
        " It provides statistics on ChIP enrichment at important genome features such as"
        " specific chromosome, promoters, gene bodies, or exons, and infers genes most likely"
        " to be regulated by a binding factor.");

    executableFileName = "ceas.py";

    validMessage = "package version";
    validationArguments << "--version";

    versionRegExp=QRegExp(executableFileName + " -- (\\d+\\.\\d+\\.\\d+.\\d+) \\(package version (\\d+\\.\\d+\\.\\d+)\\)");

    U2DataPathRegistry* dpr = AppContext::getDataPathRegistry();
    if (dpr){
        U2DataPath* dp = new U2DataPath(REF_GENES_DATA_NAME, QString(PATH_PREFIX_DATA)+QString(":")+"cistrome/refGene");
        dpr->registerEntry(dp);
    }
}

} // U2
