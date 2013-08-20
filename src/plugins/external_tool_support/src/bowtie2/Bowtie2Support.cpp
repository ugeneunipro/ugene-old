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

#include "Bowtie2Support.h"

#include <U2Core/AppContext.h>

#include <U2Gui/MainWindow.h>


namespace U2 {

Bowtie2Support::Bowtie2Support(const QString& name, const QString& path /* = */ )
    : ExternalTool(name, path)
{
    if (AppContext::getMainWindow()) {
        icon = QIcon(":external_tool_support/images/cmdline.png");
        grayIcon = QIcon(":external_tool_support/images/cmdline_gray.png");
        warnIcon = QIcon(":external_tool_support/images/cmdline_warn.png");
    }

    toolKitName = "Bowtie2";

    // Bowtie2-align
    if (name == ET_BOWTIE2_ALIGN) {
#ifdef Q_OS_WIN
        executableFileName = "bowtie2-align.exe";
#else
#if defined(Q_OS_LINUX) || defined(Q_OS_MAC)
        executableFileName = "bowtie2-align";
#endif
#endif
        validationArguments << "--help";
        validMessage = "bowtie2-align";
        description = Bowtie2Support::tr("<i>Bowtie 2 aligner</i> takes a Bowtie 2 index"
            " and a set of sequencing read files and outputs a set of alignments.");
    }


    // Bowtie2-build
    else if (name == ET_BOWTIE2_BUILD) {
#ifdef Q_OS_WIN
        executableFileName = "bowtie2-build.exe";
#else
#if defined(Q_OS_LINUX) || defined(Q_OS_MAC)
        executableFileName = "bowtie2-build";
#endif
#endif
        validationArguments << "--help";
        validMessage = "bowtie2-build";
        description = Bowtie2Support::tr("<i>Bowtie 2 build indexer</i> "
            " builds a Bowtie index from a set of DNA sequences. It outputs"
            " a set of 6 files with suffixes .1.bt2, .2.bt2, .3.bt2, .4.bt2,"
            " .rev.1.bt2, and .rev.2.bt2. These files together constitute the index:"
            " they are all that is needed to align reads to that reference."
            " The original sequence files are no longer used by <i>Bowtie 2</i>"
            " once the index is built.");
    }


    // Bowtie2-inspect
    else if (name == ET_BOWTIE2_INSPECT) {
#ifdef Q_OS_WIN
        executableFileName = "bowtie2-inspect.exe";
#else
#if defined(Q_OS_LINUX) || defined(Q_OS_MAC)
        executableFileName = "bowtie2-inspect";
#endif
#endif
        validationArguments << "--help";
        validMessage = "bowtie2-inspect";
        description = Bowtie2Support::tr("<i>Bowtie 2 index inspector</i>"
            " extracts information from a Bowtie index about what kind"
            " of index it is and what reference sequence were used to build it.");
    }
}

} // namespace
