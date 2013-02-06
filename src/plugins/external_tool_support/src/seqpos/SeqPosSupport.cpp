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

#include <U2Core/AppContext.h>
#include <U2Core/DataPathRegistry.h>

namespace U2 {

const QString SeqPosSupport::TOOL_NAME("seqpos");
const QString SeqPosSupport::ASSEMBLY_DIR("Assembly dir");

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

    validMessage = "mdseqpos";
    validationArguments << "--version";

    versionRegExp=QRegExp("Version \\d+\\.\\d+");

    U2DataPathRegistry* dpr = AppContext::getDataPathRegistry();
    if (dpr){
        U2DataPath* dp = new U2DataPath(ASSEMBLY_DIR, QString(PATH_PREFIX_DATA)+QString(":")+"cistrome/genomes", true);
        dpr->registerEntry(dp);
    }

}

} // U2
