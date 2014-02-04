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

#include "ExternalToolUtils.h"

#include <U2Core/AppContext.h>
#include <U2Core/DataPathRegistry.h>
#include <U2Core/Settings.h>
#include <U2Core/U2SafePoints.h>

#include <QtCore/QFile>
#include <QtCore/QFileInfo>

namespace U2 {

const QString ExternalToolUtils::CISTROME_DATA_DIR = "CISTROME_DATA_DIR";

void ExternalToolUtils::addDefaultCistromeDirToSettings() {
    const QString cistromeDefaultPath = QFileInfo(QString(PATH_PREFIX_DATA) + QString(":") + "cistrome").absoluteFilePath();

    const bool defaultExists = QFile::exists(cistromeDefaultPath);
    const QString savedValue = AppContext::getSettings()->getValue(CISTROME_DATA_DIR).toString();

    bool addNew = savedValue.isEmpty() && defaultExists;
    bool removeOld = !savedValue.isEmpty() && !QFile::exists(savedValue);
    bool replaceOld = removeOld && defaultExists;

    if (addNew || replaceOld) {
        AppContext::getSettings()->setValue(CISTROME_DATA_DIR, cistromeDefaultPath);
    } else if (removeOld) {
        AppContext::getSettings()->remove(CISTROME_DATA_DIR);
    }
}

void ExternalToolUtils::addCistromeDataPath(const QString& dataName, const QString& dirName, bool entriesAreFolders) {
    U2DataPathRegistry* dpr = AppContext::getDataPathRegistry();
    CHECK(NULL != dpr, );

    const QString dataPath = AppContext::getSettings()->getValue(CISTROME_DATA_DIR).toString() + QDir::separator() + dirName;
    U2DataPath* dp = new U2DataPath(dataName, dataPath, entriesAreFolders);
    bool ok = dpr->registerEntry(dp);
    if (!ok) {
        delete dp;
    }
}

}   // namespace U2

