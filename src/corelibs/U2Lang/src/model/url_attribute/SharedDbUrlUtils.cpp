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

#include <U2Core/AppContext.h>
#include <U2Core/Folder.h>
#include <U2Core/Settings.h>
#include <U2Core/U2DbiUtils.h>
#include <U2Core/U2ObjectDbi.h>
#include <U2Core/U2ObjectTypeUtils.h>
#include <U2Core/U2SafePoints.h>

#include "SharedDbUrlUtils.h"

namespace U2 {

const QString SharedDbUrlUtils::DB_PROVIDER_SEP = ">";
const QString SharedDbUrlUtils::DB_URL_SEP = ",";
const QString SharedDbUrlUtils::DB_OBJ_ID_SEP = ":";

namespace {

const QString CONN_SETTINGS_PATH = "/shared_database/recent_connections/";

bool checkFolderPath(const QString &folderPath) {
    return !folderPath.isEmpty() && folderPath.startsWith(U2ObjectDbi::ROOT_FOLDER);
}

bool disassembleObjectId(const QString &objUrl, QStringList &idParts) {
    SAFE_POINT(SharedDbUrlUtils::isDbObjectUrl(objUrl), "Invalid shared DB object URL", false);
    const QString fullObjId = objUrl.mid(objUrl.indexOf(SharedDbUrlUtils::DB_URL_SEP) + 1);
    idParts.clear();
    const int firstSepPos = fullObjId.indexOf(SharedDbUrlUtils::DB_OBJ_ID_SEP);
    SAFE_POINT(-1 != firstSepPos, "Invalid object DB URL", false);
    idParts.append(fullObjId.left(firstSepPos));

    const int secondSepPos = fullObjId.indexOf(SharedDbUrlUtils::DB_OBJ_ID_SEP, firstSepPos + 1);
    SAFE_POINT(-1 != secondSepPos, "Invalid object DB URL", false);
    SAFE_POINT(fullObjId.size() - 2 >= secondSepPos, "Invalid object DB URL", false);
    idParts.append(fullObjId.mid(firstSepPos + 1, secondSepPos - firstSepPos - 1));
    idParts.append(fullObjId.mid(secondSepPos + 1));

    return true;
}

bool str2Int(const QString &str, qint64 &res) {
    bool conversionOk = false;
    res = str.toLongLong(&conversionOk);
    return conversionOk;
}

bool str2DataType(const QString &str, U2DataType &res) {
    bool conversionOk = false;
    // hope that "U2DataType" typedef won't change
    SAFE_POINT(sizeof(U2DataType) == sizeof(qint16), "Unexpected data type detected", false);
    res = str.toUShort(&conversionOk);
    return conversionOk;
}

}

QString SharedDbUrlUtils::createDbUrl(const U2DbiRef &dbiRef) {
    SAFE_POINT(dbiRef.isValid(), "Invalid DBI reference", QString());
    return dbiRef.dbiFactoryId + DB_PROVIDER_SEP + dbiRef.dbiId;
}

QString SharedDbUrlUtils::createDbFolderUrl(const Folder &folder) {
    Document *doc = folder.getDocument();
    CHECK(NULL != doc, QString());
    const U2DbiRef dbiRef = doc->getDbiRef();
    CHECK(dbiRef.isValid(), QString());

    const QString folderPath = folder.getFolderPath();
    CHECK(checkFolderPath(folderPath), false);

    return dbiRef.dbiFactoryId + DB_PROVIDER_SEP + dbiRef.dbiId + DB_URL_SEP + folderPath;
}

bool SharedDbUrlUtils::isDbFolderUrl(const QString &url) {
    const int dbProviderSepPos = url.indexOf(DB_PROVIDER_SEP);
    if (dbProviderSepPos < 1) {
        return false;
    }

    const int dbUrlSepPos = url.indexOf(DB_URL_SEP, dbProviderSepPos);
    if (-1 == dbUrlSepPos || url.size() - 2 < dbUrlSepPos || U2ObjectDbi::ROOT_FOLDER != url[dbUrlSepPos + 1]) {
        return false;
    } else {
        return true;
    }
}

QString SharedDbUrlUtils::createDbObjectUrl(const GObject *obj) {
    SAFE_POINT(NULL != obj, "Invalid object", QString());
    const U2EntityRef entityRef = obj->getEntityRef();

    const U2DbiRef dbiRef = entityRef.dbiRef;
    CHECK(dbiRef.isValid(), QString());

    const qint64 idNumber = U2DbiUtils::toDbiId(entityRef.entityId);
    const U2DataType objType = U2DbiUtils::toType(entityRef.entityId);
    const QString objIdStr = QString::number(idNumber) + DB_OBJ_ID_SEP + QString::number(objType)
        + DB_OBJ_ID_SEP + obj->getGObjectName();

    return dbiRef.dbiFactoryId + DB_PROVIDER_SEP + dbiRef.dbiId + DB_URL_SEP + objIdStr;
}

bool SharedDbUrlUtils::isDbObjectUrl(const QString &url) {
    const int dbProviderSepPos = url.indexOf(DB_PROVIDER_SEP);
    CHECK(dbProviderSepPos >= 1, false);

    const int dbUrlSepPos = url.indexOf(DB_URL_SEP, dbProviderSepPos);
    CHECK(-1 != dbUrlSepPos, false);

    const int dbObjIdSepPos = url.indexOf(DB_OBJ_ID_SEP, dbUrlSepPos);
    CHECK(-1 != dbObjIdSepPos, false);

    const int dbObjTypeSepPos = url.indexOf(DB_OBJ_ID_SEP, dbObjIdSepPos + 1);
    if (-1 == dbObjTypeSepPos || url.size() - 2 < dbObjTypeSepPos) {
        return false;
    } else {
        return true;
    }
}

U2DbiRef SharedDbUrlUtils::getDbRefFromEntityUrl(const QString &url) {
    const int dbProviderSepPos = url.indexOf(DB_PROVIDER_SEP);
    CHECK(dbProviderSepPos >= 1, U2DbiRef());

    const int dbUrlSepPos = url.indexOf(DB_URL_SEP, dbProviderSepPos);
    return U2DbiRef(url.right(dbProviderSepPos), url.mid(dbProviderSepPos + 1, -1 != dbUrlSepPos ? dbUrlSepPos - dbProviderSepPos - 1 : -1));
}

QVariantMap SharedDbUrlUtils::getKnownDbs() {
    QVariantMap result;
    Settings *appSettings = AppContext::getSettings();
    const QStringList recentList = appSettings->getAllKeys(CONN_SETTINGS_PATH);
    foreach (const QString &shortName, recentList) {
        // TODO: fix this when other DB providers emerge
        result.insert(shortName, MYSQL_DBI_ID + DB_PROVIDER_SEP + appSettings->getValue(CONN_SETTINGS_PATH + shortName).toString());
    }
    return result;
}

QString SharedDbUrlUtils::getDbShortNameFromEntityUrl(const QString &url) {
    const QString dbUrl = getDbRefFromEntityUrl(url).dbiId;
    CHECK(!dbUrl.isEmpty(), url);
    Settings *appSettings = AppContext::getSettings();
    const QStringList recentList = appSettings->getAllKeys(CONN_SETTINGS_PATH);
    foreach (const QString &shortName, recentList) {
        if (dbUrl == appSettings->getValue(CONN_SETTINGS_PATH + shortName).toString()) {
            return shortName;
        }
    }
    return dbUrl;
}

void SharedDbUrlUtils::saveNewDbConnection(const QString &connectionName, const QString &connectionUrl) {
    SAFE_POINT(!connectionName.isEmpty() && !connectionUrl.isEmpty(), "Unexpected DB connection", );
    AppContext::getSettings()->setValue(CONN_SETTINGS_PATH + connectionName, connectionUrl);
}

U2DataId SharedDbUrlUtils::getObjectIdByUrl(const QString &objUrl) {
    QStringList objIdParts;
    CHECK(disassembleObjectId(objUrl, objIdParts), U2DataId());

    qint64 idNumber = 0;
    CHECK(str2Int(objIdParts[0], idNumber), U2DataId());

    U2DataType dataType = U2Type::Unknown;
    CHECK(str2DataType(objIdParts[1], dataType), U2DataId());

    return U2DbiUtils::toU2DataId(idNumber, dataType);
}

GObjectType SharedDbUrlUtils::getDbObjectTypeByUrl(const QString &objUrl) {
    QStringList objIdParts;
    CHECK(disassembleObjectId(objUrl, objIdParts), GObjectType());

    U2DataType dataType = U2Type::Unknown;
    CHECK(str2DataType(objIdParts[1], dataType), GObjectType());

    return U2ObjectTypeUtils::toGObjectType(dataType);
}

QString SharedDbUrlUtils::getDbObjectNameByUrl(const QString &objUrl) {
    QStringList objIdParts;
    CHECK(disassembleObjectId(objUrl, objIdParts), QString());

    return objIdParts[2];
}

QString SharedDbUrlUtils::getDbFolderPathByUrl(const QString &url) {
    SAFE_POINT(isDbFolderUrl(url), "Invalid DB folder URL", QString());

    const QString path = url.mid(url.indexOf(SharedDbUrlUtils::DB_URL_SEP) + 1);
    SAFE_POINT(!path.isEmpty(), "Invalid shared DB folder URL", QString());

    return path;
}

} // namespace U2
