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

#include <U2Core/AppContext.h>

#include <U2Gui/MainWindow.h>

#include "TopHatSupport.h"
#include "python/PythonSupport.h"

namespace U2 {


TopHatSupport::TopHatSupport(const QString& name, const QString& path)
    : ExternalTool(name, path)
{
    if (AppContext::getMainWindow()) {
        icon = QIcon(":external_tool_support/images/cmdline.png");
        grayIcon = QIcon(":external_tool_support/images/cmdline_gray.png");
        warnIcon = QIcon(":external_tool_support/images/cmdline_warn.png");
    }

#ifdef Q_OS_WIN
    executableFileName = "tophat.exe";
#else
#if defined(Q_OS_UNIX)
    executableFileName = "tophat";
#endif
#endif
    validationArguments << "--version";

    dependencies << ET_PYTHON;

    validMessage = "TopHat ";
    description = "<i>TopHat</i> is a program that aligns RNA-Seq reads to a genome"
        " in order to identify exon-exon splice junctions. It is built on"
        " the ultrafast short read mapping program Bowtie.";
    versionRegExp = QRegExp("(\\d+.\\d+.\\d+\\w?)");
    toolKitName = "TopHat";

    muted = true;
}

} // namespace
