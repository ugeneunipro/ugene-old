/**
 * UGENE - Integrated Bioinformatics Tools.
 * Copyright (C) 2008-2012 UniPro <ugene@unipro.ru>
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

namespace U2 {

TmpDirChecker::TmpDirChecker(): Task("Checking access rights to the temporary folder", TaskFlag_None) // TODO: make translation
{
}

void TmpDirChecker::run()
{
    tmpDirPath = AppContext::getAppSettings()->getUserAppsSettings()->getUserTemporaryDirPath();

    if (!checkPath(tmpDirPath) && !AppContext::isGUIMode())  {
        QString message = getError() + ". Use --tmp-dir=<path_to_file> to set new temporary directory";
        coreLog.error((message));
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
    QFile tmpFile(path + "/forCheck");

    if (!tmpFile.open(QIODevice::WriteOnly)) {
        setError("You do not have permission to write to \"" + tmpDirPath + "\" directory");
        return false;
    } else {
        tmpFile.close();
        tmpFile.remove();
    }

    return true;
}

}
