/**
 * UGENE - Integrated Bioinformatics Tools.
 * Copyright (C) 2008-2014 UniPro <ugene@unipro.ru>
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

#include <U2Core/U2SafePoints.h>

#include "UtilTestActions.h"

namespace U2 {

/************************************************************************/
/* GTest_CopyFile */
/************************************************************************/
const QString GTest_CopyFile::FROM_URL_ATTR = "from";
const QString GTest_CopyFile::TO_URL_ATTR ="to";

void GTest_CopyFile::init(XMLTestFormat *, const QDomElement &el) {
    fromUrl = el.attribute(FROM_URL_ATTR);
    toUrl = el.attribute(TO_URL_ATTR);
    replacePrefix(fromUrl);
    replacePrefix(toUrl);
}

Task::ReportResult GTest_CopyFile::report() {
    QDir().mkpath(QFileInfo(toUrl).absoluteDir().absolutePath());
    if (!QFile::copy(fromUrl, toUrl)) {
        stateInfo.setError(tr("Can't copy file '%1' to '%2'.").arg(fromUrl).arg(toUrl));
    }
    return ReportResult_Finished;
}

void GTest_CopyFile::replacePrefix(QString &path) {
    QString result;

    const QString EXPECTED_OUTPUT_DIR_PREFIX = "!expected!";
    const QString TMP_DATA_DIR_PREFIX = "!tmp_data_dir!";
    const QString COMMON_DATA_DIR_PREFIX = "!common_data_dir!";

    // Determine which environment variable is required
    QString envVarName;
    QString prefix;
    if (path.startsWith(EXPECTED_OUTPUT_DIR_PREFIX)) {
        envVarName = "EXPECTED_OUTPUT_DIR";
        prefix = EXPECTED_OUTPUT_DIR_PREFIX;
    }
    else if (path.startsWith(TMP_DATA_DIR_PREFIX)) {
        envVarName = "TEMP_DATA_DIR";
        prefix = TMP_DATA_DIR_PREFIX;
    }
    else if (path.startsWith(COMMON_DATA_DIR_PREFIX)) {
        envVarName = "COMMON_DATA_DIR";
        prefix = COMMON_DATA_DIR_PREFIX;
    }
    else {
        FAIL(QString("Unexpected 'prefix' value in the path: '%1'!").arg(path), );
    }

    // Replace with the correct value
    QString prefixPath = env->getVar(envVarName);
    SAFE_POINT(!prefixPath.isEmpty(), QString("No value for environment variable '%1'!").arg(envVarName), );
    prefixPath += "/";

    int prefixSize = prefix.size();
    QStringList relativePaths = path.mid(prefixSize).split(";");

    foreach (const QString &path, relativePaths) {
        QString fullPath = prefixPath + path;
        result += fullPath + ";";
    }

    path = result.mid(0, result.size() - 1); // without the last ';'
}

/*******************************
* GUrlTests
*******************************/
QList<XMLTestFactory*> UtilTestActions::createTestFactories() {
    QList<XMLTestFactory*> res;
    res.append(GTest_CopyFile::createFactory());
    return res;
}

}   // namespace U2
