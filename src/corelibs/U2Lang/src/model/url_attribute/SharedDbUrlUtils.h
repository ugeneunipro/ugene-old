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

#ifndef _U2_SHARED_DB_URL_UTILS_H_
#define _U2_SHARED_DB_URL_UTILS_H_

#include <U2Core/U2Type.h>

namespace U2 {

class Folder;
class GObject;

class U2LANG_EXPORT SharedDbUrlUtils {
public:
    static bool checkBaseDbUrlStructure(const QString &dbUrl);

    static QString createDbUrl(const U2DbiRef &dbiRef);

    // Example of DB Folder URL: "SQL_SERVER>user@ugene.com:3306/ugene_db,/path/to/folder"
    static QString createDbFolderUrl(const Folder &folder);
    static QString createDbFolderUrl(const QString &dbUrl, const QString &path);
    static bool isDbFolderUrl(const QString &url);

    // Example of DB Object URL: "SQL_SERVER>user@ugene.com:3306/ugene_db,123:1:human_T1"
    // the string after the comma has the following format: <object_id>:<object_type>:<object_cached_name>
    static QString createDbObjectUrl(const GObject *obj);
    // @objType here is the ID of the BaseTypes class members
    static QString createDbObjectUrl(const QString &dbUrl, qint64 objId, const QString &objType, const QString &objName);
    static bool isDbObjectUrl(const QString &url);

    // Extracts DB reference from an object of folder url.
    static U2DbiRef getDbRefFromEntityUrl(const QString &url);
    static QString getDbUrlFromEntityUrl(const QString &url);

    // Returns a user-defined name for the DB according to the @url string.
    // If the DB is unknown or invalid, the DB URL string is returned
    static QString getDbShortNameFromEntityUrl(const QString &url);

    // Returning map contains DB URL <=> DB short name
    static QVariantMap getKnownDbs();
    // Stores a new connection in app settings
    static void saveNewDbConnection(const QString &connectionName, const QString &connectionUrl);

    static U2DataId getObjectIdByUrl(const QString &url);
    static qint64 getObjectNumberIdByUrl(const QString &url);
    static GObjectType getDbObjectTypeByUrl(const QString &url);
    static QString getDbSerializedObjectTypeByUrl(const QString &url);
    static QString getDbObjectNameByUrl(const QString &url);

    static QString getDbFolderPathByUrl(const QString &url);

    static const QString DB_PROVIDER_SEP;
    static const QString DB_URL_SEP;
    static const QString DB_OBJ_ID_SEP;
};

} // namespace U2

#endif // _U2_SHARED_DB_URL_UTILS_H_
