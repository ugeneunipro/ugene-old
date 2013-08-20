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

#include "CufflinksSupport.h"

#include <U2Core/AppContext.h>

#include <U2Gui/MainWindow.h>


namespace U2 {

CufflinksSupport::CufflinksSupport(const QString& name, const QString& path)
    : ExternalTool(name, path)
{
    if (AppContext::getMainWindow()) {
        icon = QIcon(":external_tool_support/images/cmdline.png");
        grayIcon = QIcon(":external_tool_support/images/cmdline_gray.png");
        warnIcon = QIcon(":external_tool_support/images/cmdline_warn.png");
    }

    toolKitName = "Cufflinks";

    // Cuffcompare
    if (name == ET_CUFFCOMPARE) {
#ifdef Q_OS_WIN
        executableFileName = "cuffcompare.exe";
#else
#if defined(Q_OS_LINUX) || defined(Q_OS_MAC)
        executableFileName = "cuffcompare";
#endif
#endif

        validMessage = "cuffcompare";
        description = CufflinksSupport::tr("<i>Cuffcompare</i> helps"
            " comparing assembled transcripts to a reference annotation,"
            " and also tracking transcripts across multiple experiments.");
    }

    // Cuffdiff
    else if (name == ET_CUFFDIFF) {
#ifdef Q_OS_WIN
        executableFileName = "cuffdiff.exe";
#else
#if defined(Q_OS_LINUX) || defined(Q_OS_MAC)
        executableFileName = "cuffdiff";
#endif
#endif

        validMessage = "cuffdiff";
        description = CufflinksSupport::tr("<i>Cuffdiff</i> &nbsp;tests for"
            " differential expression and regulation in RNA-Seq samples.");
    }

    // Cufflinks
    else if (name == ET_CUFFLINKS) {
#ifdef Q_OS_WIN
        executableFileName = "cufflinks.exe";
#else
#if defined(Q_OS_LINUX) || defined(Q_OS_MAC)
        executableFileName = "cufflinks";
#endif
#endif

        validMessage = "cufflinks";
        description = CufflinksSupport::tr("<i>Cufflinks</i> assembles transcripts"
            " and estimates their abundances.");
    }


    // Cuffmerge
    else if (name == ET_CUFFMERGE) {
#ifdef Q_OS_WIN
        executableFileName = "cuffmerge.py";
#else
#if defined(Q_OS_LINUX) || defined(Q_OS_MAC)
        executableFileName = "cuffmerge";
#endif
#endif

        validMessage = "cuffmerge";
        description = CufflinksSupport::tr("<i>Cuffmerge</i> merges together several assemblies.");
    }

    muted = true;
}


} // namespace
