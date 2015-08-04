/**
 * UGENE - Integrated Bioinformatics Tools.
 * Copyright (C) 2008-2015 UniPro <ugene@unipro.ru>
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

#include <qglobal.h>
#ifdef Q_OS_MAC

#include <execinfo.h>
#include <stdio.h>

#include <QDir>

#include <client/mac/handler/exception_handler.h>

#include <U2Core/AppContext.h>

#include "CrashHandler.h"
#include "CrashHandlerArgsHelper.h"
#include "CrashHandlerPrivateMac.h"

namespace U2 {

const QString CrashHandlerPrivateMac::STACKTRACE_FILE_PATH = "/tmp/UGENEstacktrace.txt";

CrashHandlerPrivateMac::~CrashHandlerPrivateMac()  {
    shutdown();
}

void CrashHandlerPrivateMac::setupHandler() {
#ifndef _DEBUG      // debugger fails to launch if exception handler is installed
    QFile(STACKTRACE_FILE_PATH).remove();

    const QString dumpDir = QDir::tempPath() + "/ugene_crashes";
    QDir().mkpath(dumpDir);

    breakpadHandler = new google_breakpad::ExceptionHandler(dumpDir.toStdString(), NULL, breakpadCallback, NULL, true, NULL);
#endif
}

void CrashHandlerPrivateMac::shutdown() {
    delete breakpadHandler;
    breakpadHandler = NULL;
}

void CrashHandlerPrivateMac::storeStackTrace() const {
    const QString path = AppContext::getWorkingDirectoryPath() + "/ugenem";

    char pid_buf[30];
    sprintf(pid_buf, "%d", getpid());
    char name_buf[512];
    name_buf[readlink(path.toLatin1().constData(), name_buf, 511)] = 0;
    FILE *fp;
    fp = freopen(STACKTRACE_FILE_PATH.toLocal8Bit().constData(), "w+",stdout);
    void * stackTrace[1024];
    int frames = backtrace(stackTrace, 1024);
    backtrace_symbols_fd(stackTrace, frames, STDOUT_FILENO);
    fclose(fp);
}

bool CrashHandlerPrivateMac::breakpadCallback(const char *dump_dir, const char *minidump_id, void * /*context*/, bool succeeded) {
    QString dumpUrl;
    if (succeeded) {
        dumpUrl = QString::fromLocal8Bit(dump_dir) + "/" + QString::fromLocal8Bit(minidump_id) + ".dmp";
    }

    handleException("C++ exception|Unhandled exception", dumpUrl);

    return succeeded;
}

}   // namespace U2

#endif
