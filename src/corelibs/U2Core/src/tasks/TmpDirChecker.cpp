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

#include "TmpDirChecker.h"
#include <U2Core/AppContext.h>
#include <U2Core/AppSettings.h>
#include <U2Core/UserApplicationsSettings.h>
#include <QFile>
#include <QDir>

namespace U2 {

TmpDirChecker::TmpDirChecker(): Task("Checking access rights to the temporary folder", TaskFlag_None) // TODO: make translation
{
}

void TmpDirChecker::run()
{
    tempDirPath = AppContext::getAppSettings()->getUserAppsSettings()->getUserTemporaryDirPath();
    if (!checkPath(tempDirPath)) {
        if (!AppContext::isGUIMode()) {
            QString message = "You do not have permission to write to \"" + tempDirPath +
                "\" directory. Use --tmp-dir=<path_to_file> to set new temporary directory";
            coreLog.error((message));
        }
        else {
            emit si_checkFailed(tempDirPath);
        }
    }
}

Task::ReportResult TmpDirChecker::report()
{
    if (hasError() && AppContext::isGUIMode()) {
        stateInfo.setError(getError());
    }

    return ReportResult_Finished;
}

bool TmpDirChecker::checkPath(QString &path)
{
    QDir dir;
    dir.mkpath(path);
    QFile tmpFile(path + "/forCheck");
    if (!tmpFile.open(QIODevice::WriteOnly)) {
//#ifdef Q_OS_MAC
//        //Mac menu differs from menus of other systems. Maybe something else?
//        setError("You do not have permission to write to \"" + path +
//            "\" directory. Please, set the valid temp directory in preferences (Unipro UGENE->Preferences->General->Path to temporary files)");
//#else
//        setError("You do not have permission to write to \"" + path +
//            "\" directory. Please, set the valid temp directory in preferences (Tools->Preferences->General->Path to temporary files)");
//#endif
        return false;
    } else {
        tmpFile.close();
        tmpFile.remove();
    }

    return true;
}

}   //namespace U2
