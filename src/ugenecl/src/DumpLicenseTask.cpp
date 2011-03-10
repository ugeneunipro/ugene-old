#include <QtCore/QFile>

#include <U2Core/AppContext.h>
#include <U2Core/CMDLineRegistry.h>
#include <U2Core/CMDLineHelpProvider.h>

#include "DumpLicenseTask.h"

namespace U2 {

const QString DumpLicenseTask::LICENSE_CMDLINE_OPTION = "license";
const QString DumpLicenseTask::LICENCE_FILENAME = "license";

void DumpLicenseTask::initHelp() {
    CMDLineRegistry * cmdlineRegistry = AppContext::getCMDLineRegistry();
    CMDLineHelpProvider * helpSection = new CMDLineHelpProvider( LICENSE_CMDLINE_OPTION, tr( "dump license information" ) );
    cmdlineRegistry->registerCMDLineHelpProvider(helpSection);
}

DumpLicenseTask::DumpLicenseTask() : Task(tr("Dump license information task"), TaskFlag_None) {
}

void DumpLicenseTask::run() {
    QFile file(QString( PATH_PREFIX_DATA ) + ":" + LICENCE_FILENAME );
    if(!file.open(QIODevice::ReadOnly)) {
        setError("Cannot find license file");
        return;
    }
    QByteArray licenseText = file.readAll();
    fprintf(stdout, "%s", licenseText.constData());
}

} // U2
