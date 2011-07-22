#include <U2Core/AppContext.h>

#include "BowtieSupport.h"

namespace U2 {

// BowtieSupport

BowtieSupport::BowtieSupport(const QString &name, const QString &path):
    ExternalTool(name, path)
{
    if (AppContext::getMainWindow()) {
        icon = QIcon(":external_tool_support/images/cmdline.png");
        grayIcon = QIcon(":external_tool_support/images/cmdline_gray.png");
        warnIcon = QIcon(":external_tool_support/images/cmdline_warn.png");
    }
    if(BOWTIE_TOOL_NAME == name) {
#ifdef Q_OS_WIN
        executableFileName="bowtie.exe";
#else
    #ifdef Q_OS_LINUX
        executableFileName="bowtie";
    #endif
#endif
    } else {
#ifdef Q_OS_WIN
        executableFileName="bowtie-build.exe";
#else
    #ifdef Q_OS_LINUX
        executableFileName="bowtie-build";
    #endif
#endif
    }
    validationArguments.append("--version");
    validMessage="version";
    description=tr("<i>Bowtie<i> is an ultrafast, memory-efficient short read aligner. "
                   "It aligns short DNA sequences (reads) to the human genome at "
                   "a rate of over 25 million 35-bp reads per hour. "
                   "Bowtie indexes the genome with a Burrows-Wheeler index to keep "
                   "its memory footprint small: typically about 2.2 GB for the human "
                   "genome (2.9 GB for paired-end).");
    versionRegExp=QRegExp("version (\\d+\\.\\d+\\.\\d+)");
    toolKitName="BowtieToolkit";
}

} // namespace U2
