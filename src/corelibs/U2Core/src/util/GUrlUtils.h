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

#ifndef _U2_GURL_UTILS_H_ 
#define _U2_GURL_UTILS_H_

#include <U2Core/global.h>
#include <U2Core/GUrl.h>

#include <QtCore/QSet>
#include <QtCore/QStringList>
#include <QtCore/QUrl>

namespace U2 {

class TaskStateInfo;
class Logger;
class U2OpStatus;


class U2CORE_EXPORT GUrlUtils : public QObject {
    Q_OBJECT
public:

    //gets the uncompressed extension for the URL. Filters 'gz' like suffixes
    static QString  getUncompressedExtension(const GUrl& url);

    // ensures that url ends with one of the exts
    static GUrl     ensureFileExt(const GUrl& url, const QStringList& typeExt);

    // returns url suitable for backup renaming (increments name for rolling file until non-existing name found)
    static QString  rollFileName(const QString& url, const QString& rolledSuffix, const QSet<QString>& excludeList);

    // same as above but with empty suffix
    static QString  rollFileName(const QString& url, const QSet<QString>& excludeList) { return rollFileName(url, "", excludeList);}

    // returns a list of existing files with rolled @originalUrl
    static QStringList getRolledFilesList(const QString& originalUrl, const QString& rolledSuffix);

    // renames actual file by rolling its name
    static bool renameFileWithNameRoll(const QString& url, TaskStateInfo& ti, const QSet<QString>& excludeList = QSet<QString>(), Logger* log = NULL);

    // converts GUrl to QUrl
    static QUrl gUrl2qUrl( const GUrl& gurl);

    static QList<QUrl> gUrls2qUrls( const QList<GUrl>& gurls);

    // converts QUrl to GUrl
    static GUrl qUrl2gUrl( const QUrl& qurl);

    static QList<GUrl> qUrls2gUrls( const QList<QUrl>& qurls);


    // adds baseSuffix to the url. Ensures that URL ends with one of the exts from typeExt
    static QString prepareFileName(const QString& url, const QString& baseSuffix, const QStringList& typeExt);

    // adds count to the url. Ensures that URL ends with one of the exts from typeExt
    static QString prepareFileName(const QString& url, int count, const QStringList& typeExt);

    // checks that file path is valid: creates required directory if needed. 
    // Returns canonical path to file. Does not create nor remove file, affects just directory
    // Returns empty string and error message if some error occurs
    // Sample usage: processing URLs in "save file" inputs
    static QString prepareFileLocation(const QString& filePath, U2OpStatus& os);

    // checks that dir path is valid. Creates the directory if needed. 
    // Returns canonical directory path. Does not affect directory if already exists.
    // Returns empty string and error message if some error occurs
    // Sample usage: processing URLs in "save dir" inputs
    static QString prepareDirLocation(const QString& dirPath, U2OpStatus& os);

    static QString prepareTmpFileLocation(const QString& dir, const QString& prefix, const QString& ext, U2OpStatus& os);

    static void removeDir(const QString& path, U2OpStatus& os);

    /*removes file if exists and if its a file*/
    static void removeFile(const QString& filePath, U2OpStatus& os);

    //checks if filePath is writable
    //creates and deletes a tmp file to check permissions
    //it seems the only way to check permissions correctly on all platforms
    static bool canWriteFile(const QString& path);

    //returns default path for UGENE data
    //returns empty string if path is not ready
    static QString getDefaultDataPath();

};

} //namespace

#endif
